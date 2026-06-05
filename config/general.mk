# =========================================================
# CẤU HÌNH CHUNG ĐỂ BIÊN DỊCH HỆ ĐIỀU HÀNH
# =========================================================

CURRENT_CONFIG_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
ROOT_DIR           := $(patsubst %/,%,$(dir $(CURRENT_CONFIG_DIR)))

ARCH ?= x86_64

BUILD_DIR        := $(ROOT_DIR)/build/$(ARCH)
SHARED_DIR       := $(ROOT_DIR)/shared
SYSROOT_DIR      := $(ROOT_DIR)/sysroot
EFI_BIN_DIR      := $(SYSROOT_DIR)/EFI/BOOT

# Tên của tệp EFI theo đặc tả của UEFI
ifeq ($(ARCH),x86_64)
    EFI_BIN_NAME := BOOTX64.EFI
else ifeq ($(ARCH),aarch64)
    EFI_BIN_NAME := BOOTAA64.EFI
else ifeq ($(ARCH),riscv64)
    EFI_BIN_NAME := BOOTRISCV64.EFI
endif

EFI_BIN          := $(EFI_BIN_DIR)/$(EFI_BIN_NAME)
LINKER_SCRIPT    := $(ROOT_DIR)/linker.ld
KERNEL_BUILD_DIR := $(BUILD_DIR)/kernels
KERNEL_DIR       := $(SYSROOT_DIR)/kernels

# Nạp cấu hình riêng cho từng chip
# (Dùng cờ biên dịch đặc thù cho mỗi chip)
ifeq ($(ARCH),x86_64)
    TARGET         := x86_64-unknown-windows
    ARCH_CXX_FLAGS := -mno-red-zone
    BOOT_LDFLAGS   := --target=$(TARGET) -nostdlib -fuse-ld=lld-link \
                      -Wl,-entry:vnexos_main -Wl,-subsystem:efi_application \
                      -Wl,-nodefaultlib -Wl,-dll -Wl,-dynamicbase -Wl,-noimplib
    LINKER_SCRIPT  := $(ROOT_DIR)/arch/x86_64/linker.ld
else ifeq ($(ARCH),aarch64)
    TARGET          := aarch64-unknown-windows
    # ARM64 không có Red-zone nhưng cần ép căn chỉnh stack 16-byte
    ARCH_CXX_FLAGS  := -mno-implicit-float 
    BOOT_LDFLAGS    := --target=$(TARGET) -nostdlib -fuse-ld=lld-link \
                       -Wl,-entry:vnexos_main -Wl,-subsystem:efi_application \
                       -Wl,-nodefaultlib -Wl,-dll -Wl,-dynamicbase -Wl,-noimplib
    LINKER_SCRIPT   := $(ROOT_DIR)/arch/aarch64/linker.ld
else ifeq ($(ARCH),riscv64)
    TARGET          := riscv64-unknown-windows
    # RISC-V cần cấm sử dụng các thanh ghi số thực (Floating-Point) ở tầng Kernel
    # để tránh làm hỏng trạng thái FPU khi xảy ra Ngắt (Interrupt Context).
    ARCH_CXX_FLAGS  := -mno-implicit-float -mabi=lp64x
    BOOT_LDFLAGS    := --target=$(TARGET) -nostdlib -fuse-ld=lld-link \
                       -Wl,-entry:vnexos_main -Wl,-subsystem:efi_application \
                       -Wl,-nodefaultlib -Wl,-dll -Wl,-dynamicbase -Wl,-noimplib
    LINKER_SCRIPT   := $(ROOT_DIR)/arch/riscv64/linker.ld
endif

# clang++ làm cả compile lẫn link driver
CXX := clang++
LD  := clang++   # ← KHÔNG dùng lld-link hay ld.lld trực tiếp
KLD := ld

CXX_BASE_FLAGS := \
    --target=$(TARGET) \
    -std=c++23 \
    -ffreestanding \
    -fno-exceptions \
    -fno-rtti \
    -mno-red-zone \
    -O2 \
    -Wall -Wextra \
    -fno-asynchronous-unwind-tables \
    -I$(SHARED_DIR) \
    $(ARCH_CXX_FLAGS)

BOOT_FLAGS   := $(CXX_BASE_FLAGS) -fshort-wchar
KERNEL_FLAGS := $(CXX_BASE_FLAGS)
# BOOT_FLAGS   : Thêm '-fshort-wchar' ép kiểu kí tự rộng về 16-bit, tương thích chuỗi UCS-2 của UEFI.
# KERNEL_FLAGS : Nhân tự quản lý chuỗi riêng (ASCII/UTF-8) nên giữ nguyên tập cờ cơ bản.

KERN_LDFLAGS := \
    -T $(LINKER_SCRIPT) \
    -nostdlib \
    -static \
    -z max-page-size=0x1000