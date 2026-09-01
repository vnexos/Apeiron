/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_security2.hpp
 * @brief Định nghĩa Giao thức Xác thực Bảo mật Kiến trúc Phiên bản 2
 *        (Security2) của UEFI, mở rộng Security1 bằng cách cung cấp
 *        thêm nội dung tệp để cho phép xác thực sâu hơn.
 */
#if !defined(__SHARED__PROTOCOL__EFI_SECURITY2_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__PROTOCOL__EFI_SECURITY2_HPP

#include <efi_types.hpp>

/**************************************************************
 * ĐỊNH DANH GIAO THỨC XÁC THỰC BẢO MẬT KIẾN TRÚC PHIÊN BẢN 2
 **************************************************************/

/**
 * GUID của Giao thức Xác thực Bảo mật Kiến trúc Phiên bản 2:
 * 94AB2F58-1438-4EF1-9152-18941A3A0E68
 */
#define EFI_SECURITY2_ARCH_PROTOCOL_GUID \
  {                                      \
      0x94ab2f58,                        \
      0x1438,                            \
      0x4ef1,                            \
      {0x91, 0x52, 0x18, 0x94, 0x1a, 0x3a, 0x0e, 0x68}}

/**************************************************************
 * CON TRỎ HÀM XÁC THỰC BẢO MẬT PHIÊN BẢN 2
 **************************************************************/

struct EFI_SECURITY2_ARCH_PROTOCOL;

/**
 * Hàm xác thực tệp với quyền truy cập vào nội dung tệp. Đây là
 * phiên bản mở rộng của Security1, cho phép firmware kiểm tra cả
 * nội dung bên trong tệp EFI chứ không chỉ đường dẫn thiết bị.
 *
 * @param This        Con trỏ đến giao thức bảo mật phiên bản 2.
 * @param DevicePath  Đường dẫn thiết bị của tệp cần xác thực.
 * @param FileBuffer  Vùng đệm chứa nội dung tệp, hoặc nullptr
 *                    nếu tệp chưa được đọc vào bộ nhớ.
 * @param FileSize    Kích thước của vùng đệm tệp tính bằng byte.
 * @param BootPolicy  Cho biết tệp có đang được tải theo chính sách
 *                    khởi động hay không (TRUE = khởi động).
 * @return EFI_SUCCESS nếu tệp được phép tải, mã lỗi nếu bị từ chối.
 */
typedef EFI_STATUS(EFI_API* EFI_SECURITY2_FILE_AUTHENTICATION)(
    const EFI_SECURITY2_ARCH_PROTOCOL* This,
    const EFI_DEVICE_PATH_PROTOCOL*    DevicePath,
    void*                              FileBuffer,
    uint64_t                           FileSize,
    uint8_t                            BootPolicy);

/**************************************************************
 * CẤU TRÚC GIAO THỨC XÁC THỰC BẢO MẬT KIẾN TRÚC PHIÊN BẢN 2
 **************************************************************/

/**
 * Giao thức Xác thực Bảo mật Kiến trúc Phiên bản 2 mở rộng phiên
 * bản 1 bằng cách cung cấp thêm nội dung tệp cho hàm xác thực,
 * cho phép kiểm tra chữ ký số nhúng bên trong tệp PE/COFF.
 */
typedef struct EFI_SECURITY2_ARCH_PROTOCOL
{
  EFI_SECURITY2_FILE_AUTHENTICATION FileAuthentication;
} EFI_SECURITY2_ARCH_PROTOCOL;

#endif // __SHARED__PROTOCOL__EFI_SECURITY2_HPP
