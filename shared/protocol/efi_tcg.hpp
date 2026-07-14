/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_tcg.hpp
 * @brief Định nghĩa Giao thức TCG (Nhóm Điện Toán Tin Cậy) phiên bản 1
 *        và phiên bản 2 của UEFI, cung cấp giao diện lập trình để tương
 *        tác với chip TPM (Mô Đun Nền Tảng Tin Cậy) trên bo mạch chủ,
 *        bao gồm các thao tác đo lường toàn vẹn, mở rộng thanh ghi PCR,
 *        ghi nhật ký sự kiện khởi động và gửi lệnh thẳng tới TPM.
 */
#if !defined(__SHARED__EFI_TCG_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__EFI_TCG_HPP

#include "efi_types.hpp"

/**************************************************************
 * ĐỊNH DANH GIAO THỨC TCG (GUID)
 **************************************************************/

/**
 * GUID của Giao thức TCG phiên bản 1 (dùng với TPM 1.2):
 * F541796D-A62E-4954-A775-9584F61B9CDD
 */
#define EFI_TCG_PROTOCOL_GUID \
  {                           \
      0xf541796d, 0xa62e, 0x4954, {0xa7, 0x75, 0x95, 0x84, 0xf6, 0x1b, 0x9c, 0xdd}}

/**
 * GUID của Giao thức TCG2 (dùng với TPM 2.0):
 * 607F766C-7455-42BE-929E-4D96C2A47C25
 */
#define EFI_TCG2_PROTOCOL_GUID \
  {                            \
      0x607f766c, 0x7455, 0x42be, {0x92, 0x9e, 0x4d, 0x96, 0xc2, 0xa4, 0x7c, 0x25}}

/**************************************************************
 * HẰNG SỐ VÀ MÃ LỖI TCG
 **************************************************************/

/** Thực thi thành công theo chuẩn TCG. */
#define EFI_TCG_SUCCESS 0x0000

/** Thiết bị TPM không hoạt động hoặc bị vô hiệu hóa trong firmware. */
#define EFI_TCG_ERROR_DEACTIVATED 0x0006

/** Lệnh gửi tới TPM không hợp lệ hoặc không được hỗ trợ. */
#define EFI_TCG_ERROR_BAD_COMMAND 0x0030

/** Kích thước thanh ghi PCR không hợp lệ đối với thuật toán đang dùng. */
#define EFI_TCG_INVALID_PARAMETER 0x80280001UL

/** Vùng đệm không đủ lớn để chứa dữ liệu nhật ký sự kiện. */
#define EFI_TCG_BUFFER_TOO_SMALL 0x80280005UL

/** Chỉ số thanh ghi PCR nằm ngoài phạm vi hợp lệ (0 đến 23). */
#define EFI_TCG_INVALID_PCR_INDEX 0x80280006UL

/**************************************************************
 * HẰNG SỐ NHẬT KÝ SỰ KIỆN
 **************************************************************/

/** Định dạng nhật ký tương thích với chuẩn TPM 1.2 (SHA1). */
#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2 0x00000001
/** Định dạng nhật ký thế hệ hai hỗ trợ nhiều thuật toán băm. */
#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_2 0x00000002

/**************************************************************
 * HẰNG SỐ LOẠI SỰ KIỆN KHỞI ĐỘNG
 **************************************************************/

/** Sự kiện ghi lại giá trị CRTM (Gốc Tin Cậy Cốt Lõi) ban đầu. */
#define EV_PREBOOT_CERT 0x00000000
/** Sự kiện không đo lường, chỉ tách phân đoạn nhật ký. */
#define EV_NO_ACTION 0x00000003
/** Sự kiện ghi lại mã nhị phân đã được nạp và thực thi. */
#define EV_EFI_BOOT_SERVICES_APPLICATION 0x80000003
/** Sự kiện ghi lại kết quả kiểm tra biến UEFI của Khởi Động Bảo Mật. */
#define EV_EFI_VARIABLE_AUTHORITY 0x8000000B
/** Sự kiện ghi lại hành động phân tách bảng phân vùng. */
#define EV_EFI_GPT_EVENT 0x80000006
/** Sự kiện đo lường hành động liên quan đến Khởi Động Bảo Mật. */
#define EV_EFI_ACTION 0x80000007
/** Sự kiện ghi lại giá trị biến UEFI vào nhật ký. */
#define EV_EFI_VARIABLE_BOOT 0x80000001
/** Sự kiện đo lường cấu hình phần cứng của máy. */
#define EV_EFI_HANDOFF_TABLES 0x80000009

/**************************************************************
 * HẰNG SỐ CỜ KIỂM SOÁT TPM
 **************************************************************/

