/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efilib.hpp
 * @brief Triển khai các hàm tiện ích.
 */
#include "efilib.hpp"
#include <efi.hpp>
#include <stdarg.h>

static EFI_HANDLE         ImageHandle;
static EFI_SYSTEM_TABLE*  SystemTable;
static EFI_FILE_PROTOCOL* rootDir = nullptr;
// Cấu trúc giao thức đồ họa
static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

void EFI::init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  ::ImageHandle = ImageHandle;
  ::SystemTable = SystemTable;
}

void EFI::clear()
{
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

void EFI::printf(const char* format, ...)
{
  uint16_t buffer[1024];
  uint64_t idx = 0;

  // Đẩy 1 ký tự vào bộ đệm
  auto putChar = [&](uint16_t c) {
    if (idx < 1023) buffer[idx++] = c;
  };

  // Đẩy cả chuỗi vào bộ đệm
  auto putStr = [&](const char* s) {
    while (*s && idx < 1023)
      putChar(*(s++));
  };

  auto putWstr = [&](const uint16_t* s) {
    while (*s && idx < 1023)
      putChar(*(s++));
  };

  // Chuyển một số thành chuỗi
  auto putNumber = [&](uint64_t num, int base, int width) {
    const uint16_t* digits = EFI_TEXT("0123456789abcdef");
    uint16_t        tmp[32];
    int             i = 0;

    if (num == 0)
    {
      tmp[i++] = '0';
    } else
    {
      while (num > 0)
      {
        tmp[i++]  = digits[num % base];
        num      /= base;
      }
    }

    while (i < width)
    {
      tmp[i++] = '0';
    }

    while (--i >= 0)
    {
      putChar(tmp[i]);
    }
  };

  va_list args;
  va_start(args, format);

  const char* p = format;
  while (*p)
  {
    // Kiểm tra nếu gặp ký tự định dạng %
    if (*p == '%' && *(p + 1))
    {
      ++p;

      int width = 1;

      if (*p == '0') // Bỏ qua số 0 sau dấu %
        p++;

      // Cộng số vào width
      if (*p >= '1' && *p <= '9')
      {
        width = 0;
        while (*p >= '0' && *p <= '9')
        {
          width = width * 10 + (*p - '0');
          ++p;
        }
      }

      switch (*p) // Nhảy qua dấu %
      {
      case 'd':   // Số nguyên có dấu
      {
        int d = va_arg(args, int);
        if (d < 0)
        {
          putChar('-');
          d = -d;
        }
        putNumber(d, 10, width);
        break;
      }
      case 'x': // Số nguyên hệ Thập lục phân
      case 'p': // Con trỏ
      {
        uint64_t x = va_arg(args, uint64_t);
        putNumber(x, 16, width);
        break;
      }
      case 's': // Chuỗi ký tự rộng
      {
        char* s = va_arg(args, char*);
        if (s)
          putStr(s);
        else
          putStr("(null)");
        break;
      }
      case 'c': // Ký tự đơn
      {
        // va_arg tự động biến char/short thành int
        uint16_t c = (uint16_t)va_arg(args, int);
        putChar(c);
        break;
      }
      case 'w': // Chuỗi ký tự rộng
      {
        ++p;

        switch (*p)
        {
        case 's': {
          uint16_t* s = va_arg(args, uint16_t*);
          if (s)
            putWstr(s);
          else
            putWstr(EFI_TEXT("(null)"));
          break;
        }
        default:
          putChar('%');
          putChar('w');
          putChar(*p);
          break;
        }

        break;
      }
      case '%': // In ra dấu %
        putChar('%');
        break;
      default:
        putChar('%');
        putChar(*p);
        break;
      }
    } else if (*p == '\n')
    {
      putStr("\r\n");
    } else
    {
      putChar(*p);
    }
    ++p;
  }

  va_end(args);
  buffer[idx] = 0;

  SystemTable->ConOut->OutputString(SystemTable->ConOut, buffer);
}

void EFI::waitForKey(uint8_t k)
{
  // Xóa toàn bộ các phím còn trong bộ đệm
  SystemTable->ConIn->Reset(SystemTable->ConIn, false);

  EFI_INPUT_KEY key;
  uint64_t      index;

  while (true)
  {
    SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &index);

    if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) == 0)
    {
      // Nhấn bất kỳ phím nào
      if (k == 0)
        return;

      // Kiểm tra xem phím bấm có trùng với phím đích không
      if (key.UnicodeChar == k)
        return;
    }
  }
}

