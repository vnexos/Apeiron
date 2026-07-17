# =========================================================
# CẤU HÌNH KIẾN TRÚC: riscv64
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

EDK2_TYPE      := riscv64
EDK2_DIR       := $(ROOT_DIR)/firmware/$(EDK2_TYPE)

FD_CODE        := RISCV_VIRT_CODE.fd
FD_VARS        := RISCV_VIRT_VARS.fd

# ==[ Cờ riêng của bộ xử lý ]===========================================
# RISC-V cần cấm sử dụng các thanh ghi số thực ở tầng Nhân để tránh làm hỏng
# trạng thái bộ tính toán dấu phẩy động khi xảy ra ngắt.
ARCH_CXX_FLAGS := -march=rv64imac -mno-implicit-float -mabi=lp64 -mcmodel=medany -mno-relax -msmall-data-limit=0 -fno-jump-tables
# -mno-implicit-float : Cấm tự động dùng các thanh ghi số thực để đề phòng biến dạng dữ liệu lúc xảy ra ngắt.
# -mabi=lp64          : Ép sử dụng quy ước gọi hàm dùng các thanh ghi số nguyên 64-bit cho kiến trúc RISC-V.
# -mcmodel=medany     : Dùng địa chỉ tương đối theo bộ đếm chương trình (auipc),
#                       cho phép mã chạy độc lập vị trí (PIE/UEFI).

ARCH_ASM_FLAGS := -march=rv64imac -mabi=lp64 -mcmodel=medany

# ==[ Kiểu tệp đầu ra ]=================================================
# LLVM chưa hỗ trợ sinh mã COFF/PE trực tiếp cho RISC-V, nên phải biên dịch sang
# ELF trước rồi chuyển đổi sang PE bằng công cụ elf2efi_riscv64.py sau khi liên kết.
BOOT_TARGET    := riscv64-unknown-none-elf
KERN_TARGET    := riscv64-unknown-none-elf
BOOT_LD        := ld.lld
EFI_BIN_NAME   := BOOTRISCV64.EFI

