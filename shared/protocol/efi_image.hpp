/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_image.hpp
 * @brief Định nghĩa Giao thức Ảnh đã tải
 */
#ifndef __SHARED__PROTOCOL__EFI_IMAGE_HPP
#define __SHARED__PROTOCOL__EFI_IMAGE_HPP

#include <efi_types.hpp>

/**************************************************************
 * ĐỊNH DANH GIAO THỨC ẢNH ĐÃ TẢI
 **************************************************************/

/**
 * GUID của Giao thức Ảnh Đã Tải:
 * 5B1B31A1-9562-11D2-8E3F-00A0C969723B
 */
#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
  {                                    \
      0x5B1B31A1,                      \
      0x9562,                          \
      0x11D2,                          \
      {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}}

/**************************************************************
 * GIAO THỨC ẢNH ĐÃ TẢI
 **************************************************************/

/**
 * Cấu trúc mô tả thông tin về ảnh UEFI hiện đang được thực thi,
 * bao gồm vị trí nguồn trên đĩa và vị trí trong bộ nhớ.
 */
typedef struct
{
  uint32_t   Revision;
  EFI_HANDLE ParentHandle;
  void*      SystemTable;

  /** Thiết bị lưu trữ nơi ảnh được tải lên. */
  EFI_HANDLE DeviceHandle;
  void*      FilePath;
  void*      Reserved;

  /** Các tùy chọn được truyền vào khi tải ảnh. */
  uint32_t LoadOptionsSize;
  void*    LoadOptions;

  /** Vùng bộ nhớ nơi ảnh được ánh xạ vào. */
  void*    ImageBase;
  uint64_t ImageSize;
  int      ImageCodeType;
  int      ImageDataType;
  void*    Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

#endif // __SHARED__PROTOCOL__EFI_IMAGE_HPP