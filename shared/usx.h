/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file usx.hpp
 * @brief Định nghĩa cấu trúc cho Tệp thực thi bảo mật đa
 * năng (Universal Secured Executable - USX)
 */
#ifndef __USX_H
#define __USX_H

#include <stdint.h>

#define USX_ARCH_X86_64      0x0001 // Dòng Vi xử lý Intel, AMD 64-bit
#define USX_ARCH_AARCH64     0x0002 // Dòng Vi xử lý ARM 64-bit
#define USX_ARCH_RISCV64     0x0004 // Dòng Vi xử lý RISC-V 64-bit
#define USX_ARCH_RESERVED    0x0008 // Dành riêng cho Vi xử lý của Việt Nam sau này
#define USX_ARCH_X86         0x0010 // Dòng Vi xử lý Intel 32-bit
#define USX_ARCH_AARCH32     0x0020 // Dòng Vi xử lý ARM 32-bit
#define USX_ARCH_MIPS64      0x0040 // Dòng Vi xử lý MIPS 64-bit
#define USX_ARCH_MIPS32      0x0080 // Dòng Vi xử lý MIPS 32-bit
#define USX_ARCH_PPC64       0x0100 // Dòng Vi xử lý PowerPC 64-bit
#define USX_ARCH_SPARC64     0x0200 // Dòng Vi xử lý SPARC V9
#define USX_ARCH_S390X       0x0400 // Dòng Vi xử lý IBM System/390
#define USX_ARCH_LOONGARCH64 0x0800 // Dòng Vi xử lý LoongArch 64-bit
#define USX_ARCH_IA64        0x1000 // Dòng Vi xử lý Intel Itanium
#define USX_ARCH_AVR         0x2000 // Dòng Vi xử lý Atmel AVR
#define USX_ARCH_SUPERH      0x4000 // Dòng Vi xử lý SuperH (SH4)
#define USX_ARCH_OTHER       0x8000 // Dòng Vi xử lý Dị giáo (Custom/FPGA/VM)

typedef struct __attribute__((packed))
{
  /* NHẬN DIỆN VÀ KIỂM SOÁT (8 Byte) */
  uint8_t  MagicBytes[4]; // Bắt buộc phải là 'U', 'S', 'X', 0
  uint8_t  Version;       // Phiên bản của định dạng USX
  uint8_t  Type;          // Phân loại định dạng USX: 0-Tệp thực thi, 1-Tệp liên kết tĩnh, 2-Tệp liên kết động
  uint16_t TargetArch;    // Các nhân được hỗ trợ trong tệp thực thi này. Sử dụng các Bit trong danh sách USX_ARCH_

  /* THÔNG TIN CHUNG CỦA ỨNG DỤNG (8 Byte) */
  uint8_t  AppVersion[4]; // Phiên bản của ứng dụng lần lượt là: Major, Minor, Patch và Build
  uint16_t Flags;         // Cờ điều khiển: Bit 0 = Mã hóa?, Bit 1 = Đã ký?
  uint16_t HeaderSize;    // Kích thước của Tiêu đề (USXHeader)

  /* ĐIỂM VÀO VÀ ĐIỂM THOÁT (16 Byte) */
  uint64_t EntryPoint; // Điểm bắt đầu của chương trình trên RAM (Địa chỉ ảo)
  uint64_t ImageSize;  // Kích thước cần cấp phát khi tải tệp lên RAM (tính bằng trang)

  /* BẢNG KIẾN TRÚC VÀ CHỮ KÝ (24 Byte) */
  uint64_t ArchTableOffset; // Vị trí của Bảng kiến trúc Vi xử lý
  uint64_t SignatureOffset; // Vị trí của Chữ ký
  uint32_t ArchTableCount;  // Số lượng Vi xử lý mà tệp thực thi này hỗ trợ
  uint32_t SignatureSize;   // Kích thước của Chữ ký

  /* BẢO HIỂM VÀ PHẦN ĐỆM (8 Byte) */
  uint32_t HeaderCRC32; // Tổng kiểm của Tiêu đề
  uint8_t  Reserved[4]; // Phần đệm 4 Byte để Tiêu đề dài đúng 64 Byte
} USXHeader;

typedef struct __attribute__((packed))
{

} USXArch;

#endif // __USX_H
