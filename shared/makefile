# =========================================================
# KỊCH BẢN XÂY DỰNG CHO BỘ NẠP KHỞI ĐỘNG
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================
export

include $(ROOT_DIR)/config/compile.mk

SRCS             := $(shell find $(LIB_SHARED_DIR) -type f \( -name "*.cpp" \))
BOOT_OBJS        := $(patsubst $(LIB_SHARED_DIR)/%.cpp,$(BOOT_SHARED_DIR)/%.x86_64.o,$(SRCS)) \
					$(patsubst $(LIB_SHARED_DIR)/%.cpp,$(BOOT_SHARED_DIR)/%.aarch64.o,$(SRCS)) \
					$(patsubst $(LIB_SHARED_DIR)/%.cpp,$(BOOT_SHARED_DIR)/%.riscv64.o,$(SRCS))
SHARED_OBJS      := $(patsubst $(BOOT_SHARED_DIR)/%,$(SHARED_DIR)/%,$(BOOT_OBJS))

$(eval $(call HYPER_BOOT_COMPILE_TEMPLATE,\
	$(LIB_SHARED_DIR),\
	$(BOOT_SHARED_DIR),\
	$(BOOT_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-D__EFI_ALLOWED))

$(eval $(call HYPER_COMPILE_TEMPLATE,\
	$(LIB_SHARED_DIR),\
	$(SHARED_DIR),\
	$(BOOT_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-D__EFI_ALLOWED))

.PHONY: all clean
all: \
	$(BOOT_OBJS) \
	$(SHARED_OBJS)
	@echo "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_RED)Thư viện chung$(MODE_RESET)."

clean:
	@rm -rf $(GRUB_BUILD_DIR)
	@echo "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
