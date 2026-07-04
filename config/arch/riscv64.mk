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
ALLOW_SHIM     := 0

FD_CODE        := RISCV_VIRT_CODE.fd
FD_VARS        := RISCV_VIRT_VARS.fd

# ==[ Cờ riêng của bộ xử lý ]===========================================
# RISC-V cần cấm sử dụng các thanh ghi số thực ở tầng Nhân để tránh làm hỏng
# trạng thái bộ tính toán dấu phẩy động khi xảy ra ngắt.
ARCH_CXX_FLAGS := -mno-implicit-float -mabi=lp64 -mcmodel=medany
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

# Tắt cảnh báo ms_abi vì RISC-V UEFI không dùng quy ước gọi hàm của Windows.
# Đây là cảnh báo chính xác và vô hại — hàm sẽ dùng quy ước mặc định của RISC-V.
BOOT_FLAGS_EXTRA := -Wno-ignored-attributes

# ==[ Công cụ chuyển đổi ELF sang PE/UEFI ]=============================
# GNU objcopy tạo PE bị hỏng phần đầu tệp nên phải dùng tập lệnh Python tự viết.
RISCV_ELF2EFI := python3 $(ROOT_DIR)/tools/elf2efi_riscv64.py

# ==[ Cờ liên kết Bộ nạp khởi động ]====================================
BOOT_LDFLAGS   := \
    -nostdlib \
    -m elf64lriscv
# -nostdlib      : Không dùng các thư viện tiêu chuẩn mặc định.
# -m elf64lriscv : Chỉ định đích mô phỏng cho RISC-V 64-bit kiểu cuối nhỏ (little-endian).
# -T <tập lệnh liên kết> : Đặt mã nguồn từ địa chỉ ảo=0x1000 để tránh chồng lấp
#                          với phần đầu PE tại địa chỉ 0 (cần bổ sung khi gọi thực tế).

# ==[ Cấu hình QEMU ]===================================================
QEMU_ARCH_FLAGS := \
    -M virt \
    -cpu rv64 \
    -device virtio-gpu-pci \
    -drive if=pflash,format=raw,readonly=on,file=$(EDK2_DIR)/$(FD_CODE) \
    -drive if=pflash,format=raw,file=$(EDK2_DIR)/$(FD_VARS) \
    -device tpm-tis-device,tpmdev=tpm0