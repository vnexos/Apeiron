/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file efi_boot.hpp
 * @brief Định nghĩa Bảng Dịch Vụ Khởi Động của UEFI, cung cấp toàn bộ
 *        các hàm mà bootloader có thể gọi trước khi bàn giao quyền điều
 *        khiển cho hệ điều hành, bao gồm quản lý bộ nhớ, sự kiện, giao
 *        thức, ảnh thực thi và bộ định thời.
 */
#if !defined(__SHARED__EFI_BOOT_HPP) && defined(__EFI_ALLOWED)
#define __SHARED__EFI_BOOT_HPP

#include "efi_types.hpp"

#include "efi_memory.hpp"
#include "protocol/efi_text.hpp"

/**************************************************************
 * KIỂU PHỤ TRỢ CHO SỰ KIỆN VÀ BỘ ĐỊNH THỜI
 **************************************************************/

/* Định nghĩa của các phương thức mở giao thức */
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

/* Định nghĩa các phân loại sự kiện */
#define EVT_TIMER                         0x80000000
#define EVT_RUNTIME                       0x40000000
#define EVT_NOTIFY_WAIT                   0x00000100
#define EVT_NOTIFY_SIGNAL                 0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES     0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE 0x00000202

/* Định nghĩa độ ưu tiên của các tác vụ */
#define TPL_APPLICATION 4
#define TPL_CALLBACK    8
#define TPL_NOTIFY      16
#define TPL_HIGH_LEVEL  31

/** Hàm được gọi lại khi một sự kiện UEFI được kích hoạt. */
typedef void(EFI_API* EFI_EVENT_NOTIFY)(EFI_EVENT Event, void* Context);

/**
 * Kiểu liệt kê chỉ định hành vi của bộ định thời khi đặt lịch
 * cho một sự kiện.
 */
typedef enum {
  /** Hủy bộ định thời hiện có. */
  TimerCancel,
  /** Kích hoạt sự kiện lặp đi lặp lại theo chu kỳ. */
  TimerPeriod,
  /** Kích hoạt sự kiện một lần sau khoảng thời gian chỉ định. */
  TimerRelative
} EFI_TIMER_DELAY;

/**
 * Kiểu liệt kê xác định tiêu chí tìm kiếm khi duyệt danh sách
 * các bộ điều khiển đã đăng ký giao thức.
 */
typedef enum {
  /** Trả về tất cả tay cầm, bất kể giao thức nào. */
  AllHandles,
  /** Lọc theo thông báo đăng ký nhận sự kiện giao thức. */
  ByRegisterNotify,
  /** Lọc theo GUID của giao thức cụ thể. */
  ByProtocol
} EFI_INTERFACE_SEARCH_TYPE;

