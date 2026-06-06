# =========================================================
# CẤU HÌNH CHUNG ĐỂ BIÊN DỊCH HỆ ĐIỀU HÀNH
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

CURRENT_CONFIG_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
ROOT_DIR           := $(patsubst %/,%,$(dir $(CURRENT_CONFIG_DIR)))

ARCH ?= x86_64

BUILD_DIR        := $(ROOT_DIR)/build/$(ARCH)
SYSROOT_DIR      := $(ROOT_DIR)/sysroot
KERNEL_DIR       := $(SYSROOT_DIR)
KERNEL_FILE      := $(KERNEL_DIR)/apeiron.kern
EFI_BIN_DIR      := $(SYSROOT_DIR)/EFI/BOOT

BOOT_TARGET      := $(ARCH)-unknown-windows
KERN_TARGET      := $(ARCH)-unknown-none-elf
DISK_IMG         := $(ROOT_DIR)/disk.img

# Tên của tệp EFI theo đặc tả của UEFI
ifeq ($(ARCH),x86_64)
    EFI_BIN_NAME := BOOTX64.EFI
else ifeq ($(ARCH),aarch64)
    EFI_BIN_NAME := BOOTAA64.EFI
else ifeq ($(ARCH),riscv64)
    EFI_BIN_NAME := BOOTRISCV64.EFI
endif

EFI_BIN          := $(EFI_BIN_DIR)/$(EFI_BIN_NAME)

# Nạp cấu hình riêng cho từng vi mạch
# (Dùng cờ biên dịch đặc thù cho mỗi vi mạch)
ifeq ($(ARCH),x86_64)
    EDK2_TYPE       := x64
    ARCH_CXX_FLAGS  := -mno-red-zone
    # -mno-red-zone              : Ngăn trình biên dịch dùng vùng bộ nhớ đệm dưới con trỏ ngăn xếp để tránh bị các ngắt phá hỏng.
    ARCH_ASM_FLAGS  := -mllvm -x86-asm-syntax=intel
else ifeq ($(ARCH),aarch64)
    EDK2_TYPE       := $(ARCH)
    # ARM64 không có Red-zone nhưng cần ép căn chỉnh ngăn xếp 16-byte
    ARCH_CXX_FLAGS  := -mno-implicit-float
    # -mno-implicit-float        : Cấm tự động dùng các thanh ghi số thực để tránh làm hỏng trạng thái bộ nhớ khi chạy ngắt.
    ARCH_ASM_FLAGS  := -march=armv8-a -mgeneral-regs-only
else ifeq ($(ARCH),riscv64)
    EDK2_TYPE       := $(ARCH)
    # RISC-V cần cấm sử dụng các thanh ghi số thực (Floating-Point) ở tầng Kernel
    # để tránh làm hỏng trạng thái FPU khi xảy ra Ngắt (Interrupt Context).
    ARCH_CXX_FLAGS  := -mno-implicit-float -mabi=lp64
    # -mno-implicit-float        : Cấm tự động dùng các thanh ghi số thực để đề phòng biến dạng dữ liệu lúc xảy ra ngắt.
    # -mabi=lp64x                : Ép sử dụng quy ước gọi hàm dùng các thanh ghi số nguyên 64-bit cho kiến trúc RISC-V.
    ARCH_ASM_FLAGS  := -march=rv64imac -mabi=lp64
endif

BOOT_LDFLAGS      = --target=$(BOOT_TARGET) -nostdlib -fuse-ld=lld-link \
                    -Wl,-entry:vnexos_main -Wl,-subsystem:efi_application \
                    -Wl,-nodefaultlib -Wl,-dll -Wl,-dynamicbase -Wl,-noimplib
# --target=<...>                 : Chỉ định kiến trúc và hệ điều hành đích để sinh mã máy phù hợp.
# -nostdlib                      : Không dùng các thư viện tiêu chuẩn mặc định.
# -fuse-ld=lld-link              : Ép sử dụng trình liên kết để tạo ra định dạng tệp Windows PE.
# -Wl,-entry:vnexos_main         : Thiết lập hàm vnexos_main làm điểm khởi đầu của chương trình.
# -Wl,-subsystem:efi_application : Định nghĩa phần mềm này là một ứng dụng chạy trên môi trường UEFI.
# -Wl,-nodefaultlib              : Bỏ qua tất cả các thư viện liên kết mặc định của hệ điều hành.
# -Wl,-dll                       : Xuất ra định dạng thư viện liên kết động, bắt buộc đối với tệp EFI.
# -Wl,-dynamicbase               : Cho phép thay đổi ngẫu nhiên địa chỉ nạp trong bộ nhớ để bảo mật.
# -Wl,-noimplib                  : Không tạo ra tệp thư viện nhập khẩu phụ khi phát sinh định dạng thư viện liên kết động.

# clang++ làm cả compile lẫn link driver
CXX              := clang++
ASM              := clang++
QEMU             := qemu-system-$(ARCH)
EDK2_DIR         := $(ROOT_DIR)/firmware/$(EDK2_TYPE)
BOOT_LD          := clang++
# Dùng ld.lld để hỗ trợ biên dịch chéo đa kiến trúc tốt hơn
KERN_LD          := ld.lld

