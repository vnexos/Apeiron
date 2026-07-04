# =========================================================
# CẤU HÌNH KIẾN TRÚC: x86_64
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

EDK2_TYPE      := x64
EDK2_DIR       := $(ROOT_DIR)/firmware/$(EDK2_TYPE)
ALLOW_SHIM     := 0

FD_CODE        := OVMF_CODE_4M.fd
FD_VARS        := OVMF_VARS_4M.fd

# ==[ Cờ riêng của bộ xử lý ]===========================================
ARCH_CXX_FLAGS := -mno-red-zone
# -mno-red-zone : Ngăn trình biên dịch dùng vùng bộ nhớ đệm dưới con trỏ ngăn xếp
#                để tránh bị các ngắt phá hỏng.

# Ghi chú: để xuất assembly cú pháp Intel khi debug (chỉ dùng với cờ -S), thêm tay:
#   -mllvm -x86-asm-syntax=intel
# Cờ này không ảnh hưởng đến đầu ra nhị phân khi build bình thường (-c).
ARCH_ASM_FLAGS :=


# ==[ Kiểu tệp đầu ra ]=================================================
# x86_64 biên dịch thẳng sang COFF/PE, không cần bước chuyển đổi trung gian.
BOOT_TARGET    := x86_64-unknown-windows
KERN_TARGET    := x86_64-unknown-none-elf
BOOT_LD        := clang++
EFI_BIN_NAME   := BOOTX64.EFI
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
    -M q35 \
    -cpu host \
    -enable-kvm \
    -vga virtio \
    -drive if=pflash,format=raw,readonly=on,unit=0,file=$(EDK2_DIR)/$(FD_CODE) \
    -drive if=pflash,format=raw,unit=1,file=$(EDK2_DIR)/$(FD_VARS) \
    -device tpm-tis,tpmdev=tpm0