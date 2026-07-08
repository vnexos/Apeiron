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

static uint16_t* filePath[4] = {
    EFI_TEXT("\\assets\\logos\\logo512.bmp"),
    EFI_TEXT("\\assets\\logos\\logo256.bmp"),
    EFI_TEXT("\\assets\\logos\\logo128.bmp"),
    EFI_TEXT("\\assets\\logos\\logo64.bmp"),
};

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

  /* Cấu hình đồ họa */
  EFI_GRAPHICS_OUTPUT_PROTOCOL*      gop          = setupGraphics();
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* graphicsMode = gop->Mode;

  uint64_t resX = graphicsMode->Info->HorizontalResolution;
  uint64_t resY = graphicsMode->Info->VerticalResolution;

  /* Lấp đầy màn hình bằng một màu */
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL fill_color{
      0,
      0,
      0,
      0,
  };

  gop->Blt(
      gop, &fill_color,
      EfiBltVideoFill,
      0, 0,
      0, 0,
      resX, resY,
      0);

  /* Vẽ biểu trưng của hãng sản xuất ra màn hình */
  ACPI_BGRT* bgrt = getBgrt(SystemTable);

  uint32_t logoWidth  = 0;
  uint32_t logoHeight = 0;

  if (bgrt)
  {
    drawBmp(bgrt->ImageAddress, bgrt->ImageXOffset, bgrt->ImageYOffset, &logoWidth, &logoHeight);
  }

  /* Tính toán và phân vùng để vẽ biểu trưng phù hợp */
  int32_t logoSegment = (resY - (bgrt->ImageYOffset + logoHeight)) / 3;

  /* Đọc chứng chỉ gốc vào bộ nhớ */
  uint8_t*   key;
  uint64_t   keySize;
  EFI_STATUS status = loadFile(EFI_TEXT("\\EFI\\BOOT\\root.crt"), &key, &keySize);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\root.crt");
    return 1;
  }

  /* Kiểm tra chữ ký ảnh và vẽ ra màn hình */
  uint8_t* imageData;
  uint64_t imageSize;
  if (logoSegment > 512)
    status = loadFile(filePath[0], (uint8_t**)&imageData, &imageSize);
  else if (logoSegment > 256)
    status = loadFile(filePath[1], (uint8_t**)&imageData, &imageSize);
  else if (logoSegment > 128)
    status = loadFile(filePath[2], (uint8_t**)&imageData, &imageSize);
  else
    status = loadFile(filePath[3], (uint8_t**)&imageData, &imageSize);
  if (!EFI_ERROR(status))
  {
    if (Sign::verifyFileSignature(imageData, imageSize, key, keySize))
    {
      BmpInfoHeader* bi = (BmpInfoHeader*)(imageData + sizeof(BmpFileHeader));

      uint64_t logoX  = (resX - bi->BiWidth) / 2;
      int64_t  deltaY = (logoSegment - bi->BiHeight) / 2;
      uint64_t logoY  = (bi->BiHeight < logoSegment) ? (resY - logoSegment + deltaY) : (resY - bi->BiHeight - 20);

      drawBmp((uint64_t)imageData, logoX, logoY);
    }
  }
  bs->FreePool(imageData);

  waitForKey();

  uint8_t* buffer;
  uint64_t size;
  status = loadFile(EFI_TEXT("\\EFI\\BOOT\\vnexos.efi"), &buffer, &size);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\vnexos.efi");
    return 1;
  }

  if (Sign::verifyFileSignature(buffer, size, key, keySize))
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