ifeq ($(ARCH),x86_64)
    QEMU_ARCH_FLAGS   := \
        -M q35 \
        -cpu host \
        -enable-kvm \
        -vga virtio \
        -drive if=pflash,format=raw,readonly=on,file=$(EDK2_DIR)/OVMF_CODE.4m.fd \
        -drive if=pflash,format=raw,file=$(EDK2_DIR)/OVMF_VARS.4m.fd
else ifeq ($(ARCH),aarch64)
    QEMU_ARCH_FLAGS   := \
        -M virt \
        -cpu cortex-a57 \
        -device ramfb \
        -device virtio-gpu-pci \
        -drive if=pflash,format=raw,readonly=on,file=$(EDK2_DIR)/QEMU_EFI.fd \
        -drive if=pflash,format=raw,file=$(EDK2_DIR)/QEMU_VARS.fd
else ifeq ($(ARCH),riscv64)
    QEMU_ARCH_FLAGS   := \
        -M virt \
        -cpu rv64 \
        -device virtio-gpu-pci \
        -drive if=pflash,format=raw,readonly=on,file=$(EDK2_DIR)/RISCV_VIRT_CODE.fd \
        -drive if=pflash,format=raw,file=$(EDK2_DIR)/RISCV_VIRT_VARS.fd
endif

QEMU_FLAGS := $(QEMU_ARCH_FLAGS) \
    -m 2G \
    -net none \
    -serial stdio \
    -display sdl \
    -monitor telnet:127.0.0.1:5555,server,nowait

CXX_BASE_FLAGS   := \
    -std=c++23 \
    -ffreestanding \
    -fno-exceptions \
    -fno-rtti \
    -O2 \
    -Wall -Wextra \
    -fno-asynchronous-unwind-tables \
    -fno-use-cxa-atexit \
    -fno-threadsafe-statics \
    $(ARCH_CXX_FLAGS)
# -std=c++23                     : Sử dụng tiêu chuẩn ngôn ngữ C++23 mới nhất để tận dụng các tính năng hiện đại.
# -ffreestanding                 : Biên dịch trong môi trường độc lập, không giả định sự tồn tại của thư viện chuẩn hay OS.
# -fno-exceptions                : Tắt cơ chế bốc ngoại lệ (Exceptions) để giảm kích thước mã máy và tránh cần runtime hỗ trợ.
# -fno-rtti                      : Tắt thông tin kiểu dữ liệu trong thời gian chạy (Runtime Type Info), giảm shadow overhead cho class.
# -O2                            : Bật tối ưu hóa mã nguồn cấp độ 2 để tăng tốc độ thực thi của Hệ điều hành.
# -Wall -Wextra                  : Bật tất cả các cảnh báo (Warnings) cơ bản và nâng cao để kiểm soát code nghiêm ngặt.
# -fno-asynchronous-unwind-tables: Bỏ sinh các bảng gỡ lỗi unwind (call stack) không cần thiết trong môi trường bare-metal.
# -fno-use-cxa-atexit            : Cấm tự động đăng ký hàm hủy toàn cục qua __cxa_atexit (tránh lỗi thiếu hàm runtime ẩn).
# -fno-threadsafe-statics        : Tắt cơ chế khóa (locks) khi khởi tạo biến tĩnh nội bộ, ngăn crash khi chưa có đa luồng.
# $(ARCH_CXX_FLAGS)              : Nhúng các cờ tối ưu đặc thù riêng cho từng kiến trúc chip đã cấu hình ở trên.

BOOT_FLAGS     := $(CXX_BASE_FLAGS) --target=$(BOOT_TARGET) -fshort-wchar
KERNEL_FLAGS   := $(CXX_BASE_FLAGS) --target=$(KERN_TARGET)
# BOOT_FLAGS   : Thêm '-fshort-wchar' ép kiểu kí tự rộng về 16-bit, tương thích chuỗi UCS-2 của UEFI.
# KERNEL_FLAGS : Nhân tự quản lý chuỗi riêng (ASCII/UTF-8) nên giữ nguyên tập cờ cơ bản.
BOOT_ASMFLAGS  := $(ARCH_ASM_FLAGS) --target=$(BOOT_TARGET)
KERNEL_ASMFLAGS:= $(ARCH_ASM_FLAGS) --target=$(KERN_TARGET)

KERN_LDFLAGS := \
    -nostdlib \
    -static \
    -z max-page-size=0x1000
# -nostdlib                      : Không liên kết với các thư viện hệ thống hoặc thư viện tiêu chuẩn mặc định của máy host.
# -static                        : Ép buộc liên kết tĩnh toàn bộ mã nguồn, không sử dụng thư viện liên kết động (.so / .dll).
# -z max-page-size=0x1000        : Thiết lập kích thước trang bộ nhớ tối đa là 4KB (0x1000), khớp với cấu trúc phân trang mặc định.