/** Yêu cầu cập nhật giá trị thanh ghi PCR bằng phép toán mở rộng. */
#define PE_CFP_EXTEND 0x00000002
/** Yêu cầu ghi sự kiện vào nhật ký mà không cập nhật thanh ghi PCR. */
#define PE_CFP_LOG_ONLY 0x00000004

/**************************************************************
 * THUẬT TOÁN BĂM
 **************************************************************/

/** Mã nhận diện thuật toán SHA1, dùng với TPM 1.2 và thanh ghi PCR cổ điển. */
#define EFI_TCG2_BOOT_HASH_ALG_SHA1 0x00000001
/** Mã nhận diện thuật toán SHA256, khuyến nghị cho TPM 2.0. */
#define EFI_TCG2_BOOT_HASH_ALG_SHA256 0x00000004
/** Mã nhận diện thuật toán SHA384, cung cấp độ bảo mật cao hơn. */
#define EFI_TCG2_BOOT_HASH_ALG_SHA384 0x00000008
/** Mã nhận diện thuật toán SHA512. */
#define EFI_TCG2_BOOT_HASH_ALG_SHA512 0x00000010

/**************************************************************
 * CẤU TRÚC DỮ LIỆU TCG PHIÊN BẢN 1
 **************************************************************/

/** Thông tin năng lực của chip TPM 1.2 được firmware phát hiện. */
typedef struct
{
  /** Số thanh ghi PCR mà TPM cung cấp (thường là 24). */
  uint8_t PCRCount;
  /** Kích thước của một lần đo lường theo thuật toán SHA1 (luôn là 20). */
  uint8_t  HashCount;
  uint16_t Reserved;
  /** Cờ trạng thái thiết bị: bit 0 xác nhận TPM đang hoạt động. */
  uint32_t FeatureFlags;
  /** Địa chỉ vật lý nơi phần cứng TPM được ánh xạ vào bộ nhớ. */
  uint64_t EventLogLocation;
  /** Con trỏ đến mục cuối cùng đã ghi trong nhật ký sự kiện. */
  uint64_t EventLogLastEntry;
} EFI_TCG_BOOT_SERVICE_CAPABILITY;

/** Mô tả một mục nhật ký sự kiện chuẩn TCG 1.2. */
typedef struct
{
  /** Chỉ số thanh ghi PCR bị ảnh hưởng bởi sự kiện này. */
  uint32_t PCRIndex;
  /** Loại sự kiện, ví dụ đo lường mã, biến cấu hình, hành động. */
  uint32_t EventType;
  /** Giá trị SHA1 20 byte của dữ liệu được đo lường. */
  uint8_t Digest[20];
  /** Độ dài thực tế của dữ liệu mô tả sự kiện phía sau. */
  uint32_t EventSize;
  /** Dữ liệu mô tả sự kiện với độ dài thay đổi (EventSize byte). */
  uint8_t Event[];
} TCG_PCClientPCREvent;

/**************************************************************
 * CẤU TRÚC DỮ LIỆU TCG PHIÊN BẢN 2
 **************************************************************/

/** Thông tin năng lực của chip TPM 2.0 được giao thức TCG2 cung cấp. */
typedef struct
{
  /** Kích thước của chính cấu trúc này tính bằng byte. */
  uint8_t Size;
  /** Phiên bản đặc tả TCG mà firmware tuân theo. */
  uint8_t SpecVersionMajor;
  uint8_t SpecVersionMinor;
  /** Số phiên bản errata của đặc tả TCG. */
  uint8_t SpecErrata;
  /** Kích thước con trỏ dùng trong nhật ký (4 hoặc 8 byte). */
  uint8_t UINTNSize;
  /** Mặt nạ bit chỉ định thuật toán băm nào được TPM hỗ trợ. */
  uint32_t SupportedEventLogs;
  /** Bằng 1 nếu TPM đang hoạt động và có thể nhận lệnh. */
  uint8_t TPMPresentFlag;
  /** Mặt nạ bit chỉ định thuật toán băm được phép dùng khi đo lường. */
  uint32_t HashAlgorithmBitmap;
  /** Số lượng ngân hàng PCR mà TPM đang kích hoạt. */
  uint32_t NumberOfPCRBanks;
  /** Mặt nạ bit của các ngân hàng PCR hiện đang hoạt động. */
  uint32_t ActivePCRBanks;
} EFI_TCG2_BOOT_SERVICE_CAPABILITY;

/**
 * Tiêu đề của một mục nhật ký sự kiện TCG 2 (còn gọi là TCG_PCR_EVENT2),
 * được theo sau bởi một mảng các giá trị băm và phần dữ liệu sự kiện.
 */
