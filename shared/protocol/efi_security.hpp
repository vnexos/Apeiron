/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_security.hpp
 * @brief Định nghĩa Giao thức Xác thực Bảo mật Kiến trúc (Security1)
 *        của UEFI, cho phép chặn và tùy chỉnh quá trình xác thực
 *        tệp trước khi được firmware tải lên bộ nhớ.
 */
#if !defined(__SHARED__PROTOCOL__EFI_SECURITY_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__PROTOCOL__EFI_SECURITY_HPP

#include <efi_types.hpp>

/**************************************************************
 * ĐỊNH DANH GIAO THỨC XÁC THỰC BẢO MẬT KIẾN TRÚC
 **************************************************************/

/**
 * GUID của Giao thức Xác thực Bảo mật Kiến trúc:
 * A46423E3-4617-49F1-B9FF-D1BFA9115839
 */
#define EFI_SECURITY_ARCH_PROTOCOL_GUID \
  {                                     \
      0xA46423E3,                       \
      0x4617,                           \
      0x49F1,                           \
      {0xB9, 0xFF, 0xD1, 0xBF, 0xA9, 0x11, 0x58, 0x39}}

/**************************************************************
 * CON TRỎ HÀM XÁC THỰC BẢO MẬT
 **************************************************************/

struct EFI_SECURITY_ARCH_PROTOCOL;

/**
 * Hàm xác thực trạng thái bảo mật của một tệp trước khi được
 * firmware tải. Firmware gọi hàm này bên trong LoadImage() khi
 * chế độ Secure Boot được kích hoạt.
 *
 * @param This                  Con trỏ đến giao thức bảo mật hiện tại.
 * @param AuthenticationStatus  Trạng thái xác thực ban đầu từ firmware.
 * @param File                  Đường dẫn thiết bị của tệp cần xác thực.
 * @return EFI_SUCCESS nếu tệp được phép tải, mã lỗi nếu bị từ chối.
 */
typedef EFI_STATUS(EFI_API* EFI_SECURITY_FILE_AUTHENTICATION_STATE)(
    const EFI_SECURITY_ARCH_PROTOCOL* This,
    uint32_t                          AuthenticationStatus,
    const EFI_DEVICE_PATH_PROTOCOL*   File);

/**************************************************************
 * CẤU TRÚC GIAO THỨC XÁC THỰC BẢO MẬT KIẾN TRÚC
 **************************************************************/

/**
 * Giao thức Xác thực Bảo mật Kiến trúc cung cấp cơ chế để
 * firmware xác thực chữ ký số của tệp EFI trước khi tải.
 * Đây là phiên bản đầu tiên (Security1), chỉ nhận đường dẫn
 * thiết bị mà không nhận nội dung tệp.
 */
typedef struct EFI_SECURITY_ARCH_PROTOCOL
{
  EFI_SECURITY_FILE_AUTHENTICATION_STATE FileAuthenticationState;
} EFI_SECURITY_ARCH_PROTOCOL;

#endif // __SHARED__PROTOCOL__EFI_SECURITY_HPP
