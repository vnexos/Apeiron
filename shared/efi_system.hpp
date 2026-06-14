/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_system.hpp
 * @brief Định nghĩa Bảng Hệ Thống của UEFI, là cấu trúc trung tâm được
 *        firmware truyền vào hàm efi_main khi bootloader khởi chạy, cung
 *        cấp con trỏ đến mọi dịch vụ và giao thức quan trọng.
 */
#ifndef __SHARED__EFI_SYSTEM_HPP
#define __SHARED__EFI_SYSTEM_HPP

#include "efi_types.hpp"

#include "efi_boot.hpp"
#include "protocol/efi_text.hpp"

/**
 * Bảng Hệ Thống là điểm khởi đầu duy nhất để bootloader truy cập
 * toàn bộ hạ tầng của firmware UEFI, gồm giao diện văn bản, dịch
 * vụ khởi động, dịch vụ thời gian chạy và bảng cấu hình mở rộng.
 */
typedef struct
{
  /** Tiêu đề xác thực tính toàn vẹn của bảng hệ thống. */
  EFI_TABLE_HEADER Hdr;

  /** Chuỗi tên nhà sản xuất firmware, mã hóa Unicode 16 bit. */
  uint16_t* FirmwareVendor;

  /** Phiên bản firmware của nhà sản xuất, không theo chuẩn UEFI. */
  uint32_t FirmwareRevision;

  /** Tay cầm của thiết bị xuất văn bản mặc định (bàn phím). */
  EFI_HANDLE ConsoleInHandle;

  /** Giao thức Nhập Văn Bản Đơn Giản để đọc phím bấm từ bàn phím. */
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL* ConIn;

  /** Tay cầm của thiết bị xuất văn bản mặc định (màn hình). */
  EFI_HANDLE ConsoleOutHandle;

  /** Giao thức Xuất Văn Bản Đơn Giản để ghi văn bản ra màn hình. */
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;

  /** Tay cầm của thiết bị xuất lỗi chuẩn. */
  EFI_HANDLE StandardErrorHandle;

  /** Giao thức Xuất Văn Bản dùng cho thông báo lỗi hệ thống. */
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;

  /** Con trỏ đến Bảng Dịch Vụ Thời Gian Chạy, hợp lệ cả sau ExitBootServices. */
  void* RuntimeServices;

  /** Con trỏ đến Bảng Dịch Vụ Khởi Động, chỉ hợp lệ trước ExitBootServices. */
  EFI_BOOT_SERVICES* BootServices;

  /** Số lượng mục trong mảng bảng cấu hình bên dưới. */
  uint64_t NumberOfTableEntries;

  /** Mảng bảng cấu hình, mỗi mục là cặp GUID và con trỏ dữ liệu. */
  EFI_CONFIGURATION_TABLE* ConfigurationTable;
} EFI_SYSTEM_TABLE;

#endif // __SHARED__EFI_SYSTEM_HPP