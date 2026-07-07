/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file acpi.hpp
 * @brief Định nghĩa các cấu trúc bảng phân tầng của ACPI
 */
#ifndef __SHARED__ACPI_HPP
#define __SHARED__ACPI_HPP

#include <stdint.h>

/**
 * CON TRỎ MÔ TẢ HỆ THỐNG GỐC (RSDP)
 */
typedef struct __attribute__((packed))
{
  char     Signature[8]; // Chữ ký: "RSD PTR "
  uint8_t  Checksum;     // Tổng kiểm của phiên bản ACPI 1.0
  char     OEMID[6];     // Chuỗi nhận diện nhà sản xuất (OEM)
  uint8_t  Revision;     // Phiên bản ACPI (0: ACPI 1.0, 2: ACPI 2.0+)
  uint32_t RsdtAddress;  // Địa chỉ vật lý 32-bit trỏ tới bản RSDT

  /* Các trường mở rộng từ ACPI 2.0 trở đi (Dùng cho hệ thống 64-bit) */
  uint32_t Length;           // Kích thước toàn bộ cấu trúc dữ liệu RSDP
  uint64_t XsdtAddress;      // Mấu chốt: Địa chỉ vật lý 64-bit trỏ tới bản mục lục tổng XSDT
  uint8_t  ExtendedChecksum; // Tổng kiểm toàn bộ bản mở rộng
  uint8_t  Reserved[3];      // Các byte dự phòng
} ACPI_RSDP;

/**
 * BẢNG TIÊU ĐỀ ACPI CHUNG
 */
typedef struct __attribute__((packed))
{
  char     Signature[4];    // Chữ ký nhận diện bảng gồm 4 byte (Ví dụ: "FACP", "APIC", "BGRT")
  uint32_t Length;          // Tổng chiều dài của bảng tính cả phần tiêu đề này
  uint8_t  Revision;        // Phiên bản cấu trúc nội bộ của bảng
  uint8_t  Checksum;        // Tổng byte kiểm tra tính toàn vẹn của bảng
  char     OEMID[6];        // ID hãng sản xuất thiết bị phần cứng
  char     OEMTableID[8];   // ID bảng do nhà sản xuất quy định
  uint32_t OEMRevision;     // Phiên bản phần mềm của nhà sản xuất
  uint32_t CreatorID;       // ID của công cụ (ASL Compiler) tạo ra bảng
  uint32_t CreatorRevision; // Phiên bản của công cụ tạo ra bảng
} ACPI_SDT_HEADER;

/**
 * CÁC BẢNG MỤC LỤC CHÍNH
 */
typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  uint32_t        Entry[];
} ACPI_RSDT;

typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  uint64_t        Entry[];
} ACPI_XSDT;

/**
 * CẤU TRÚC ĐỊA CHỈ CHUNG (GAS)
 */
typedef struct __attribute__((packed))
{
  uint8_t  AddressSpaceID; // 0: Hệ thống nhớ (MMIO), 1: Hệ thống I/O
  uint8_t  RegisterBitWidth;
  uint8_t  RegisterBitOffset;
  uint8_t  AccessSize;
  uint64_t Address; // Địa chỉ vật lý 64-bit của thanh ghi
} ACPI_GAS;

/**
 * BẢNG MÔ TẢ ACPI CỐ ĐỊNH (FADT)
 * Quản lý năng lượng (P-States/Sleep)
 * Chữ ký: "FACP"
 */
typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  /* Các địa chỉ trỏ vật lý 32-bit truyền thống */
  uint32_t FirmwareCtrl; // Địa chỉ vật lý 32-bit trỏ tới FACS
  uint32_t Dsdt;         // Địa chỉ vật lý 32-bit trỏ tới DSDT chứa mã bytecode AML
  /* Đặc tính quản lý năng lượng hệ thống */
  uint8_t  Reserved;           // Dự phòng (phải bằng 0)
  uint8_t  PreferredPmProfile; // Hồ sơ năng lượng tối ưu (0: Unspecified, 1: Desktop, 2: Mobile, 3: Workstation...)
  uint16_t SciInt;             // Mã số ngắt ngắt hệ thống điều khiển thanh ghi (SCI)
  uint32_t SmiCmd;             // Địa chỉ cổng I/O để gửi mã lệnh kích hoạt/hủy kích hoạt chế độ ACPI lên SMI
  uint8_t  AcpiEnable;         // Mã lệnh ghi vào thanh ghi SmiCmd để ép CPU bật chế độ ACPI
  uint8_t  AcpiDisable;        // Mã lệnh ghi vào thanh ghi SmiCmd để tắt chế độ ACPI
  uint8_t  S4BiosReq;          // Mã lệnh ghi vào thanh ghi SmiCmd để yêu cầu BIOS chuyển sang trạng thái ngủ đông S4
  uint8_t  PstateCnt;          // Mã lệnh điều khiển trạng thái hiệu năng của CPU
  /* Thanh ghi quản lý năng lượng 32-bit */
  uint32_t Pm1aEvtBlk; // Địa chỉ cơ sở cổng I/O cho Khối Sự kiện nguồn PM1a Event Block
  uint32_t Pm1bEvtBlk; // Địa chỉ cơ sở cổng I/O cho Khối Sự kiện nguồn PM1b Event Block (Nếu có)
  uint32_t Pm1aCntBlk; // Địa chỉ cơ sở cổng I/O cho Khối Điều khiển nguồn PM1a Control Block (Để tắt nguồn/Sleep máy)
  uint32_t Pm1bCntBlk; // Địa chỉ cơ sở cổng I/O cho Khối Điều khiển nguồn PM1b Control Block (Nếu có)
  uint32_t Pm2CntBlk;  // Địa chỉ cơ sở cổng I/O cho Khối Điều khiển PM2 Control Block
  uint32_t PmTmrBlk;   // Địa chỉ cơ sở cổng I/O cho Khối Bộ đếm thời gian ACPI Timer (Xung nhịp 3.579545 MHz)
  uint32_t Gpe0Blk;    // Địa chỉ cơ sở cổng I/O cho Khối Sự kiện chung GPE0 Block
  uint32_t Gpe1Blk;    // Địa chỉ cơ sở cổng I/O cho Khối Sự kiện chung GPE1 Block
  /* Độ dài byte/bit của các khối quản lý năng lượng trên */
  uint8_t Pm1EvtLen;  // Chiều dài bằng byte của khối Pm1a_EVT và Pm1b_EVT (Tối thiểu phải bằng 4)
  uint8_t Pm1CntLen;  // Chiều dài bằng byte của khối Pm1a_CNT và Pm1b_CNT (Tối thiểu phải bằng 2)
  uint8_t Pm2CntLen;  // Chiều dài bằng byte của khối Pm2_CNT (Mặc định bằng 1 nếu có hỗ trợ)
  uint8_t PmTmrLen;   // Chiều dài bằng byte của khối Pm_TMR (Mặc định bằng 4)
  uint8_t Gpe0BlkLen; // Chiều dài bằng byte của khối Gpe0_BLK (Luôn luôn là bội số của 2)
  uint8_t Gpe1BlkLen; // Chiều dài bằng byte của khối Gpe1_BLK (Luôn luôn là bội số của 2)
  uint8_t Gpe1Base;   // Số thứ tự ngắt ngắt bắt đầu của khối GPE1 bên trong hệ thống ACPI tổng
  uint8_t CstCnt;     // Mã lệnh điều khiển chuyển đổi trạng thái nhàn rỗi CPU (C-States) lên SMI
  /* Độ trễ và phân đoạn đệm dữ liệu của phần cứng */
  uint16_t PLvl2Lat;    // Độ trễ thời gian tồi tệ nhất (bằng microsecond) khi CPU thoát trạng thái ngủ C2
  uint16_t PLvl3Lat;    // Độ trễ thời gian tồi tệ nhất (bằng microsecond) khi CPU thoát trạng thái ngủ C3
  uint16_t FlushSize;   // Kích thước vùng nhớ đệm Cache được xả khi gọi lệnh Flush (Phải bằng 0 nếu ko hỗ trợ)
  uint16_t FlushStride; // Bước nhảy dữ liệu khi thực hiện lệnh xả bộ nhớ đệm Cache phần cứng
  /* Kiến trúc khởi động và thanh ghi kiểm trách cố định */
  uint8_t  DutyOffset;   // Vị trí bit bắt đầu của trường điều khiển tỷ lệ xung nhịp CPU trong thanh ghi P_BLK
  uint8_t  DutyWidth;    // Độ rộng bit của trường điều khiển tỷ lệ xung nhịp CPU trong thanh ghi P_BLK
  uint8_t  DayAlrm;      // Chỉ số ngày lưu trong chip RTC (Real-Time Clock) phục vụ tính năng báo thức bật máy
  uint8_t  MonAlrm;      // Chỉ số tháng lưu trong chip RTC phục vụ tính năng báo thức bật máy
  uint8_t  Century;      // Chỉ số thế kỷ (Ví dụ: 20) lưu trong chip RTC phục vụ tính năng cập nhật giờ chuẩn
  uint16_t IapcBootArch; // Cờ cấu hình kiến trúc khởi động của x86 (Bit 0: Legacy Devices, Bit 1: 8042 Keyboard Controller...)
  uint8_t  Reserved2;    // Trường dự phòng (Phải bằng 0)
  uint32_t Flags;        // Cờ tính năng phần cứng cố định hệ thống (Bit 4: PWR_BUTTON_OBJ, Bit 10: WBINVD...)
  /* Cơ chế đặt lại phần cứng thô */
  ACPI_GAS ResetReg;   // Cấu trúc địa chỉ GAS mô tả vị trí thanh ghi Reset cứng (MMIO hoặc Cổng I/O)
  uint8_t  ResetValue; // Giá trị byte cần ghi vào cấu trúc ResetReg để ra lệnh ép Reset phần cứng lập tức
  /* Đặc tính khởi động ARM và phiên bản FADT mở rộng */
  uint8_t  Reserved3[3];     // Các byte dự phòng (Phải bằng 0)
  uint16_t ArmBootArch;      // Cờ cấu hình kiến trúc khởi động của ARM64 (Bit 0: PSCI Hỗ trợ, Bit 1: PSCI Sử dụng SMC...)
  uint8_t  FadtMinorVersion; // Phiên bản phụ của bảng FADT mở rộng (Dùng từ ACPI 5.1 trở đi)
  uint64_t XFirmwareCtrl;    // Bản mở rộng 64-bit: Địa chỉ vật lý trỏ tới bảng FACS (Thay thế hoàn toàn cho trường 32-bit)
  uint64_t XDsdt;            // Bản mở rộng 64-bit: Địa chỉ vật lý dẫn thẳng tới bảng DSDT (Tử huyệt chứa AML mật mã bọc thép)
  ACPI_GAS XPm1aEvtBlk;      // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Sự kiện nguồn PM1a Event Block
  ACPI_GAS XPm1bEvtBlk;      // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Sự kiện nguồn PM1b Event Block
  ACPI_GAS XPm1aCntBlk;      // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Điều khiển nguồn PM1a Control Block
  ACPI_GAS XPm1bCntBlk;      // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Điều khiển nguồn PM1b Control Block
  ACPI_GAS XPm2CntBlk;       // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Điều khiển PM2 Control Block
  ACPI_GAS XPmTmrBlk;        // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Bộ đếm thời gian ACPI Timer
  ACPI_GAS XGpe0Blk;         // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Sự kiện chung GPE0 Block
  ACPI_GAS XGpe1Blk;         // Bản mở rộng 64-bit: Cấu trúc địa chỉ GAS cho Khối Sự kiện chung GPE1 Block
} ACPI_FADT;