typedef struct
{
  /** Chỉ số thanh ghi PCR bị ảnh hưởng bởi sự kiện này. */
  uint32_t PCRIndex;
  /** Loại sự kiện theo định nghĩa của chuẩn TCG. */
  uint32_t EventType;
  /**
   * Tiếp theo là TPML_DIGEST_VALUES (danh sách băm nhiều thuật toán)
   * và EVENT_DATA (dữ liệu mô tả sự kiện, kích thước thay đổi).
   * Hai trường này không khai báo trực tiếp do kích thước động.
   */
} TCG_PCR_EVENT2_HDR;

/**
 * Cấu trúc sự kiện đầu vào giao thức TCG2, được điền đầy đủ trước
 * khi truyền vào hàm HashLogExtendEvent hoặc HashLogEvent.
 */
typedef struct
{
  /** Kích thước của toàn bộ cấu trúc này bao gồm cả phần dữ liệu. */
  uint32_t Size;
  /** Tiêu đề sự kiện mô tả thanh ghi PCR và loại sự kiện. */
  TCG_PCR_EVENT2_HDR Header;
  /** Dữ liệu mô tả sự kiện, có kích thước thay đổi, tiếp nối sau đây. */
  uint8_t Event[];
} EFI_TCG2_EVENT;

/**************************************************************
 * CON TRỎ HÀM GIAO THỨC TCG PHIÊN BẢN 1
 **************************************************************/

struct _EFI_TCG_PROTOCOL;

/** Lấy thông tin năng lực của TPM 1.2 và trạng thái nhật ký sự kiện. */
typedef EFI_STATUS(EFI_API* EFI_TCG_STATUS_CHECK)(
    struct _EFI_TCG_PROTOCOL*        This,
    EFI_TCG_BOOT_SERVICE_CAPABILITY* ProtocolCapability,
    uint32_t*                        TCGFeatureFlags,
    uint64_t*                        EventLogLocation,
    uint64_t*                        EventLogLastEntry);

/** Mở rộng thanh ghi PCR bằng giá trị băm và ghi nhật ký không đo lường. */
typedef EFI_STATUS(EFI_API* EFI_TCG_EXTEND)(
    struct _EFI_TCG_PROTOCOL* This,
    uint8_t*                  DigestToExtend,
    uint64_t                  PCRIndex,
    uint8_t*                  NewPCRValue);

/** Ghi một mục nhật ký sự kiện vào nhật ký PCR không kèm đo lường. */
typedef EFI_STATUS(EFI_API* EFI_TCG_LOG_EVENT)(
    struct _EFI_TCG_PROTOCOL* This,
    TCG_PCClientPCREvent*     TCGLogData,
    uint32_t*                 EventNumber,
    uint32_t                  Flags);

/**
 * Tính băm SHA1 của vùng dữ liệu, mở rộng thanh ghi PCR chỉ định
 * và ghi nhật ký sự kiện trong một thao tác nguyên tử duy nhất.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG_HASH_ALL)(
    struct _EFI_TCG_PROTOCOL* This,
    uint8_t*                  HashData,
    uint64_t                  HashDataLen,
    uint32_t                  AlgorithmId,
    uint64_t*                 HashedDataLen,
    uint8_t**                 HashedDataResult);

/**
 * Tính băm, mở rộng thanh ghi PCR và ghi nhật ký cùng lúc, là thao
 * tác đo lường toàn vẹn tiêu chuẩn trong chuỗi tin cậy khởi động.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG_HASH_LOG_EXTEND_EVENT)(
    struct _EFI_TCG_PROTOCOL* This,
    uint8_t*                  HashData,
    uint64_t                  HashDataLen,
    uint32_t                  AlgorithmId,
    TCG_PCClientPCREvent*     TCGLogData,
    uint32_t*                 EventNumber,
    uint64_t*                 EventLogLastEntry);

/** Gửi trực tiếp một lệnh thô theo chuẩn TCG vào phần cứng TPM. */
typedef EFI_STATUS(EFI_API* EFI_TCG_PASS_THROUGH_TO_TPM)(
    struct _EFI_TCG_PROTOCOL* This,
    uint32_t                  TpmInputParameterBlockSize,
    uint8_t*                  TpmInputParameterBlock,
    uint32_t                  TpmOutputParameterBlockSize,
    uint8_t*                  TpmOutputParameterBlock);

/**
 * Giao thức TCG phiên bản 1 cho phép bootloader đo lường và ghi nhật
 * ký tính toàn vẹn của các thành phần khởi động vào chip TPM 1.2.
 */
