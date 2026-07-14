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

SRCS             := $(shell find $(SHARED_DIR) -type f \( -name "*.cpp" \))
OBJS             := $(patsubst $(SHARED_DIR)/%.cpp,$(SHARED_BUILD_DIR)/%.o,$(SRCS))

$(eval $(call COMPILE_TEMPLATE,\
	$(SHARED_DIR),\
	$(SHARED_BUILD_DIR),\
	$(BOOT_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-D__EFI_ALLOWED))

.PHONY: all clean
all: \
	$(OBJS)
	@echo "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_RED)Thư viện chung$(MODE_RESET)."

clean:
	@rm -rf $(GRUB_BUILD_DIR)
	@echo "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
