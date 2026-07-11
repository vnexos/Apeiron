/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file common.hpp
 * @brief Định nghĩa các cấu trúc dùng chung
 */
#ifndef __SHARED__COMMON_HPP
#define __SHARED__COMMON_HPP

#include <stdint.h>

/* MẪU ĐÓNG GÓI */
// Đóng gói 2 giá trị 32-bit thành 1 biến 64-bit
#define PACK_32_TO_64(low, high) (((uint64_t)(high) << 32) | ((uint32_t)(low) & 0xFFFFFFFF))

/* MẪU RÃ GÓI */
// Lấy 32 bit cao (Nửa cao)
#define UNPACK_HIGH_32(val64) ((uint32_t)((val64) >> 32))
// Lấy 32 bit thấp (Nửa thấp)
#define UNPACK_LOW_32(val64) ((uint32_t)((val64) & 0xFFFFFFFF))

/* CẤU TRÚC DÙNG CHUNG CỦA CÁC TẦNG KHỞI ĐỘNG */
typedef struct
{
  uint64_t framebuffer;
  uint64_t frameBufferSize;
  uint32_t horizontalResolution;
  uint32_t verticalResolution;
  uint64_t OEMLogoSize;     // Nửa cao: chiều cao | nửa thấp: chiều rộng
  uint64_t OEMLogoPosition; // Nửa cao: tọa độ y  | nửa thấp: tọa độ x
  uint64_t logoSize;        // Nửa cao: chiều cao | nửa thấp: chiều rộng
  uint64_t logoPosition;    // Nửa cao: tọa độ y  | nửa thấp: tọa độ x
} ApeironCommonParameters;

#endif // __SHARED__COMMON_HPP