/** Thông tin về một đối tượng đang sử dụng một giao thức cụ thể. */
typedef struct
{
  EFI_HANDLE AgentHandle;
  EFI_HANDLE ControllerHandle;
  uint32_t   Attributes;
  uint32_t   OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

/**************************************************************
 * CON TRỎ HÀM DỊCH VỤ KHỞI ĐỘNG
 **************************************************************/

/** Nâng mức ưu tiên tác vụ lên giá trị mới, trả về mức cũ. */
typedef EFI_TPL(EFI_API* EFI_RAISE_TPL)(EFI_TPL NewTpl);

/** Khôi phục mức ưu tiên tác vụ về giá trị trước đó. */
typedef void(EFI_API* EFI_RESTORE_TPL)(EFI_TPL OldTpl);

/** Cấp phát các trang bộ nhớ vật lý theo kiểu và ràng buộc địa chỉ. */
typedef EFI_STATUS(EFI_API* EFI_ALLOCATE_PAGES)(
    EFI_ALLOCATE_TYPE Type,
    EFI_MEMORY_TYPE   MemoryType,
    uint64_t          Pages,
    uint64_t*         Memory);

/** Giải phóng các trang bộ nhớ đã cấp phát trước đó. */
typedef EFI_STATUS(EFI_API* EFI_FREE_PAGES)(uint64_t Memory, uint64_t Pages);

/** Lấy bản đồ bộ nhớ hiện tại, bắt buộc gọi trước ExitBootServices. */
typedef EFI_STATUS(EFI_API* EFI_GET_MEMORY_MAP)(
    uint64_t*              MemoryMapSize,
    EFI_MEMORY_DESCRIPTOR* MemoryMap,
    uint64_t*              MapKey,
    uint64_t*              DescriptorSize,
    uint32_t*              DescriptorVersion);

/** Cấp phát một vùng đệm nhỏ từ vùng nhớ được quản lý bởi UEFI. */
typedef EFI_STATUS(EFI_API* EFI_ALLOCATE_POOL)(
    EFI_MEMORY_TYPE PoolType,
    uint64_t        Size,
    void**          Buffer);

/** Giải phóng vùng đệm đã cấp phát qua AllocatePool. */
typedef EFI_STATUS(EFI_API* EFI_FREE_POOL)(void* Buffer);

/** Tạo một sự kiện mới với hàm gọi lại và mức ưu tiên chỉ định. */
typedef EFI_STATUS(EFI_API* EFI_CREATE_EVENT)(
    uint32_t         Type,
    EFI_TPL          NotifyTpl,
    EFI_EVENT_NOTIFY NotifyFunction,
    void*            NotifyContext,
    EFI_EVENT*       Event);

/** Lập lịch kích hoạt sự kiện theo bộ định thời. */
typedef EFI_STATUS(EFI_API* EFI_SET_TIMER)(
    EFI_EVENT       Event,
    EFI_TIMER_DELAY Type,
    uint64_t        TriggerTime);

/** Chặn thực thi cho đến khi ít nhất một trong các sự kiện được kích hoạt. */
typedef EFI_STATUS(EFI_API* EFI_WAIT_FOR_EVENT)(
    uint64_t   NumberOfEvents,
    EFI_EVENT* Event,
    uint64_t*  Index);

/** Kích hoạt thủ công một sự kiện từ mã nguồn. */
typedef EFI_STATUS(EFI_API* EFI_SIGNAL_EVENT)(EFI_EVENT Event);

/** Đóng và hủy một sự kiện, giải phóng tài nguyên liên quan. */
typedef EFI_STATUS(EFI_API* EFI_CLOSE_EVENT)(EFI_EVENT Event);

/** Kiểm tra xem một sự kiện đã được kích hoạt hay chưa mà không chặn. */
typedef EFI_STATUS(EFI_API* EFI_CHECK_EVENT)(EFI_EVENT Event);

/** Đăng ký cài đặt một giao thức lên một tay cầm thiết bị. */
typedef EFI_STATUS(EFI_API* EFI_INSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE* Handle,
    EFI_GUID*   Protocol,
    uint32_t    InterfaceType,
    void*       Interface);

/** Thay thế cài đặt giao thức hiện có bằng một cài đặt mới. */
typedef EFI_STATUS(EFI_API* EFI_REINSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void*      OldInterface,
    void*      NewInterface);

/** Gỡ bỏ giao thức đã cài đặt khỏi một tay cầm thiết bị. */
typedef EFI_STATUS(EFI_API* EFI_UNINSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void*      Interface);

/** Tra cứu giao thức từ một tay cầm thiết bị theo GUID. */
typedef EFI_STATUS(EFI_API* EFI_HANDLE_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void**     Interface);

typedef void* VoidReservedPointer;

/** Đăng ký để nhận thông báo khi một giao thức được cài đặt mới. */
typedef EFI_STATUS(EFI_API* EFI_REGISTER_PROTOCOL_NOTIFY)(
    EFI_GUID* Protocol,
    EFI_EVENT Event,
    void**    Registration);

/** Tìm kiếm và liệt kê các tay cầm theo tiêu chí tìm kiếm. */
typedef EFI_STATUS(EFI_API* EFI_LOCATE_HANDLE)(
    EFI_INTERFACE_SEARCH_TYPE SearchType,
    EFI_GUID*                 Protocol,
    void*                     SearchKey,
    uint64_t*                 BufferSize,
    EFI_HANDLE*               Buffer);

/** Tìm tay cầm thiết bị phù hợp nhất trên một đường dẫn thiết bị. */
typedef EFI_STATUS(EFI_API* EFI_LOCATE_DEVICE_PATH)(
    EFI_GUID*                  Protocol,
    EFI_DEVICE_PATH_PROTOCOL** DevicePath,
    EFI_HANDLE*                Device);

/** Cài đặt một bảng cấu hình tùy chỉnh vào bảng hệ thống UEFI. */
typedef EFI_STATUS(EFI_API* EFI_INSTALL_CONFIGURATION_TABLE)(
    EFI_GUID* Guid,
    void*     Table);

/** Tải một ảnh thực thi UEFI từ thiết bị hoặc vùng đệm bộ nhớ. */
typedef EFI_STATUS(EFI_API* EFI_IMAGE_LOAD)(
    uint8_t                   BootPolicy,
    EFI_HANDLE                ParentImageHandle,
    EFI_DEVICE_PATH_PROTOCOL* DevicePath,
    void*                     SourceBuffer,
    uint64_t                  SourceSize,
    EFI_HANDLE*               ImageHandle);

/** Bắt đầu thực thi một ảnh đã được tải vào bộ nhớ. */
typedef EFI_STATUS(EFI_API* EFI_IMAGE_START)(
    EFI_HANDLE ImageHandle,
    uint64_t*  ExitDataSize,
    uint16_t** ExitData);

/** Kết thúc ảnh hiện tại và trả mã trạng thái cho ảnh cha. */
typedef EFI_STATUS(EFI_API* EFI_EXIT)(
    EFI_HANDLE ImageHandle,
    EFI_STATUS ExitStatus,
    uint64_t   ExitDataSize,
    uint16_t*  WatchdogData);

/** Gỡ bỏ một ảnh đã tải khỏi bộ nhớ và giải phóng tài nguyên. */
typedef EFI_STATUS(EFI_API* EFI_IMAGE_UNLOAD)(EFI_HANDLE ImageHandle);

/**
 * Kết thúc toàn bộ dịch vụ khởi động và bàn giao quyền điều khiển
 * phần cứng cho hệ điều hành. Phải gọi sau khi lấy bản đồ bộ nhớ.
 */
typedef EFI_STATUS(EFI_API* EFI_EXIT_BOOT_SERVICES)(
    EFI_HANDLE ImageHandle,
    uint64_t   MapKey);

/** Lấy bộ đếm đơn điệu tăng dần, dùng để tạo số ngẫu nhiên hoặc dấu thời gian. */
typedef EFI_STATUS(EFI_API* EFI_GET_NEXT_MONOTONIC_COUNT)(uint64_t* Count);

/** Tạm dừng thực thi trong một khoảng thời gian tính bằng micro giây. */
typedef EFI_STATUS(EFI_API* EFI_STALL)(uint64_t Microseconds);

/** Đặt bộ giám sát thời gian, sẽ kích hoạt khởi động lại nếu không được đặt lại kịp thời. */
typedef EFI_STATUS(EFI_API* EFI_SET_WATCHDOG_TIMER)(
    uint64_t  Timeout,
    uint64_t  WatchdogCode,
    uint64_t  DataSize,
    uint16_t* WatchdogData);

/** Kết nối trình điều khiển vào một bộ điều khiển thiết bị. */
typedef EFI_STATUS(EFI_API* EFI_CONNECT_CONTROLLER)(
    EFI_HANDLE                ControllerHandle,
    EFI_HANDLE*               DriverImageHandle,
    EFI_DEVICE_PATH_PROTOCOL* RemainingDevicePath,
    uint8_t                   Recursive);

/** Ngắt kết nối trình điều khiển khỏi một bộ điều khiển thiết bị. */
typedef EFI_STATUS(EFI_API* EFI_DISCONNECT_CONTROLLER)(
    EFI_HANDLE ControllerHandle,
    EFI_HANDLE DriverImageHandle,
    EFI_HANDLE ChildHandle);

/** Mở giao thức từ một tay cầm, kiểm soát quyền truy cập và sở hữu. */
typedef EFI_STATUS(EFI_API* EFI_OPEN_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void**     Interface,
    EFI_HANDLE AgentHandle,
    EFI_HANDLE ControllerHandle,
    uint32_t   Attributes);

/** Đóng quyền truy cập giao thức đã mở trước đó. */
typedef EFI_STATUS(EFI_API* EFI_CLOSE_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    EFI_HANDLE AgentHandle,
    EFI_HANDLE ControllerHandle);

/** Truy vấn danh sách các đối tượng đang sử dụng một giao thức. */
typedef EFI_STATUS(EFI_API* EFI_OPEN_PROTOCOL_INFORMATION)(
    EFI_HANDLE                            Handle,
    EFI_GUID*                             Protocol,
    EFI_OPEN_PROTOCOL_INFORMATION_ENTRY** EntryBuffer,
    uint32_t*                             EntryCount);

/** Liệt kê tất cả các GUID giao thức được cài đặt trên một tay cầm. */
typedef EFI_STATUS(EFI_API* EFI_PROTOCOLS_PER_HANDLE)(
    EFI_HANDLE  Handle,
    EFI_GUID*** ProtocolBuffer,
    uint32_t*   ProtocolBufferCount);

/** Tìm và cấp phát mảng các tay cầm thỏa tiêu chí tìm kiếm. */
typedef EFI_STATUS(EFI_API* EFI_LOCATE_HANDLE_BUFFER)(
    EFI_INTERFACE_SEARCH_TYPE SearchType,
    EFI_GUID*                 Protocol,
    void*                     SearchKey,
    uint64_t*                 NoHandles,
    EFI_HANDLE**              Buffer);

/** Tìm tay cầm đầu tiên hỗ trợ một giao thức và trả về con trỏ giao thức đó. */
typedef EFI_STATUS(EFI_API* EFI_LOCATE_PROTOCOL)(
    EFI_GUID* Protocol,
    void*     Registration,
    void**    Interface);

/** Cài đặt đồng thời nhiều giao thức lên một tay cầm trong một lần gọi. */
typedef EFI_STATUS(EFI_API* EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES)(
    EFI_HANDLE* Handle, ...);

/** Gỡ bỏ đồng thời nhiều giao thức khỏi một tay cầm trong một lần gọi. */
typedef EFI_STATUS(EFI_API* EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES)(
    EFI_HANDLE Handle, ...);

/** Tính tổng kiểm tra CRC32 cho một vùng dữ liệu. */
typedef EFI_STATUS(EFI_API* EFI_CALCULATE_CRC32)(
    void*     Data,
    uint64_t  DataSize,
    uint32_t* Crc32);

/** Sao chép một vùng bộ nhớ, tương đương memcpy nhưng theo chuẩn UEFI. */
typedef void(EFI_API* EFI_COPY_MEM)(
    void*    Destination,
    void*    Source,
    uint64_t Length);

/** Tô màu một vùng bộ nhớ bằng giá trị byte cố định, tương đương memset. */
typedef void(EFI_API* EFI_SET_MEM)(
    void*    Buffer,
    uint64_t Size,
    uint8_t  Value);

/** Tạo sự kiện mới thuộc một nhóm sự kiện theo GUID. */
typedef EFI_STATUS(EFI_API* EFI_CREATE_EVENT_EX)(
    uint32_t         Type,
    EFI_TPL          NotifyTpl,
    EFI_EVENT_NOTIFY NotifyFunction,
    void*            NotifyContext,
    EFI_GUID*        EventGroup,
    EFI_EVENT*       Event);

/**************************************************************
 * CẤU TRÚC BẢNG DỊCH VỤ KHỞI ĐỘNG
 **************************************************************/

/**
 * Bảng Dịch Vụ Khởi Động tập hợp toàn bộ các hàm mà firmware
 * UEFI cung cấp cho bootloader trong giai đoạn khởi động.
 * Bảng này chỉ hợp lệ trước khi gọi ExitBootServices.
 */
typedef struct
{
  EFI_TABLE_HEADER                           Hdr;
  EFI_RAISE_TPL                              RaiseTPL;
  EFI_RESTORE_TPL                            RestoreTPL;
  EFI_ALLOCATE_PAGES                         AllocatePages;
  EFI_FREE_PAGES                             FreePages;
  EFI_GET_MEMORY_MAP                         GetMemoryMap;
  EFI_ALLOCATE_POOL                          AllocatePool;
  EFI_FREE_POOL                              FreePool;
  EFI_CREATE_EVENT                           CreateEvent;
  EFI_SET_TIMER                              SetTimer;
  EFI_WAIT_FOR_EVENT                         WaitForEvent;
  EFI_SIGNAL_EVENT                           SignalEvent;
  EFI_CLOSE_EVENT                            CloseEvent;
  EFI_CHECK_EVENT                            CheckEvent;
  EFI_INSTALL_PROTOCOL_INTERFACE             InstallProtocolInterface;
  EFI_REINSTALL_PROTOCOL_INTERFACE           ReinstallProtocolInterface;
  EFI_UNINSTALL_PROTOCOL_INTERFACE           UninstallProtocolInterface;
  EFI_HANDLE_PROTOCOL                        HandleProtocol;
  void*                                      VoidReserved;
  EFI_REGISTER_PROTOCOL_NOTIFY               RegisterProtocolNotify;
  EFI_LOCATE_HANDLE                          LocateHandle;
  EFI_LOCATE_DEVICE_PATH                     LocateDevicePath;
  EFI_INSTALL_CONFIGURATION_TABLE            InstallConfigurationTable;
  EFI_IMAGE_LOAD                             LoadImage;
  EFI_IMAGE_START                            StartImage;
  EFI_EXIT                                   Exit;
  EFI_IMAGE_UNLOAD                           UnloadImage;
  EFI_EXIT_BOOT_SERVICES                     ExitBootServices;
  EFI_GET_NEXT_MONOTONIC_COUNT               GetNextMonotonicCount;
  EFI_STALL                                  Stall;
  EFI_SET_WATCHDOG_TIMER                     SetWatchdogTimer;
  EFI_CONNECT_CONTROLLER                     ConnectController;
  EFI_DISCONNECT_CONTROLLER                  DisconnectController;
  EFI_OPEN_PROTOCOL                          OpenProtocol;
  EFI_CLOSE_PROTOCOL                         CloseProtocol;
  EFI_OPEN_PROTOCOL_INFORMATION              OpenProtocolInformation;
  EFI_PROTOCOLS_PER_HANDLE                   ProtocolsPerHandle;
  EFI_LOCATE_HANDLE_BUFFER                   LocateHandleBuffer;
  EFI_LOCATE_PROTOCOL                        LocateProtocol;
  EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES   InstallMultipleProtocolInterfaces;
  EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES UninstallMultipleProtocolInterfaces;
  EFI_CALCULATE_CRC32                        CalculateCrc32;
  EFI_COPY_MEM                               CopyMem;
  EFI_SET_MEM                                SetMem;
  EFI_CREATE_EVENT_EX                        CreateEventEx;
} EFI_BOOT_SERVICES;

#endif // __SHARED__EFI_BOOT_HPP