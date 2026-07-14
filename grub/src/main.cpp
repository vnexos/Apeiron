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
#include <common.hpp>
#include <cpu.hpp>
#include <efilib.hpp>
#include <post_quantum/sig/dilithium.hpp>
#include <post_quantum/sign.hpp>
#include <string.hpp>

#if defined(__x86_64__)
#define VNEXOS_FILE EFI_TEXT("\\EFI\\BOOT\\vnexosx64.efi")
#elif defined(__aarch64__)
#define VNEXOS_FILE EFI_TEXT("\\EFI\\BOOT\\vnexosaa64.efi")
#elif defined(__riscv)
#define VNEXOS_FILE EFI_TEXT("\\EFI\\BOOT\\vnexosriscv64.efi")
#else
#error "Dòng vi xử lý này chưa được VNExos hỗ trợ!"
#endif

using namespace EFI;

static const uint16_t* logoPath[4];

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
  logoPath[0]           = EFI_TEXT("\\assets\\logos\\logo512.bmp");
  logoPath[1]           = EFI_TEXT("\\assets\\logos\\logo256.bmp");
  logoPath[2]           = EFI_TEXT("\\assets\\logos\\logo128.bmp");
  logoPath[3]           = EFI_TEXT("\\assets\\logos\\logo64.bmp");

  /* Khởi tạo thư viện EFI */
  init(ImageHandle, SystemTable);
  printf("\r"); // Đưa con trỏ về phía đầu màn hình

  /* Cấu hình đồ họa */
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = setupGraphics();
  EFI_STATUS                    status;

  uint32_t logoWidth          = 0;
  uint32_t logoHeight         = 0;
  uint64_t vnexosLogoSize     = 0,
           vnexosLogoPosition = 0;

  uint64_t resX = 0,
           resY = 0;

  /* Đọc chứng chỉ gốc vào bộ nhớ */
  uint8_t* key;
  uint64_t keySize;
  status = loadFile(EFI_TEXT("\\EFI\\BOOT\\root.crt"), &key, &keySize);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\nNhan phim bat ky de thoat...", "\\EFI\\BOOT\\root.crt");
    waitForKey();
    printf("\n");
    return status;
  }

  /* Thông tin biểu trưng của hãng thiết bị */
  ACPI_BGRT* bgrt = getBgrt(SystemTable);

  if (gop)
  {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* graphicsMode = gop->Mode;

    resX = graphicsMode->Info->HorizontalResolution;
    resY = graphicsMode->Info->VerticalResolution;

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
    int32_t logoSegment = 0;

    if (bgrt)
    {
      drawBmp(bgrt->ImageAddress, bgrt->ImageXOffset, bgrt->ImageYOffset, &logoWidth, &logoHeight);
      /* Tính toán và phân vùng để vẽ biểu trưng phù hợp */
      logoSegment = (resY - (bgrt->ImageYOffset + logoHeight)) / 2;
    }

    /* Kiểm tra chữ ký ảnh và vẽ ra màn hình */
    uint8_t* imageData;
    uint64_t imageSize;
    uint8_t  logoPathIndex;
    if (logoSegment)
    {
      if (logoSegment > 512)
        logoPathIndex = 0;
      else if (logoSegment > 256)
        logoPathIndex = 1;
      else if (logoSegment > 128)
        logoPathIndex = 2;
      else
        logoPathIndex = 3;
    } else
    {
      uint64_t logoPart = resY / 3;
      if (logoPart > 512)
        logoPathIndex = 0;
      else if (logoPart > 256)
        logoPathIndex = 1;
      else if (logoPart > 128)
        logoPathIndex = 2;
      else
        logoPathIndex = 3;
    }

    status = loadFile(logoPath[logoPathIndex], (uint8_t**)&imageData, &imageSize);

    if (!EFI_ERROR(status))
    {
      if (Sign::verifyFileSignature(imageData, imageSize, key, keySize))
      {
        BmpInfoHeader* bi = (BmpInfoHeader*)(imageData + sizeof(BmpFileHeader));
        uint64_t       logoX, logoY;
        if (logoSegment)
        {
          logoX          = (resX - bi->BiWidth) / 2;
          int64_t deltaY = (logoSegment - bi->BiHeight) / 2;
          logoY          = (bi->BiHeight < logoSegment) ? (resY - logoSegment + deltaY) : (resY - bi->BiHeight - 20);
        } else
        {
          logoX = (resX - bi->BiWidth) / 2;
          logoY = (resY - bi->BiHeight) / 2;
        }
        vnexosLogoSize     = PACK_32_TO_64(bi->BiWidth, bi->BiHeight);
        vnexosLogoPosition = PACK_32_TO_64(logoX, logoY);
        drawBmp((uint64_t)imageData, logoX, logoY);
      } else
      {
        printf("LOI: Tep bieu trung khong the xac thuc: %s\nNhan phim bat ky de thoat...", logoPath[logoPathIndex]);
        waitForKey();
        printf("\n");
        return status;
      }
      bs->FreePool(imageData);
    }
  }

  /* Đọc bộ nạp mồi lên bộ nhớ, kiểm tra chữ ký và xử lý nó */
  uint8_t* buffer;
  uint64_t size;
  status = loadFile(VNEXOS_FILE, &buffer, &size);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\nNhan phim bat ky de thoat...", VNEXOS_FILE);
    waitForKey();
    printf("\n");
    return status;
  }

  if (!Sign::verifyEfiFileSignature(buffer, size, key, keySize))
  {
    printf("LOI: Chu ky khong hop le: %s\nNhan phim bat ky de thoat...", VNEXOS_FILE);
    waitForKey();
    printf("\n");
    return 1;
  }

  bs->FreePool(key);

  EFI_LOADED_IMAGE_PROTOCOL* imageLip     = nullptr;
  EFI_GUID                   imageLipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  bs->OpenProtocol(ImageHandle, &imageLipGuid, (void**)&imageLip, ImageHandle, nullptr, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

  EFI_HANDLE childImageHandle;
  status = bs->LoadImage(
      false,
      ImageHandle,
      imageLip ? imageLip->FilePath : nullptr,
      buffer,
      size,
      &childImageHandle);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the tai tep EFI (Ma loi: %x)\nNhan phim bat ky de thoat...", status);
    waitForKey();
    printf("\n");
    return status;
  }

  bs->FreePool(buffer);

  /* Đóng gói các thông tin quan trọng để gửi từ bộ nạp mồi sang bộ nạp chính */
  ApeironCommonParameters* params;

  status = bs->AllocatePool(EfiLoaderData, sizeof(ApeironCommonParameters), (void**)&params);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the cap phat bo nho\nNhan phim bat ky de thoat...");
    waitForKey();
    printf("\n");
    return status;
  }

  params->horizontalResolution = resX;
  params->verticalResolution   = resY;
  params->OEMLogoSize          = PACK_32_TO_64(logoWidth, logoHeight);
  if (bgrt)
    params->OEMLogoPosition = PACK_32_TO_64(bgrt->ImageXOffset, bgrt->ImageYOffset);
  else
    params->OEMLogoPosition = 0;
  params->logoSize               = vnexosLogoSize;
  params->logoPosition           = vnexosLogoPosition;
  params->graphicsOutputProtocol = gop;

  /* Dùng giao thức ảnh đã tải để truyền thông tin qua chương trình tiếp theo */
  EFI_LOADED_IMAGE_PROTOCOL* loadedChildImage;
  EFI_GUID                   loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

  status = bs->OpenProtocol(
      childImageHandle,
      &loadedImageGuid,
      (void**)&loadedChildImage,
      ImageHandle, nullptr,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the mo giao thuc anh da tai cho tep EFI\nNhan phim bat ky de thoat...");
    waitForKey();
    printf("\n");
    return status;
  }

  loadedChildImage->DeviceHandle    = imageLip->DeviceHandle;
  loadedChildImage->LoadOptions     = (void*)params;
  loadedChildImage->LoadOptionsSize = sizeof(ApeironCommonParameters);

  return bs->StartImage(childImageHandle, nullptr, nullptr);
}
