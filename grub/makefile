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
include $(ROOT_DIR)/config/linker.mk

GRUB_DIR       := $(ROOT_DIR)/grub
GRUB_BUILD_DIR := $(BUILD_DIR)/grub
SRCS           := $(shell find $(GRUB_DIR)/src -type f \( -name "*.cpp" -o -name "*.s" \))
X86_64_OBJS    := $(patsubst $(GRUB_DIR)/src/%.cpp,$(GRUB_BUILD_DIR)/%.x86_64.o,$(filter %.cpp,$(SRCS))) \
				  $(shell find $(BOOT_SHARED_DIR) -type f \( -name "*.x86_64.o" \))
AARCH64_OBJS   := $(patsubst $(GRUB_DIR)/src/%.cpp,$(GRUB_BUILD_DIR)/%.aarch64.o,$(filter %.cpp,$(SRCS))) \
				  $(shell find $(BOOT_SHARED_DIR) -type f \( -name "*.aarch64.o" \))
RISCV64_OBJS   := $(patsubst $(GRUB_DIR)/src/%.cpp,$(GRUB_BUILD_DIR)/%.riscv64.o,$(filter %.cpp,$(SRCS))) \
				  $(shell find $(BOOT_SHARED_DIR) -type f \( -name "*.riscv64.o" \))
EFI_BINS       := $(EFI_BIN_DIR)/BOOTX64.EFI \
				  $(EFI_BIN_DIR)/BOOTAA64.EFI \
				  $(EFI_BIN_DIR)/BOOTRISCV64.EFI
GRUB_LINKER_SCRIPT := $(GRUB_DIR)/linker_grub_riscv64.ld

$(eval $(call HYPER_BOOT_COMPILE_TEMPLATE,\
	$(GRUB_DIR)/src,\
	$(GRUB_BUILD_DIR),\
	$(CXX_BASE_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-D__EFI_ALLOWED))
$(eval $(call EFI_LINK_TEMPLATE,\
	$(EFI_BIN_DIR),\
	$(X86_64_OBJS),\
	$(AARCH64_OBJS),\
	$(RISCV64_OBJS),vnexos_grub_main,\
	$(GRUB_LINKER_SCRIPT),1))

.PHONY: all clean
all: \
	$(EFI_BINS)
	@echo "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Bộ nạp mồi$(MODE_RESET)."

clean:
	@rm -rf $(GRUB_BUILD_DIR)
	@echo "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
