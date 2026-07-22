/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_memory.hpp
 * @brief Định nghĩa các kiểu liệt kê và cấu trúc liên quan đến quản lý
 *        bộ nhớ vật lý trong UEFI, bao gồm phân loại vùng nhớ, chiến
 *        lược cấp phát trang và bộ mô tả bản đồ bộ nhớ.
 */
#if !defined(__SHARED__EFI_MEMORY_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__EFI_MEMORY_HPP

#include <efi_types.hpp>

/**************************************************************
 * PHÂN LOẠI VÙNG BỘ NHỚ
 **************************************************************/

/**
 * Kiểu liệt kê mô tả mục đích sử dụng của từng vùng bộ nhớ
 * trong bản đồ bộ nhớ UEFI.
 */
typedef enum {
  /** Vùng nhớ dành riêng, không được dùng. */
  EfiReservedMemoryType,
  /** Mã thực thi của bootloader. */
  EfiLoaderCode,
  /** Dữ liệu của bootloader. */
  EfiLoaderData,
  /** Mã thực thi của dịch vụ khởi động, giải phóng sau khi thoát. */
  EfiBootServicesCode,
  /** Dữ liệu của dịch vụ khởi động, giải phóng sau khi thoát. */
  EfiBootServicesData,
  /** Mã thực thi của dịch vụ thời gian chạy, cần giữ lại vĩnh viễn. */
  EfiRuntimeServicesCode,
  /** Dữ liệu của dịch vụ thời gian chạy, cần giữ lại vĩnh viễn. */
  EfiRuntimeServicesData,
  /** Vùng nhớ thông thường, sẵn sàng cấp cho hệ điều hành. */
  EfiConventionalMemory,
  /** Vùng nhớ bị lỗi phần cứng, không nên sử dụng. */
  EfiUnusableMemory,
  /** Vùng nhớ bảng ACPI, có thể thu hồi sau khi đọc xong. */
  EfiACPIReclaimMemory,
  /** Vùng nhớ không dễ bay hơi của ACPI, phải giữ lại vĩnh viễn. */
  EfiACPIMemoryNVS,
  /** Vùng ánh xạ thiết bị nhập xuất vào không gian địa chỉ bộ nhớ. */
  EfiMemoryMappedIO,
  /** Vùng ánh xạ cổng nhập xuất vào không gian địa chỉ bộ nhớ. */
  EfiMemoryMappedIOPortSpace,
  /** Vùng nhớ dành riêng cho bộ vi xử lý. */
  EfiPalCode,
  /** Vùng nhớ không bay hơi bền vững (ví dụ: bộ nhớ NVDIMM). */
  EfiPersistentMemory,
  EfiMaxMemoryType
} EFI_MEMORY_TYPE;

/**************************************************************
 * CHIẾN LƯỢC CẤP PHÁT TRANG
 **************************************************************/

/**
 * Kiểu liệt kê chỉ định ràng buộc địa chỉ khi yêu cầu cấp
 * phát các trang bộ nhớ vật lý.
 */
typedef enum {
  /** Cấp phát tại bất kỳ địa chỉ khả dụng nào. */
  AllocateAnyPages,
  /** Cấp phát tại địa chỉ nhỏ hơn hoặc bằng giá trị tối đa chỉ định. */
  AllocateMaxAddress,
  /** Cấp phát tại đúng địa chỉ vật lý đã chỉ định. */
  AllocateAddress,
  MaxAllocateType
} EFI_ALLOCATE_TYPE;

/**************************************************************
 * BỘ MÔ TẢ VÙNG BỘ NHỚ
 **************************************************************/

/**
 * Mô tả một vùng bộ nhớ liên tục trong bản đồ bộ nhớ UEFI,
 * bao gồm kiểu, địa chỉ vật lý, địa chỉ ảo và số trang.
 */
typedef struct
{
  uint32_t Type;
  uint64_t PhysicalStart;
  uint64_t VirtualStart;
  uint64_t NumberOfPages;
  uint64_t Attribute;
} EFI_MEMORY_DESCRIPTOR;

#endif // __SHARED__EFI_MEMORY_HPP