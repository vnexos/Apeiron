/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi.hpp
 * @brief Đây là cầu nối giữa mã nguồn C++ và phần cứng thông qua
 *        firmware UEFI.
 */
#ifndef __SHARED__EFI_HPP
#define __SHARED__EFI_HPP

/** Kiểu dữ liệu nguyên thủy, hằng số và cấu trúc thời gian. */
#include <efi_types.hpp>

/** Giao thức Tệp và Giao thức Hệ Thống Tệp Đơn Giản. */
#include <protocol/efi_file.hpp>

/** Giao thức Xuất/Nhập Văn Bản, Ảnh Đã Tải và Bảng Cấu Hình. */
#include <protocol/efi_text.hpp>

/** Giao thức Xuất Đồ Họa (GOP) và các cấu trúc điểm ảnh. */
#include <protocol/efi_graphics.hpp>

/** Giao thức TCG phiên bản 1 (TPM 1.2) và phiên bản 2 (TPM 2.0). */
#include <protocol/efi_tcg.hpp>

/** Giao thức tệp ảnh đã tải */
#include <protocol/efi_image.hpp>

/** Kiểu bộ nhớ, chiến lược cấp phát và bộ mô tả bản đồ bộ nhớ. */
#include <efi_memory.hpp>

/** Bảng Dịch Vụ Khởi Động với đầy đủ các hàm UEFI giai đoạn khởi động. */
#include <efi_boot.hpp>

/** Bảng Hệ Thống, điểm nhập trung tâm khi bootloader được gọi. */
#include <efi_system.hpp>

/** Giao thức Đa Nhân (MP Services) cho phép tương tác với các APs. */
#include <protocol/efi_mp.hpp>

#endif // __SHARED__EFI_HPP
