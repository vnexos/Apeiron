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
  auto putChar = [&](char c) {
    if (idx < 1023) buffer[idx++] = c;
  };

  // Đẩy cả chuỗi vào bộ đệm
  auto putStr = [&](const char* s) {
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
  bs->AllocatePool(EfiLoaderData, infoSize, (void**)&fileInfo);
  fileHandle->GetInfo(fileHandle, &fileInfoGuid, &infoSize, fileInfo);

  *size = fileInfo->FileSize;
  bs->FreePool(fileInfo); // Giải phóng ngay sau khi lấy được kích thước

  uint64_t readSize = *size;

  bs->AllocatePool(EfiLoaderData, readSize, (void**)buffer);

  // Đọc tệp vào bộ nhớ
  status = fileHandle->Read(fileHandle, &readSize, *buffer);
  fileHandle->Close(fileHandle);
  if (EFI_ERROR(status)) return status;

  *size = readSize;

  return 0;
}