/**
 * BẢNG MÔ TẢ APIC ĐA THÀNH PHẦN
 * Cấu hình đa nhân CPU
 * Chữ ký: "APIC"
 */
typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  uint32_t        LocalInterruptControllerAddress; // Địa chỉ vật lý của Local APIC (x86)
  uint32_t        Flags;                           // Cờ PC-AT tương thích
  uint8_t         InterruptControllerStructures[]; // Mảng các bản ghi mô tả phân cấp ngắt đa nhân CPU
} ACPI;

/**
 * BẢNG TÀI NGUYÊN ĐỒ HỌA MỞ RỘNG
 * Lưu thông tin vị trí RAM và tọa độ hiển thị logo gốc của hãng thiết bị
 * Chữ ký: "BGRT"
 */
typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  uint16_t        Version;      // Phiên bản của bảng (Mặc định là 1)
  uint8_t         Status;       // Trạng thái (Bit 0 = 1: Logo đang hiển thị hợp lệ)
  uint8_t         ImageType;    // Loại định dạng ảnh (Luôn là 0: Định dạng BMP)
  uint64_t        ImageAddress; // Địa chỉ vật lý 64-bit chứa file ảnh BMP gốc của hãng
  uint32_t        ImageXOffset; // Tọa độ X để hiển thị ảnh trên màn hình
  uint32_t        ImageYOffset; // Tọa độ Y để hiển thị ảnh trên màn hình
} ACPI_BGRT;

/**
 * BẢNG BỘ ĐỊNH THỜI SỰ KIỆN ĐỘ CHÍNH XÁC CAO
 * Giúp lập lịch trong nhân
 * Chữ ký: "HPET"
 */
typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  uint32_t        EventTimerBlockId;
  ACPI_GAS        BaseAddress; // Địa chỉ cơ sở của HPET Registers
  uint8_t         HpetNumber;
  uint16_t        MainCounterMinimumClockTick;
  uint8_t         PageProtectionAndOemAttribute;
} ACPI_HPET;

/**
 * BẢNG MÔ TẢ KHÔNG GIAN CẤU HÌNH ÁNH XẠ BỘ NHỚ PCI EXPRESS
 * Chứa địa chỉ cơ sở vùng nhớ ánh xạ (MMIO) cho bus PCI Express
 * Chữ ký: "MCFG"
 */
typedef struct __attribute__((packed))
{
  uint64_t BaseAddress; // Địa chỉ cơ sở ánh xạ bộ nhớ của cấu hình PCIe
  uint16_t PciSegmentGroupNumber;
  uint8_t  StartBusNumber;
  uint8_t  EndBusNumber;
  uint32_t Reserved;
} ACPI_MCFG_ENTRY;

typedef struct __attribute__((packed))
{
  ACPI_SDT_HEADER Header;
  uint64_t        Reserved;
  ACPI_MCFG_ENTRY Entries[]; // Danh sách các vùng cấu hình PCIe tương ứng
} ACPI_MCFG;

#endif // __SHARED__ACPI_HPP
