# =========================================================
# CẤU HÌNH KIẾN TRÚC: aarch64
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

EDK2_TYPE      := aarch64
EDK2_DIR       := $(ROOT_DIR)/firmware/$(EDK2_TYPE)
ALLOW_SHIM     := 0

# ==[ Cờ riêng của bộ xử lý ]===========================================
# ARM64 không có Red-zone nhưng cần ép căn chỉnh ngăn xếp 16-byte.
ARCH_CXX_FLAGS := -mno-implicit-float
# -mno-implicit-float : Cấm tự động dùng các thanh ghi số thực để tránh làm hỏng
#                       trạng thái bộ nhớ khi chạy ngắt.

ARCH_ASM_FLAGS := -march=armv8-a -mgeneral-regs-only

# ==[ Kiểu tệp đầu ra ]=================================================
# aarch64 biên dịch thẳng sang COFF/PE, không cần bước chuyển đổi trung gian.
BOOT_TARGET    := aarch64-unknown-windows
KERN_TARGET    := aarch64-unknown-none-elf
BOOT_LD        := clang++
EFI_BIN_NAME   := BOOTAA64.EFI

# ==[ Cờ liên kết Bộ nạp khởi động ]====================================
BOOT_LDFLAGS   := \
    --target=$(BOOT_TARGET) \
    -nostdlib \
    -fuse-ld=lld-link \
    -Wl,-entry:vnexos_main \
    -Wl,-subsystem:efi_application \
    -Wl,-nodefaultlib \
    -Wl,-dll \
    -Wl,-dynamicbase \
    -Wl,-noimplib
# --target=<...>                 : Chỉ định kiến trúc và hệ điều hành đích để sinh mã máy phù hợp.
# -nostdlib                      : Không dùng các thư viện tiêu chuẩn mặc định.
# -fuse-ld=lld-link              : Ép sử dụng trình liên kết để tạo ra định dạng tệp Windows PE.
# -Wl,-entry:vnexos_main         : Thiết lập hàm vnexos_main làm điểm khởi đầu của chương trình.
# -Wl,-subsystem:efi_application : Định nghĩa phần mềm này là một ứng dụng chạy trên môi trường UEFI.
# -Wl,-nodefaultlib              : Bỏ qua tất cả các thư viện liên kết mặc định của hệ điều hành.
# -Wl,-dll                       : Xuất ra định dạng thư viện liên kết động, bắt buộc đối với tệp EFI.
# -Wl,-dynamicbase               : Cho phép thay đổi ngẫu nhiên địa chỉ nạp trong bộ nhớ để bảo mật.
# -Wl,-noimplib                  : Không tạo ra tệp thư viện nhập khẩu phụ khi phát sinh định dạng thư viện liên kết động.

# ==[ Cấu hình QEMU ]===================================================
QEMU_ARCH_FLAGS := \
    -M virt \
    -cpu cortex-a57 \
    -device virtio-gpu-pci \
    -drive if=pflash,format=raw,readonly=on,file=$(EDK2_DIR)/QEMU_EFI.fd \
    -drive if=pflash,format=raw,file=$(EDK2_DIR)/QEMU_VARS.fd \
    -device tpm-tis-device,tpmdev=tpm0