EFI_STATUS EFI::loadFile(const uint16_t* path, uint8_t** buffer, uint64_t* size)
{
  EFI_STATUS         status;
  EFI_BOOT_SERVICES* bs = SystemTable->BootServices;

  if (!rootDir)
  {
    // Lấy tệp hiện tại để truy vết phân vùng khởi động
    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    EFI_GUID                   loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    status = bs->OpenProtocol(
        ImageHandle, &loadedImageGuid, (void**)&loadedImage,
        ImageHandle, nullptr, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status)) return status;

    // Mở giao thức Tệp hệ thống trên phân vùng khởi động
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem;
    EFI_GUID                         fileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    status = bs->OpenProtocol(
        loadedImage->DeviceHandle, &fileSystemGuid, (void**)&fileSystem,
        ImageHandle, nullptr, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status)) return status;

    // Mở thư mục gốc
    status = fileSystem->OpenVolume(fileSystem, &rootDir);
    if (EFI_ERROR(status)) return status;
  }

  // Mở tệp tin theo đường dẫn
  EFI_FILE_PROTOCOL* fileHandle;
  status = rootDir->Open(rootDir, &fileHandle, (uint16_t*)path, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status)) return status;

  // Lấy kích thước thực tế của tệp qua EFI_FILE_INFO
  uint64_t       infoSize     = 0;
  EFI_FILE_INFO* fileInfo     = nullptr;
  EFI_GUID       fileInfoGuid = EFI_FILE_INFO_ID;

  fileHandle->GetInfo(fileHandle, &fileInfoGuid, &infoSize, nullptr);
  status = bs->AllocatePool(EfiLoaderData, infoSize, (void**)&fileInfo);
  if (EFI_ERROR(status))
    return status;
  fileHandle->GetInfo(fileHandle, &fileInfoGuid, &infoSize, fileInfo);

  *size = fileInfo->FileSize;
  bs->FreePool(fileInfo); // Giải phóng ngay sau khi lấy được kích thước

  uint64_t readSize = *size;

  status = bs->AllocatePool(EfiLoaderData, readSize, (void**)buffer);
  if (EFI_ERROR(status))
    return status;

  // Đọc tệp vào bộ nhớ
  status = fileHandle->Read(fileHandle, &readSize, *buffer);
  fileHandle->Close(fileHandle);
  if (EFI_ERROR(status))
  {
    bs->FreePool(*buffer);
    return status;
  }

  *size = readSize;

  return 0;
}

EFI_GRAPHICS_OUTPUT_PROTOCOL* EFI::setupGraphics()
{
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  if (EFI_ERROR(SystemTable->BootServices->LocateProtocol(&gopGuid, nullptr, (void**)&gop)))
    return nullptr;

  return gop;
}

