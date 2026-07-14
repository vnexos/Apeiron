/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_mp.hpp
 * @brief Định nghĩa Giao thức Đa Nhân (MP Services) của UEFI, cung cấp
 *        khả năng truy vấn số lượng nhân xử lý, trạng thái và thực thi
 *        mã trên các nhân phụ (Application Processors).
 */
#if !defined(__SHARED__PROTOCOL__EFI_MP_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__PROTOCOL__EFI_MP_HPP

#include <efi_types.hpp>

/**************************************************************
 * ĐỊNH DANH GIAO THỨC ĐA NHÂN
 **************************************************************/

/**
 * GUID của Giao thức Dịch vụ Đa nhân (MP Services Protocol):
 * 3FDDA605-A76E-4F46-AD29-12F4531B3D08
 */
#define EFI_MP_SERVICES_PROTOCOL_GUID \
  {                                   \
      0x3fdda605,                     \
      0xa76e,                         \
      0x4f46,                         \
      {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}}

/**************************************************************
 * HẰNG SỐ CỜ TRẠNG THÁI CPU
 **************************************************************/

/** Cho biết bộ vi xử lý này đóng vai trò là nhân khởi động chính (BSP). */
#define PROCESSOR_AS_BSP_BIT 0x00000001
/** Cho biết bộ vi xử lý này đang được bật (enabled). */
#define PROCESSOR_ENABLED_BIT 0x00000002
/** Cho biết tình trạng sức khỏe của bộ vi xử lý có đang tốt không. */
#define PROCESSOR_HEALTH_STATUS_BIT 0x00000004

/**************************************************************
 * CẤU TRÚC DỮ LIỆU ĐA NHÂN
 **************************************************************/

/**
 * Cấu trúc xác định vị trí vật lý của nhân xử lý trong hệ thống.
 */
typedef struct
{
  uint32_t Package;
  uint32_t Core;
  uint32_t Thread;
} EFI_CPU_PHYSICAL_LOCATION;

/**
 * Cấu trúc chứa thông tin chi tiết về một nhân xử lý.
 */
typedef struct
{
  uint64_t                  ProcessorId;
  uint32_t                  StatusFlag;
  EFI_CPU_PHYSICAL_LOCATION Location;
} EFI_PROCESSOR_INFORMATION;

/**
 * Hàm gọi lại (callback) dùng để chạy trên các nhân phụ (AP).
 *
 * @param Buffer Bộ nhớ tạm dùng để truyền dữ liệu cho hàm con.
 */
typedef void(EFI_API* EFI_AP_PROCEDURE)(void* Buffer);

/**************************************************************
 * ĐỊNH NGHĨA GIAO THỨC ĐA NHÂN
 **************************************************************/

typedef struct _EFI_MP_SERVICES_PROTOCOL EFI_MP_SERVICES_PROTOCOL;

