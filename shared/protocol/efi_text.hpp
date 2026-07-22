/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_text.hpp
 * @brief Định nghĩa Giao thức Xuất Văn Bản Đơn Giản, Giao thức Nhập
 *        Văn Bản Đơn Giản và Giao thức Ảnh Đã Tải của UEFI, cung cấp
 *        khả năng hiển thị chữ lên màn hình, đọc phím bấm và truy vấn
 *        thông tin về ảnh đang chạy.
 */
#if !defined(__SHARED__PROTOCOL__EFI_TEXT_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__PROTOCOL__EFI_TEXT_HPP

#include <efi_types.hpp>

/**************************************************************
 * TIÊU ĐỀ BẢNG UEFI
 **************************************************************/

/**
 * Tiêu đề chung đứng đầu mọi bảng dịch vụ UEFI, dùng để xác thực
 * tính toàn vẹn của bảng thông qua chữ ký và tổng kiểm tra CRC32.
 */
typedef struct
{
  uint64_t Signature;
  uint32_t Revision;
  uint32_t HeaderSize;
  uint32_t CRC32;
  uint32_t Reserved;
} EFI_TABLE_HEADER;

/**************************************************************
 * BẢNG CẤU HÌNH UEFI
 **************************************************************/

/**
 * Một mục trong mảng bảng cấu hình của UEFI, cho phép firmware
 * và hệ điều hành trao đổi dữ liệu qua các cặp GUID và con trỏ.
 */
typedef struct
{
  EFI_GUID VendorGuid;
  void*    VendorTable;
} EFI_CONFIGURATION_TABLE;

/**************************************************************
 * GIAO THỨC XUẤT VĂN BẢN ĐƠN GIẢN
 **************************************************************/

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

/** Đặt lại thiết bị xuất văn bản về trạng thái ban đầu. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_RESET)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint8_t                                  ExtendedVerification);

/** Ghi chuỗi Unicode ra màn hình tại vị trí con trỏ hiện tại. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint16_t*                                String);

/** Kiểm tra xem thiết bị có hỗ trợ hiển thị chuỗi ký tự đã cho hay không. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_TEST_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint16_t*                                String);

/** Truy vấn số cột và số hàng của một chế độ hiển thị. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_QUERY_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 ModeNumber,
    uint64_t*                                Columns,
    uint64_t*                                Rows);

/** Chuyển sang chế độ hiển thị văn bản theo số thứ tự. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_SET_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 ModeNumber);

/** Thiết lập màu chữ và màu nền cho văn bản tiếp theo. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_SET_ATTRIBUTE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 Attribute);

/** Xóa sạch toàn bộ màn hình về màu nền hiện tại. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_CLEAR_SCREEN)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This);

/** Di chuyển con trỏ văn bản đến tọa độ cột và hàng chỉ định. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_SET_CURSOR_POSITION)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 Column,
    uint64_t                                 Row);

/** Bật hoặc tắt hiển thị con trỏ nhấp nháy trên màn hình. */
typedef EFI_STATUS(EFI_API* EFI_TEXT_ENABLE_CURSOR)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint8_t                                  Visible);

/**
 * Giao thức Xuất Văn Bản Đơn Giản, cung cấp giao diện dòng lệnh
 * tối thiểu để bootloader giao tiếp với người dùng qua màn hình.
 */
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
  EFI_TEXT_RESET               Reset;
  EFI_TEXT_STRING              OutputString;
  EFI_TEXT_TEST_STRING         TestString;
  EFI_TEXT_QUERY_MODE          QueryMode;
  EFI_TEXT_SET_MODE            SetMode;
  EFI_TEXT_SET_ATTRIBUTE       SetAttribute;
  EFI_TEXT_CLEAR_SCREEN        ClearScreen;
  EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
  EFI_TEXT_ENABLE_CURSOR       EnableCursor;
  void*                        Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

/**************************************************************
 * GIAO THỨC NHẬP VĂN BẢN ĐƠN GIẢN
 **************************************************************/

/**
 * Cấu trúc mô tả một lần bấm phím, gồm mã quét phần cứng
 * và ký tự Unicode tương ứng.
 */
typedef struct
{
  uint16_t ScanCode;
  char16_t UnicodeChar;
} EFI_INPUT_KEY;

struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

/** Đặt lại thiết bị nhập về trạng thái ban đầu. */
typedef EFI_STATUS(EFI_API* EFI_INPUT_RESET)(
    struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This,
    bool                                    ExtendedVerification);

/** Đọc một phím bấm từ bàn phím, trả về lỗi nếu chưa có phím nào. */
typedef EFI_STATUS(EFI_API* EFI_INPUT_READ_KEY)(
    struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This,
    EFI_INPUT_KEY*                          Key);

/**
 * Giao thức Nhập Văn Bản Đơn Giản, cho phép bootloader đọc
 * phím bấm từ bàn phím thông qua firmware UEFI.
 */
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL
{
  EFI_INPUT_RESET    Reset;
  EFI_INPUT_READ_KEY ReadKeyStroke;
  void*              WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

#endif // __SHARED__PROTOCOL__EFI_TEXT_HPP