typedef struct _EFI_TCG_PROTOCOL
{
  EFI_TCG_STATUS_CHECK          StatusCheck;
  EFI_TCG_HASH_ALL              HashAll;
  EFI_TCG_LOG_EVENT             LogEvent;
  EFI_TCG_PASS_THROUGH_TO_TPM   PassThroughToTPM;
  EFI_TCG_HASH_LOG_EXTEND_EVENT HashLogExtendEvent;
} EFI_TCG_PROTOCOL;

/**************************************************************
 * CON TRỎ HÀM GIAO THỨC TCG PHIÊN BẢN 2
 **************************************************************/

struct _EFI_TCG2_PROTOCOL;

/**
 * Lấy thông tin năng lực đầy đủ của TPM 2.0, bao gồm danh sách
 * thuật toán băm được hỗ trợ và cờ trạng thái thiết bị.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_GET_CAPABILITY)(
    struct _EFI_TCG2_PROTOCOL*        This,
    EFI_TCG2_BOOT_SERVICE_CAPABILITY* ProtocolCapability);

/**
 * Truy xuất địa chỉ và kích thước của vùng nhật ký sự kiện theo
 * định dạng được chỉ định, có thể là TCG 1.2 hoặc TCG 2.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_GET_EVENT_LOG)(
    struct _EFI_TCG2_PROTOCOL* This,
    uint32_t                   EventLogFormat,
    uint64_t*                  EventLogLocation,
    uint64_t*                  EventLogLastEntry,
    uint8_t*                   EventLogTruncated);

/**
 * Tính băm dữ liệu theo các thuật toán đang kích hoạt, mở rộng
 * các thanh ghi PCR tương ứng và ghi nhật ký sự kiện, tất cả trong
 * một lần gọi nguyên tử. Đây là hàm đo lường chính của TCG2.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_HASH_LOG_EXTEND_EVENT)(
    struct _EFI_TCG2_PROTOCOL* This,
    uint64_t                   Flags,
    uint64_t                   DataToHash,
    uint64_t                   DataToHashLen,
    EFI_TCG2_EVENT*            EfiTcgEvent);

/**
 * Gửi lệnh thô trực tiếp vào TPM 2.0, truyền khối tham số đầu
 * vào và nhận lại khối kết quả theo chuẩn TPM Software Stack.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_SUBMIT_COMMAND)(
    struct _EFI_TCG2_PROTOCOL* This,
    uint32_t                   InputParameterBlockSize,
    uint8_t*                   InputParameterBlock,
    uint32_t                   OutputParameterBlockSize,
    uint8_t*                   OutputParameterBlock);

/**
 * Lấy khả năng cờ kiểm soát hoạt động (Physical Presence) hiện tại
 * mà hệ điều hành có thể yêu cầu người dùng xác nhận.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_GET_ACTIVE_PCR_BANKS)(
    struct _EFI_TCG2_PROTOCOL* This,
    uint32_t*                  ActivePcrBanks);

/**
 * Thiết lập danh sách ngân hàng PCR sẽ được kích hoạt sau lần
 * khởi động lại tiếp theo. Thay đổi yêu cầu sự xác nhận vật lý.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_SET_ACTIVE_PCR_BANKS)(
    struct _EFI_TCG2_PROTOCOL* This,
    uint32_t                   ActivePcrBanks);

/**
 * Lấy trạng thái yêu cầu Hiện Diện Vật Lý đang chờ xử lý, cho biết
 * hành động nào đang chờ người dùng xác nhận ở lần khởi động kế tiếp.
 */
typedef EFI_STATUS(EFI_API* EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS)(
    struct _EFI_TCG2_PROTOCOL* This,
    uint32_t*                  OperationPresent,
    uint32_t*                  Response);

/**
 * Giao thức TCG phiên bản 2 cung cấp đầy đủ khả năng đo lường đa
 * thuật toán cho TPM 2.0, là nền tảng của cơ chế Khởi Động Bảo Mật
 * và chuỗi tin cậy trên các hệ thống hiện đại.
 */
typedef struct _EFI_TCG2_PROTOCOL
{
  EFI_TCG2_GET_CAPABILITY                     GetCapability;
  EFI_TCG2_GET_EVENT_LOG                      GetEventLog;
  EFI_TCG2_HASH_LOG_EXTEND_EVENT              HashLogExtendEvent;
  EFI_TCG2_SUBMIT_COMMAND                     SubmitCommand;
  EFI_TCG2_GET_ACTIVE_PCR_BANKS               GetActivePcrBanks;
  EFI_TCG2_SET_ACTIVE_PCR_BANKS               SetActivePcrBanks;
  EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS GetResultOfSetActivePcrBanks;
} EFI_TCG2_PROTOCOL;

#endif // __SHARED__EFI_TCG_HPP