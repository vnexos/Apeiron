/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file main.cpp
 * @brief Tệp khởi đầu của Bộ nạp mồi
 */
#include <acpi.hpp>
#include <cpu.hpp>
#include <efilib.hpp>
#include <post_quantum/sign.hpp>
#include <string.hpp>

using namespace EFI;

struct __attribute__((packed)) BmpFileHeader
{
  uint16_t BfType; // Phải là 0x4d42 ('B', 'M')
  uint32_t BfSize; // Tổng dung lượng toàn bộ tệp BMP
  uint16_t BfReserved1;
  uint16_t BfReserved2;
  uint32_t BfOffBits; // Vị trí bắt đầu của mảng Pixel
};

struct __attribute__((packed)) BmpInfoHeader
{
  uint32_t BiSize;
  int32_t  BiWidth;       // Chiều rộng
  int32_t  BiHeight;      // Chiều cao
  uint16_t BiPlanes;
  uint16_t BiBitCount;    // Sộ sâu màu (Ví dụ: 24-bit RGB hoặc 32-bit ARGB)
  uint32_t BiCompression; // Thường là 0 (Không nén)
};

EFI_GRAPHICS_OUTPUT_PROTOCOL* setupGraphics(EFI_BOOT_SERVICES* bs)
{
  // GIAO THỨC ĐỒ HỌA
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  EFI_GUID                      gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  if (EFI_ERROR(bs->LocateProtocol(&gopGuid, nullptr, (void**)&gop)))
    return nullptr;

  return gop;
}

ACPI_BGRT* getBgrt(EFI_SYSTEM_TABLE* SystemTable)
{
  EFI_GUID Acpi20TableGuid = {
      0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

  // Tìm RSDP trong Bảng cấu hình
  ACPI_RSDP* rsdp = nullptr;
  for (uint64_t i = 0; i < SystemTable->NumberOfTableEntries; ++i)
  {
    if (memcmp(&SystemTable->ConfigurationTable[i].VendorGuid,
               &Acpi20TableGuid, sizeof(EFI_GUID)) == 0)
    {
      rsdp = (ACPI_RSDP*)SystemTable->ConfigurationTable[i].VendorTable;
      break;
    }
  }

  if (!rsdp || rsdp->Revision < 2 || rsdp->XsdtAddress == 0)
    return nullptr;

  // Lấy XSDT từ RSDP
  ACPI_XSDT* xsdt    = (ACPI_XSDT*)rsdp->XsdtAddress;
  uint64_t   entries = (xsdt->Header.Length - sizeof(ACPI_SDT_HEADER)) / 8;

  // Duyệt tìm bảng BGRT
  for (uint64_t i = 0; i < entries; ++i)
  {
    ACPI_SDT_HEADER* hdr = (ACPI_SDT_HEADER*)xsdt->Entry[i];
    if (memcmp(hdr->Signature, "BGRT", 4) == 0)
      return (ACPI_BGRT*)hdr;
  }

  return nullptr;
}

extern "C" [[gnu::ms_abi]] EFI_STATUS vnexos_grub_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  EFI_BOOT_SERVICES* bs = SystemTable->BootServices;

  /* Khởi tạo thư viện EFI */
  init(ImageHandle, SystemTable);
  clear();

  /* Cấu hình đồ họa */
  EFI_GRAPHICS_OUTPUT_PROTOCOL*      gop          = setupGraphics(bs);
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* graphicsMode = gop->Mode;

  uint32_t* framebuffer = (uint32_t*)graphicsMode->FrameBufferBase;
  uint32_t  resX        = graphicsMode->Info->HorizontalResolution;
  uint32_t  resY        = graphicsMode->Info->VerticalResolution;
  uint32_t  ppsl        = graphicsMode->Info->PixelsPerScanLine;

  /* Vẽ biểu trưng của hãng sản xuất ra màn hình */
  ACPI_BGRT* bgrt = getBgrt(SystemTable);

  if (bgrt)
  {
    uint8_t* bmpBuffer = (uint8_t*)(uintptr_t)bgrt->ImageAddress;

    BmpFileHeader* fileHeader = (BmpFileHeader*)bmpBuffer;
    if (fileHeader->BfType != 0x4d42)
      goto invalidBgrt;

    BmpInfoHeader* infoHeader = (BmpInfoHeader*)(bmpBuffer + sizeof(BmpFileHeader));
    uint32_t       width      = infoHeader->BiWidth;
    uint32_t       height     = infoHeader->BiHeight;

    uint32_t rowSize   = ((infoHeader->BiBitCount * infoHeader->BiWidth + 31) / 32) * 4;
    uint8_t* rawData   = bmpBuffer + fileHeader->BfOffBits;
    uint32_t totalSize = width * height;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* logo;
    bs->AllocatePool(EfiLoaderData, totalSize * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (void**)&logo);

    for (uint64_t y = 0; y < height; ++y)
    {
      // SỬA LỖI LẬT TRỤC Y: B bốc hàng từ đáy file BMP lên (height - 1 - y)
      // để nạp vào hàng đỉnh của mảng logo, giúp ảnh hiển thị xuôi chiều!
      uint8_t* rowPtr = rawData + ((height - 1 - y) * rowSize);

      for (uint64_t x = 0; x < width; ++x)
      {
        uint64_t logoIdx = y * width + x;

        if (infoHeader->BiBitCount == 32)
        {
          // SỬA LỖI MÀU 32-BIT: BMP 32-bit là BGRA, khớp hoàn toàn với cấu trúc BLT PIXEL
          logo[logoIdx].Blue     = rowPtr[x * 4 + 0];
          logo[logoIdx].Green    = rowPtr[x * 4 + 1];
          logo[logoIdx].Red      = rowPtr[x * 4 + 2];
          logo[logoIdx].Reserved = rowPtr[x * 4 + 3];
        } else if (infoHeader->BiBitCount == 24)
        {
          // SỬA LỖI MÀU 24-BIT: BMP 24-bit là BGR, copy đúng kênh vào BLT PIXEL
          logo[logoIdx].Blue     = rowPtr[x * 3 + 0];
          logo[logoIdx].Green    = rowPtr[x * 3 + 1];
          logo[logoIdx].Red      = rowPtr[x * 3 + 2];
          logo[logoIdx].Reserved = 0; // Luôn xóa byte dự phòng về 0
        }
      }
    }

    EFI_STATUS status = gop->Blt(
        gop, logo,
        EfiBltBufferToVideo,
        0, 0,
        bgrt->ImageXOffset,
        bgrt->ImageYOffset,
        width, height, 0);
    if (EFI_ERROR(status))
      return 1;
  }

invalidBgrt:

  waitForKey();

  uint8_t*   buffer;
  uint64_t   size;
  EFI_STATUS status = loadFile(EFI_TEXT("\\EFI\\BOOT\\vnexos.efi"), &buffer, &size);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\vnexos.efi");
    return 1;
  }

  uint8_t* key;
  uint64_t keySize;
  status = loadFile(EFI_TEXT("\\EFI\\BOOT\\root.crt"), &key, &keySize);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\root.crt");
    return 1;
  }

  if (Sign::verifyFileData(buffer, size, key, keySize))
    printf("OK\n");
  else
    printf("NG\n");

  bs->FreePool(key);
  bs->FreePool(buffer);

  while (true)
  {
    cpu_halt();
  }
  return EFI_SUCCESS;
}
