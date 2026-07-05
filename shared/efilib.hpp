/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efilib.hpp
 * @brief Khai báo các hàm tiện ích. Một số hàm gỡ lỗi có
 * thể sẽ bị xóa theo sự phát triển của VNExos Bản Nguyên.
 */
#ifndef __SHARED__EFILIB_HPP
#define __SHARED__EFILIB_HPP

#include <efi.hpp>

namespace EFI {
/**
 * Khởi tạo cho thư viện EFI
 * @param _imageHandle Mã định danh cho chương trình đang chạy
 * @param _systemTable Cổng vào thế giới UEFI
 */
void init(EFI_HANDLE _imageHandle, EFI_SYSTEM_TABLE* _systemTable);

/**
 * Quét sạch màn hình
 */
void clear();

/**
 * Hàm in ra trên màn hình UEFI
 * @param format Chuỗi cần in ra có chứa các định dạng
 */
void printf(const char* format, ...);

/**
 * Đợi người dùng nhấn phím
 * @param k Phím đích cần phải bấm, nếu là 0 nghĩa là bất kỳ phím nào
 */
void waitForKey(uint8_t k = 0);

/**
 * Đọc tệp vào một vùng nhớ chỉ định
 * @param path   Đường dẫn tới tệp
 * @param buffer Bộ nhớ mà tệp được ghi vào
 * @param size   Kích thước của tệp
 * @return 0 nếu chạy thành công, 1 thì ngược lại
 */
EFI_STATUS loadFile(const uint16_t* path, void** buffer, uint64_t* size);
} // namespace EFI

#endif // __SHARED__EFILIB_HPP