/**
 * Lấy số lượng các nhân xử lý (logical processors) hiện có.
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param NumberOfProcessors    Số lượng nhân xử lý tổng cộng.
 * @param NumberOfEnabledProcessors Số lượng nhân xử lý đang được bật.
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_GET_NUMBER_OF_PROCESSORS)(
    EFI_MP_SERVICES_PROTOCOL* This,
    uint64_t*                 NumberOfProcessors,
    uint64_t*                 NumberOfEnabledProcessors);

/**
 * Lấy thông tin trạng thái và định danh của một nhân xử lý.
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param ProcessorNumber       Chỉ số của nhân cần lấy thông tin.
 * @param ProcessorInfoBuffer   Bộ đệm lưu trữ thông tin nhân xử lý trả về.
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_GET_PROCESSOR_INFO)(
    EFI_MP_SERVICES_PROTOCOL*  This,
    uint64_t                   ProcessorNumber,
    EFI_PROCESSOR_INFORMATION* ProcessorInfoBuffer);

/**
 * Khởi động một hàm trên tất cả các nhân phụ (APs).
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param Procedure             Hàm sẽ được thực thi trên các nhân phụ.
 * @param SingleThread          Nếu TRUE, các nhân phụ sẽ chạy tuần tự. Nếu FALSE, chúng sẽ chạy song song.
 * @param WaitEvent             Sự kiện dùng để chờ khi chế độ bất đồng bộ được bật.
 * @param TimeoutInMicroseconds Thời gian chờ tối đa. Nếu 0, sẽ chờ vô hạn.
 * @param ProcedureArgument     Con trỏ dữ liệu truyền vào hàm Procedure.
 * @param FailedCpuList         (Tùy chọn) Danh sách các nhân chạy thất bại.
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_STARTUP_ALL_APS)(
    EFI_MP_SERVICES_PROTOCOL* This,
    EFI_AP_PROCEDURE          Procedure,
    bool                      SingleThread,
    EFI_EVENT                 WaitEvent,
    uint64_t                  TimeoutInMicroseconds,
    void*                     ProcedureArgument,
    uint64_t**                FailedCpuList);

/**
 * Khởi động một hàm trên một nhân phụ (AP) chỉ định.
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param Procedure             Hàm sẽ được thực thi trên nhân phụ.
 * @param ProcessorNumber       Chỉ số của nhân phụ cần thực thi.
 * @param WaitEvent             Sự kiện chờ (tùy chọn).
 * @param TimeoutInMicroseconds Thời gian chờ tối đa.
 * @param ProcedureArgument     Con trỏ dữ liệu truyền vào hàm Procedure.
 * @param Finished              Trạng thái hoàn thành (tùy chọn).
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_STARTUP_THIS_AP)(
    EFI_MP_SERVICES_PROTOCOL* This,
    EFI_AP_PROCEDURE          Procedure,
    uint64_t                  ProcessorNumber,
    EFI_EVENT                 WaitEvent,
    uint64_t                  TimeoutInMicroseconds,
    void*                     ProcedureArgument,
    bool*                     Finished);

/**
 * Chỉ định một nhân xử lý khác làm nhân khởi động chính (BSP).
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param ProcessorNumber       Chỉ số của nhân sẽ trở thành BSP mới.
 * @param EnableOldBSP          Nếu TRUE, nhân BSP cũ sẽ tiếp tục là nhân phụ (AP).
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_SWITCH_BSP)(
    EFI_MP_SERVICES_PROTOCOL* This,
    uint64_t                  ProcessorNumber,
    bool                      EnableOldBSP);

/**
 * Bật hoặc tắt một nhân phụ (AP).
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param ProcessorNumber       Chỉ số của nhân phụ cần thay đổi.
 * @param EnableAP              TRUE để bật, FALSE để tắt.
 * @param HealthFlag            Cờ kiểm tra tình trạng sức khỏe (tùy chọn).
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_ENABLEDISABLEAP)(
    EFI_MP_SERVICES_PROTOCOL* This,
    uint64_t                  ProcessorNumber,
    bool                      EnableAP,
    uint32_t*                 HealthFlag);

/**
 * Trả về chỉ số (ProcessorNumber) của nhân đang gọi hàm này.
 *
 * @param This                  Con trỏ trỏ đến giao thức EFI_MP_SERVICES_PROTOCOL.
 * @param ProcessorNumber       Biến lưu trữ chỉ số trả về.
 */
typedef EFI_STATUS(EFI_API* EFI_MP_SERVICES_WHOAMI)(
    EFI_MP_SERVICES_PROTOCOL* This,
    uint64_t*                 ProcessorNumber);

/**
 * @brief Giao thức đa nhân (MP Services) dùng để khởi chạy hàm trên các vi xử lý khác nhau.
 */
struct _EFI_MP_SERVICES_PROTOCOL
{
  EFI_MP_SERVICES_GET_NUMBER_OF_PROCESSORS GetNumberOfProcessors;
  EFI_MP_SERVICES_GET_PROCESSOR_INFO       GetProcessorInfo;
  EFI_MP_SERVICES_STARTUP_ALL_APS          StartupAllAPs;
  EFI_MP_SERVICES_STARTUP_THIS_AP          StartupThisAP;
  EFI_MP_SERVICES_SWITCH_BSP               SwitchBSP;
  EFI_MP_SERVICES_ENABLEDISABLEAP          EnableDisableAP;
  EFI_MP_SERVICES_WHOAMI                   WhoAmI;
};

#endif // __SHARED__PROTOCOL__EFI_MP_HPP
