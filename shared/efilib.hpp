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
#if !defined(__SHARED__EFILIB_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__EFILIB_HPP

#include <efi.hpp>

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
EFI_STATUS loadFile(const uint16_t* path, uint8_t** buffer, uint64_t* size);

/**
 * Cấu hình đồ họa thông qua giao thức đồ họa
 * @return Cấu trúc giao thức đồ họa
 */
EFI_GRAPHICS_OUTPUT_PROTOCOL* setupGraphics();

/**
 * Đọc tệp BMP trên bộ nhớ và vẽ ra màn hình
 * @param imageAddress Địa chỉ của tệp BMP trên bộ nhớ
 * @param x            Tọa độ chiều ngang của ảnh trên màn hình
 * @param y            Tọa độ chiều dọc của ảnh trên màn hình
 * @return 0 nếu như ảnh BMP được vẽ thành công
 */
EFI_STATUS drawBmp(uint64_t imageAddress, uint64_t x, uint64_t y, uint32_t* imgWidth = nullptr, uint32_t* imgHeight = nullptr);
} // namespace EFI

#endif // __SHARED__EFILIB_HPP