EFI_STATUS EFI::drawBmp(uint64_t imageAddress, uint64_t x, uint64_t y, uint32_t* imgWidth, uint32_t* imgHeight)
{
  uint8_t* bmpBuffer = (uint8_t*)(uintptr_t)imageAddress;

  BmpFileHeader* fileHeader = (BmpFileHeader*)bmpBuffer;
  if (fileHeader->BfType != 0x4d42)
    return 1;

  BmpInfoHeader* infoHeader = (BmpInfoHeader*)(bmpBuffer + sizeof(BmpFileHeader));
  int32_t        width      = infoHeader->BiWidth;
  int32_t        height     = infoHeader->BiHeight;

  // Xử lý chiều cao âm nghĩa là ảnh lưu từ trên xuống
  bool isTopDown = false;
  if (height < 0)
  {
    isTopDown = true;
    height    = -height;
  }

  if (imgWidth)
    *imgWidth = width;
  if (imgHeight)
    *imgHeight = height;

  // Từ chối các loại ảnh không hỗ trợ
  if (infoHeader->BiCompression != 0 && infoHeader->BiCompression != 3)
    return EFI_UNSUPPORTED;
  if (infoHeader->BiBitCount != 16 && infoHeader->BiBitCount != 24 && infoHeader->BiBitCount != 32)
    return EFI_UNSUPPORTED;

  uint32_t redMask   = 0;
  uint32_t greenMask = 0;
  uint32_t blueMask  = 0;
  uint32_t alphaMask = 0;

  if (infoHeader->BiCompression == 3)
  {
    // Với BI_BITFIELDS (3), các mask luôn nằm ở offset 40 từ đầu infoHeader
    uint32_t* masks = (uint32_t*)((uint8_t*)infoHeader + 40);
    redMask         = masks[0];
    greenMask       = masks[1];
    blueMask        = masks[2];

    // Nếu BiSize đủ lớn cho alpha mask (>= 56, tức BITMAPV3INFOHEADER)
    if (infoHeader->BiSize >= 56)
      alphaMask = masks[3];
  } else if (infoHeader->BiBitCount == 16)
  {
    // Mặc định của 16-bit BMP không nén là RGB555
    redMask   = 0x7C00;
    greenMask = 0x03E0;
    blueMask  = 0x001F;
  }

  auto extractColor = [](uint32_t pixel, uint32_t mask) -> uint8_t {
    if (!mask) return 0;
    int      shift = 0;
    uint32_t m     = mask;
    while ((m & 1) == 0)
    {
      m >>= 1;
      shift++;
    }
    uint32_t val = (pixel & mask) >> shift;
    return (uint8_t)((val * 255) / m);
  };

  uint32_t rowSize   = ((infoHeader->BiBitCount * width + 31) / 32) * 4;
  uint8_t* rawData   = bmpBuffer + fileHeader->BfOffBits;
  uint32_t totalSize = width * height;

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* image;
  EFI_STATUS                     status = SystemTable->BootServices->AllocatePool(EfiLoaderData, totalSize * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (void**)&image);
  if (EFI_ERROR(status))
    return status;

  for (int32_t y = 0; y < height; ++y)
  {
    // Nếu là hướng từ trên xuống, copy xuôi. Nếu hướng từ dưới lên, copy ngược từ dưới lên.
    uint32_t srcY   = isTopDown ? y : (height - 1 - y);
    uint8_t* rowPtr = rawData + (srcY * rowSize);

    for (int32_t x = 0; x < width; ++x)
    {
      uint64_t imageIdx = y * width + x;

      if (infoHeader->BiCompression == 3 || infoHeader->BiBitCount == 16)
      {
        uint32_t pixel = 0;
        if (infoHeader->BiBitCount == 32)
          pixel = rowPtr[x * 4] | (rowPtr[x * 4 + 1] << 8) | (rowPtr[x * 4 + 2] << 16) | (rowPtr[x * 4 + 3] << 24);
        else if (infoHeader->BiBitCount == 16)
          pixel = rowPtr[x * 2] | (rowPtr[x * 2 + 1] << 8);
        else if (infoHeader->BiBitCount == 24)
          pixel = rowPtr[x * 3] | (rowPtr[x * 3 + 1] << 8) | (rowPtr[x * 3 + 2] << 16);

        image[imageIdx].Red      = extractColor(pixel, redMask);
        image[imageIdx].Green    = extractColor(pixel, greenMask);
        image[imageIdx].Blue     = extractColor(pixel, blueMask);
        image[imageIdx].Reserved = extractColor(pixel, alphaMask);
      } else if (infoHeader->BiBitCount == 32)
      {
        image[imageIdx].Blue     = rowPtr[x * 4 + 0];
        image[imageIdx].Green    = rowPtr[x * 4 + 1];
        image[imageIdx].Red      = rowPtr[x * 4 + 2];
        image[imageIdx].Reserved = rowPtr[x * 4 + 3];
      } else if (infoHeader->BiBitCount == 24)
      {
        image[imageIdx].Blue     = rowPtr[x * 3 + 0];
        image[imageIdx].Green    = rowPtr[x * 3 + 1];
        image[imageIdx].Red      = rowPtr[x * 3 + 2];
        image[imageIdx].Reserved = 0;
      }
    }
  }

  status = gop->Blt(
      gop, image,
      EfiBltBufferToVideo,
      0, 0,
      x,
      y,
      width, height, 0);

  // Giải phóng RAM
  SystemTable->BootServices->FreePool(image);

  return status;
}
