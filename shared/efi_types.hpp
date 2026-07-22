/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_types.hpp
 * @brief Định nghĩa các kiểu dữ liệu nguyên thủy, hằng số và cấu trúc
 *        nền tảng dùng chung cho toàn bộ giao diện lập trình UEFI.
 */
#if !defined(__SHARED__EFI_TYPES_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__EFI_TYPES_HPP

#include <stdint.h>

/**
 * Quy ước gọi hàm theo chuẩn Microsoft ABI, bắt buộc đối với
 * mọi hàm được firmware UEFI xuất ra hoặc nhận vào.
 */
#define EFI_API __attribute__((ms_abi))

/**************************************************************
 * KIỂU DỮ LIỆU NGUYÊN THỦY
 **************************************************************/

typedef uint64_t EFI_STATUS;
typedef void*    EFI_HANDLE;
typedef void*    EFI_EVENT;
typedef uint64_t EFI_TPL;

/**************************************************************
 * HẰNG SỐ TRẠNG THÁI
 **************************************************************/

/** Thực thi thành công, không có lỗi. */
#define EFI_SUCCESS     0
#define EFI_UNSUPPORTED 3

/** Kiểm tra xem mã trạng thái có phải là lỗi hay không. */
#define EFI_ERROR(status) ((status) != EFI_SUCCESS)

/** Chuyển chuỗi ký tự thường sang chuỗi Unicode 16 bit cho UEFI. */
#define EFI_TEXT(str) ((uint16_t*)(L##str))

/** Vùng đệm quá nhỏ để chứa dữ liệu yêu cầu. */
#define EFI_BUFFER_TOO_SMALL 0x8000000000000005

/**************************************************************
 * CẤU TRÚC NỀN TẢNG
 **************************************************************/

/** Định danh giao thức toàn cục dùng để tra cứu dịch vụ UEFI. */
typedef struct
{
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t  Data4[8];
} EFI_GUID;

/** Đường dẫn thiết bị dùng để mô tả vị trí phần cứng hoặc tệp. */
typedef struct
{
  uint8_t Type;
  uint8_t SubType;
  uint8_t Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

/** Cấu trúc thời gian nội bộ của firmware UEFI. */
typedef struct EFI_TIME
{
  uint16_t Year;
  uint8_t  Month;
  uint8_t  Day;
  uint8_t  Hour;
  uint8_t  Minute;
  uint8_t  Second;
  uint8_t  Pad1;
  uint32_t Nanosecond;
  int16_t  TimeZone;
  uint8_t  Daylight;
  uint8_t  PAD2;
} EFI_TIME;

#endif // __SHARED__EFI_TYPES_HPP
