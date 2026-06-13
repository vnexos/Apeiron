# =========================================================
# CẤU HÌNH CHUNG ĐỂ BIÊN DỊCH HỆ ĐIỀU HÀNH
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

# ==[ Đường dẫn gốc ]===================================================
CURRENT_CONFIG_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
ROOT_DIR           := $(patsubst %/,%,$(dir $(CURRENT_CONFIG_DIR)))

# ==[ Kiến trúc đích ]==================================================
ARCH ?= x86_64

# Nạp toàn bộ cấu hình riêng cho kiến trúc đã chọn.
# Mỗi tệp arch/<kiến trúc>.mk khai báo: ARCH_CXX_FLAGS, ARCH_ASM_FLAGS,
# BOOT_TARGET, KERN_TARGET, BOOT_LD, EFI_BIN_NAME, BOOT_LDFLAGS, QEMU_ARCH_FLAGS.
include $(CURRENT_CONFIG_DIR)/arch/$(ARCH).mk

# ==[ Đường dẫn đầu ra ]================================================
BUILD_DIR  := $(ROOT_DIR)/build/$(ARCH)
SYSROOT_DIR:= $(ROOT_DIR)/sysroot
KERNEL_DIR := $(SYSROOT_DIR)
KERNEL_FILE:= $(KERNEL_DIR)/apeiron.kern
EFI_BIN_DIR:= $(SYSROOT_DIR)/EFI/BOOT
EFI_BIN    := $(EFI_BIN_DIR)/$(EFI_BIN_NAME)
SHIM_DIR   := $(ROOT_DIR)/shim/$(ARCH)
DISK_IMG   := $(ROOT_DIR)/disk.img

# ==[ Tệp chữ ký số công khai ]==========================================
CERT_DIR   := $(ROOT_DIR)/cert
PUB_FILE   := $(CERT_DIR)/vnexos.crt
DER_FILE   := $(CERT_DIR)/vnexos.cer
PRIV_FILE  := $(CERT_DIR)/vnexos.key

# ==[ Công cụ biên dịch ]================================================
TOOLS_ADDSBAT := python3 $(ROOT_DIR)/tools/add_sbat.py

# clang++ đảm nhận cả biên dịch lẫn điều khiển liên kết.
CXX  := clang++
ASM  := clang++
# ld.lld hỗ trợ biên dịch chéo đa kiến trúc tốt hơn trình liên kết hệ thống.
KERN_LD := ld.lld
QEMU    := qemu-system-$(ARCH)

# ==[ Cờ biên dịch C++ dùng chung ]=====================================
CXX_BASE_FLAGS := \
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
# -std=c++23                      : Sử dụng tiêu chuẩn ngôn ngữ C++23 để tận dụng các tính năng hiện đại.
# -ffreestanding                  : Biên dịch trong môi trường độc lập, không giả định sự tồn tại
#                                   của thư viện chuẩn hay hệ điều hành.
# -fno-exceptions                 : Tắt cơ chế bắt ngoại lệ để giảm kích thước mã máy
#                                   và tránh cần thời gian chạy hỗ trợ.
# -fno-rtti                       : Tắt thông tin kiểu dữ liệu trong thời gian chạy,
#                                   giảm phần phụ trợ cho các lớp.
# -O2                             : Bật tối ưu hóa mã nguồn cấp độ 2 để tăng tốc độ thực thi.
# -Wall -Wextra                   : Bật tất cả các cảnh báo cơ bản và nâng cao để kiểm soát
#                                   mã nguồn nghiêm ngặt.
# -fno-asynchronous-unwind-tables : Bỏ sinh các bảng gỡ rối unwind không cần thiết
#                                   trong môi trường bare-metal.
# -fno-use-cxa-atexit             : Cấm tự động đăng ký hàm hủy toàn cục qua __cxa_atexit
#                                   để tránh lỗi thiếu hàm thời gian chạy ẩn.
# -fno-threadsafe-statics         : Tắt cơ chế khóa khi khởi tạo biến tĩnh nội bộ,
#                                   ngăn sập chương trình khi chưa có đa luồng.
# $(ARCH_CXX_FLAGS)               : Nhúng các cờ đặc thù riêng cho từng kiến trúc chip.

# ==[ Cờ biên dịch theo thành phần ]====================================
# BOOT_FLAGS   : Thêm -fshort-wchar ép kiểu ký tự rộng về 16-bit,
#                tương thích chuỗi UCS-2 của UEFI.
BOOT_FLAGS     := $(CXX_BASE_FLAGS) --target=$(BOOT_TARGET) -fshort-wchar $(BOOT_FLAGS_EXTRA)

# KERNEL_FLAGS : Nhân tự quản lý chuỗi riêng (ASCII/UTF-8) nên giữ nguyên tập cờ cơ bản.
KERNEL_FLAGS   := $(CXX_BASE_FLAGS) --target=$(KERN_TARGET)

BOOT_ASMFLAGS  := $(ARCH_ASM_FLAGS) --target=$(BOOT_TARGET)
KERNEL_ASMFLAGS:= $(ARCH_ASM_FLAGS) --target=$(KERN_TARGET)

# ==[ Cờ liên kết nhân ]================================================

KERN_LDFLAGS := \
    -nostdlib \
    -static \
    -z max-page-size=0x1000
# -nostdlib              : Không liên kết với các thư viện hệ thống hoặc thư viện chuẩn mặc định.
# -static                : Ép buộc liên kết tĩnh toàn bộ, không dùng thư viện liên kết động.
# -z max-page-size=0x1000: Thiết lập kích thước trang bộ nhớ tối đa là 4KB (0x1000),
#                          khớp với cấu trúc phân trang mặc định.

# ==[ Cờ chạy QEMU ]====================================================
QEMU_FLAGS := \
    $(QEMU_ARCH_FLAGS) \
    -m 2G \
    -net none \
    -serial stdio \
    -display sdl \
    -monitor telnet:127.0.0.1:5555,server,nowait