/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_file.hpp
 * @brief Định nghĩa Giao thức Tệp và Giao thức Hệ Thống Tệp Đơn Giản
 *        của UEFI, bao gồm các thao tác mở, đọc, ghi, đóng tệp và
 *        truy vấn thông tin tệp hoặc phân vùng lưu trữ.
 */
#if !defined(__SHARED__PROTOCOL__EFI_FILE_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__PROTOCOL__EFI_FILE_HPP

#include <efi_types.hpp>

/**************************************************************
 * ĐỊNH DANH GIAO THỨC TỆP
 **************************************************************/

/**
 * GUID của Giao thức Hệ Thống Tệp Đơn Giản:
 * 0964E5B2-6459-11D2-8E39-00A0C969723B
 */
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
  {                                          \
      0x0964e5b22,                           \
      0x6459,                                \
      0x11d2,                                \
      {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

/**************************************************************
 * ĐỊNH DANH THÔNG TIN TỆP (GUID)
 **************************************************************/

/** GUID truy vấn siêu dữ liệu của một tệp cụ thể. */
#define EFI_FILE_INFO_ID \
  {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

/** GUID truy vấn thông tin tổng quan của hệ thống tệp. */
#define EFI_FILE_SYSTEM_INFO_ID \
  {0x09576e93, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

/** GUID truy vấn nhãn tên của phân vùng lưu trữ. */
#define EFI_FILE_SYSTEM_VOLUME_LABEL_ID \
  {0xdb47d7d3, 0xfe81, 0x11d3, {0x9a, 0x35, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

/**************************************************************
 * PHIÊN BẢN GIAO THỨC TỆP
 **************************************************************/

#define EFI_FILE_PROTOCOL_REVISION        0x00010000
#define EFI_FILE_PROTOCOL_REVISION2       0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

/**************************************************************
 * CHẾ ĐỘ MỞ TỆP
 **************************************************************/

/** Mở tệp chỉ để đọc. */
#define EFI_FILE_MODE_READ 0x0000000000000001

/** Mở tệp để ghi, yêu cầu đồng thời cờ đọc. */
#define EFI_FILE_MODE_WRITE 0x0000000000000002

/** Tạo tệp mới nếu chưa tồn tại, kết hợp với cờ đọc và ghi. */
#define EFI_FILE_MODE_CREATE 0x8000000000000000

/**************************************************************
 * THUỘC TÍNH TỆP
 **************************************************************/

#define EFI_FILE_READ_ONLY  0x0000000000000001
#define EFI_FILE_HIDDEN     0x0000000000000002
#define EFI_FILE_SYSTEM     0x0000000000000004
#define EFI_FILE_RESERVED   0x0000000000000008
#define EFI_FILE_DIRECTORY  0x0000000000000010
#define EFI_FILE_ARCHIVE    0x0000000000000020
#define EFI_FILE_VALID_ATTR 0x0000000000000037

/**************************************************************
 * CẤU TRÚC THÔNG TIN TỆP
 **************************************************************/

/** Siêu dữ liệu của một tệp: kích thước, thời gian, thuộc tính và tên. */
typedef struct EFI_FILE_INFO
{
  uint64_t Size;
  uint64_t FileSize;
  uint64_t PhysicalSize;
  EFI_TIME CreateTime;
  EFI_TIME LastAccessTime;
  EFI_TIME ModificationTime;
  uint64_t Attribute;
  uint16_t FileName[];
} EFI_FILE_INFO;

/** Thông tin tổng quan về hệ thống tệp đang được gắn kết. */
typedef struct EFI_FILE_SYSTEM_INFO
{
  uint64_t Size;
  uint8_t  ReadOnly;
  uint64_t VolumeSize;
  uint64_t FreeSpace;
  uint32_t BlockSize;
  uint16_t VolumeLabel;
} EFI_FILE_SYSTEM_INFO;

/** Nhãn tên của phân vùng, có thể đọc và ghi qua giao thức tệp. */
typedef struct EFI_FILE_SYSTEM_VOLUME_LABEL
{
  uint16_t VolumeLabel[];
} EFI_FILE_SYSTEM_VOLUME_LABEL;

/** Thẻ thao tác bất đồng bộ dùng cho các hàm đọc/ghi không chặn. */
typedef struct EFI_FILE_IO_TOKEN
{
  EFI_EVENT  Event;
  EFI_STATUS Status;
  uint64_t   BufferSize;
  void*      Buffer;
} EFI_FILE_IO_TOKEN;

/**************************************************************
 * CON TRỎ HÀM CỦA GIAO THỨC TỆP
 **************************************************************/

struct EFI_FILE_PROTOCOL;

/** Mở hoặc tạo một tệp từ thư mục hiện tại. */
typedef EFI_STATUS(EFI_API* EFI_FILE_OPEN)(
    struct EFI_FILE_PROTOCOL*  This,
    struct EFI_FILE_PROTOCOL** NewHandle,
    uint16_t*                  FileName,
    uint64_t                   OpenMode,
    uint64_t                   Attributes);

/** Đóng tệp và giải phóng tài nguyên liên quan. */
typedef EFI_STATUS(EFI_API* EFI_FILE_CLOSE)(struct EFI_FILE_PROTOCOL* This);

/** Xóa tệp khỏi hệ thống tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_DELETE)(struct EFI_FILE_PROTOCOL* This);

/** Đọc dữ liệu từ tệp vào vùng đệm chỉ định. */
typedef EFI_STATUS(EFI_API* EFI_FILE_READ)(
    struct EFI_FILE_PROTOCOL* This,
    uint64_t*                 BufferSize,
    void*                     Buffer);

/** Ghi dữ liệu từ vùng đệm vào tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_WRITE)(
    struct EFI_FILE_PROTOCOL* This,
    uint64_t*                 BufferSize,
    void*                     Buffer);

/** Phiên bản bất đồng bộ của thao tác mở tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_OPEN_EX)(
    struct EFI_FILE_PROTOCOL*  This,
    struct EFI_FILE_PROTOCOL** NewHandle,
    uint16_t*                  FileName,
    uint64_t                   OpenMode,
    uint64_t                   Attributes,
    EFI_FILE_IO_TOKEN*         Token);

/** Phiên bản bất đồng bộ của thao tác đọc tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_READ_EX)(
    struct EFI_FILE_PROTOCOL* This,
    EFI_FILE_IO_TOKEN*        Token);

/** Phiên bản bất đồng bộ của thao tác ghi tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_WRITE_EX)(
    struct EFI_FILE_PROTOCOL* This,
    EFI_FILE_IO_TOKEN*        Token);

/** Phiên bản bất đồng bộ của thao tác đẩy dữ liệu xuống đĩa. */
typedef EFI_STATUS(EFI_API* EFI_FILE_FLUSH_EX)(
    struct EFI_FILE_PROTOCOL* This,
    EFI_FILE_IO_TOKEN*        Token);

/** Đặt lại con trỏ vị trí đọc/ghi trong tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_SET_POSITION)(
    struct EFI_FILE_PROTOCOL* This,
    uint64_t                  Position);

/** Lấy vị trí con trỏ đọc/ghi hiện tại trong tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_GET_POSITION)(
    struct EFI_FILE_PROTOCOL* This,
    uint64_t*                 Position);

/** Truy vấn thông tin của tệp hoặc hệ thống tệp theo loại GUID. */
typedef EFI_STATUS(EFI_API* EFI_FILE_GET_INFO)(
    struct EFI_FILE_PROTOCOL* This,
    EFI_GUID*                 InformationType,
    uint64_t*                 BufferSize,
    void*                     Buffer);

/** Thiết lập thuộc tính hoặc siêu dữ liệu cho tệp. */
typedef EFI_STATUS(EFI_API* EFI_FILE_SET_INFO)(
    struct EFI_FILE_PROTOCOL* This,
    EFI_GUID*                 InformationType,
    uint64_t                  BufferSize,
    void*                     Buffer);

/** Đẩy toàn bộ dữ liệu còn trong bộ đệm xuống thiết bị lưu trữ. */
typedef EFI_STATUS(EFI_API* EFI_FILE_FLUSH)(struct EFI_FILE_PROTOCOL* This);

/**************************************************************
 * CẤU TRÚC GIAO THỨC TỆP
 **************************************************************/

/**
 * Giao thức Tệp cung cấp các thao tác đọc, ghi, duyệt thư mục
 * và quản lý siêu dữ liệu trên hệ thống tệp FAT của UEFI.
 */
typedef struct EFI_FILE_PROTOCOL
{
  uint64_t              Revision;
  EFI_FILE_OPEN         Open;
  EFI_FILE_CLOSE        Close;
  EFI_FILE_DELETE       Delete;
  EFI_FILE_READ         Read;
  EFI_FILE_WRITE        Write;
  EFI_FILE_GET_POSITION GetPosition;
  EFI_FILE_SET_POSITION SetPosition;
  EFI_FILE_GET_INFO     GetInfo;
  EFI_FILE_SET_INFO     SetInfo;
  EFI_FILE_FLUSH        Flush;
  EFI_FILE_OPEN_EX      OpenEx;
  EFI_FILE_READ_EX      ReadEx;
  EFI_FILE_WRITE_EX     WriteEx;
  EFI_FILE_FLUSH_EX     FlushEx;
} EFI_FILE_PROTOCOL;

/**************************************************************
 * GIAO THỨC HỆ THỐNG TỆP ĐƠN GIẢN
 **************************************************************/

struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

/**
 * Mở thư mục gốc của phân vùng, trả về con trỏ Giao thức Tệp
 * để bắt đầu duyệt cây thư mục.
 */
typedef EFI_STATUS(EFI_API* EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
    struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* This,
    EFI_FILE_PROTOCOL**                     Root);

/**
 * Giao thức cấp cao nhất để truy cập hệ thống tệp FAT trên
 * một thiết bị lưu trữ được firmware UEFI nhận ra.
 */
typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
{
  uint64_t                                    Revision;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

#endif // __SHARED__PROTOCOL__EFI_FILE_HPP
