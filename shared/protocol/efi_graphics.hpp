/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_graphics.hpp
 * @brief Định nghĩa Giao thức Xuất Đồ Họa (GOP) của UEFI, cung cấp
 *        khả năng truy cập bộ đệm khung hình, truy vấn và thiết lập
 *        chế độ hiển thị, cũng như thực hiện các thao tác sao chép
 *        khối điểm ảnh trực tiếp lên màn hình.
 */
#if !defined(__SHARED__PROTOCOL__EFI_GRAPHICS_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__PROTOCOL__EFI_GRAPHICS_HPP

#include <efi_types.hpp>

/**************************************************************
 * ĐỊNH DANH GIAO THỨC XUẤT ĐỒ HỌA
 **************************************************************/

/**
 * GUID của Giao thức Xuất Đồ Họa:
 * 9042A9DE-23DC-4A38-96FB-7ADED080516A
 */
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
  {                                       \
      0x9042a9de,                         \
      0x23dc,                             \
      0x4a38,                             \
      {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}

/**************************************************************
 * ĐỊNH DẠNG ĐIỂM ẢNH
 **************************************************************/

/**
 * Kiểu liệt kê mô tả cách các kênh màu được sắp xếp trong bộ nhớ
 * cho mỗi điểm ảnh trên màn hình.
 */
typedef enum {
  /** Đỏ, Lục, Lam, rồi đến byte dự trữ, mỗi kênh 8 bit. */
  PixelRedGreenBlueReserved8BitPerColor,
  /** Lam, Lục, Đỏ, rồi đến byte dự trữ, mỗi kênh 8 bit. */
  PixelBlueGreenRedReserved8BitPerColor,
  /** Mặt nạ bit tùy chỉnh, xem thêm trong EFI_PIXEL_BITMASK. */
  PixelBitMask,
  /** Chỉ hỗ trợ sao chép khối, không truy cập trực tiếp bộ đệm. */
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

/**
 * Mặt nạ bit xác định vị trí từng kênh màu trong một điểm ảnh
 * khi chế độ PixelBitMask được sử dụng.
 */
typedef struct
{
  uint32_t RedMask;
  uint32_t GreenMask;
  uint32_t BlueMask;
  uint32_t ReservedMask;
} EFI_PIXEL_BITMASK;

/**
 * Các thao tác thông qua hàm Blt
 */
typedef enum {
  EfiBltVideoFill,                 // 0: Tô đặc một khối chữ nhật trên màn hình bằng 1 màu duy nhất (Xóa màn hình).
  EfiBltVideoToBltBuffer,          // 1: Đọc ngược pixel từ màn hình (VRAM) lưu vào mảng RAM thường.
  EfiBltBufferToVideo,             // 2: Đẩy một mảng pixel từ RAM thường lên màn hình (Vẽ ảnh/logo).
  EfiBltVideoToVideo,              // 3: Copy trực tiếp một khối pixel từ vùng này sang vùng khác trên VRAM.
  EfiGraphicsOutputBltOperationMax // Giá trị lính canh để kiểm tra giới hạn biên của enum.
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

/**
 * Lưu các giá trị của Pixel
 */
typedef struct
{
  uint8_t Blue;     // Kênh màu Xanh lam (0 - 255)
  uint8_t Green;    // Kênh màu Xanh lục (0 - 255)
  uint8_t Red;      // Kênh màu Đỏ       (0 - 255)
  uint8_t Reserved; // Kênh dự phòng / Alpha channel (Thường để mặc định bằng 0)
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

/**************************************************************
 * THÔNG TIN CHẾ ĐỘ HIỂN THỊ
 **************************************************************/

/**
 * Mô tả đặc điểm của một chế độ hiển thị cụ thể, bao gồm độ phân
 * giải và định dạng điểm ảnh.
 */
typedef struct
{
  uint32_t                  Version;
  uint32_t                  HorizontalResolution;
  uint32_t                  VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  EFI_PIXEL_BITMASK         PixelInformation;
  uint32_t                  PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

/**
 * Trạng thái hiện tại của Giao thức Xuất Đồ Họa, bao gồm địa chỉ
 * vật lý và kích thước của bộ đệm khung hình.
 */
typedef struct
{
  uint32_t                              MaxMode;
  uint32_t                              Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
  uint64_t                              SizeOfInfo;
  uint64_t                              FrameBufferBase;
  uint64_t                              FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

/**************************************************************
 * CON TRỎ HÀM CỦA GIAO THỨC XUẤT ĐỒ HỌA
 **************************************************************/

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL;

/** Truy vấn thông tin chi tiết của một chế độ hiển thị theo số thứ tự. */
typedef EFI_STATUS(EFI_API* EFI_GRAPHICS_OUTPUT_QUERY_MODE)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL*  This,
    uint32_t                               ModeNumber,
    uint64_t*                              SizeOfInfo,
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info);

/** Chuyển sang chế độ hiển thị theo số thứ tự, cập nhật bộ đệm khung hình. */
typedef EFI_STATUS(EFI_API* EFI_GRAPHICS_OUTPUT_SET_MODE)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL* This,
    uint32_t                              ModeNumber);

/**
 * Sao chép hoặc tô màu khối điểm ảnh hình chữ nhật, hỗ trợ các thao
 * tác từ vùng đệm lên màn hình, từ màn hình vào vùng đệm, hoặc tô màu.
 */
typedef EFI_STATUS(EFI_API* EFI_GRAPHICS_OUTPUT_BLT)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL* This,
    void*                                 BltBuffer,
    uint32_t                              BltOperation,
    uint64_t                              SourceX,
    uint64_t                              SourceY,
    uint64_t                              DestinationX,
    uint64_t                              DestinationY,
    uint64_t                              Width,
    uint64_t                              Height,
    uint64_t                              Delta);

/**************************************************************
 * CẤU TRÚC GIAO THỨC XUẤT ĐỒ HỌA
 **************************************************************/

/**
 * Giao thức Xuất Đồ Họa là cửa ngõ chính để bootloader truy cập
 * màn hình ở chế độ đồ họa thông qua firmware UEFI.
 */
typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL
{
  EFI_GRAPHICS_OUTPUT_QUERY_MODE     QueryMode;
  EFI_GRAPHICS_OUTPUT_SET_MODE       SetMode;
  EFI_GRAPHICS_OUTPUT_BLT            Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

#endif // __SHARED__PROTOCOL__EFI_GRAPHICS_HPP
