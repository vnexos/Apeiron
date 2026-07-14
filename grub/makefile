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

GRUB_DIR       := $(ROOT_DIR)/grub
GRUB_BUILD_DIR := $(BUILD_DIR)/grub
# Tệp ELF trung gian (chỉ dùng cho riscv64 trước khi chuyển sang PE)
GRUB_ELF_SO    := $(GRUB_BUILD_DIR)/grub.elf
SRCS           := $(shell find $(GRUB_DIR)/src -type f \( -name "*.cpp" -o -name "*.s" \))
OBJS           := $(patsubst $(GRUB_DIR)/src/%.cpp,$(GRUB_BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS))) \
				  $(shell find $(SHARED_BUILD_DIR) -type f \( -name "*.o" \))
GRUB_LINKER_SCRIPT := $(GRUB_DIR)/linker_grub_riscv64.ld

$(eval $(call COMPILE_TEMPLATE,\
	$(GRUB_DIR)/src,\
	$(GRUB_BUILD_DIR),\
	$(BOOT_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-D__EFI_ALLOWED))

.PHONY: all clean
all: \
	$(EFI_BIN)
	@echo "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Bộ nạp mồi$(MODE_RESET)."

# Với riscv64, quy trình build EFI gồm 2 bước:
# 1. ld.lld + tập lệnh liên kết -> ELF thực thi (mã bắt đầu từ Địa chỉ ảo=0x1000)
# 2. tools/elf2efi_riscv64.py -> Ứng dụng EFI chuẩn PE32+ đúng đặc tả UEFI
#    (có section .reloc rỗng bắt buộc để UEFI loader không từ chối image)
$(EFI_BIN): $(OBJS)
	@mkdir -p $(EFI_BIN_DIR) $(GRUB_BUILD_DIR)
ifeq ($(ARCH),riscv64)
	@echo "$(MSG_LD) Đang liên kết ELF trung gian: $(GRUB_ELF_SO)"
	@$(BOOT_LD) $(BOOT_LDFLAGS) -T$(GRUB_LINKER_SCRIPT) -o $(GRUB_ELF_SO) $^
	@echo "$(MSG_LD) Đang chuyển đổi ELF -> PE/UEFI: $@"
	@$(RISCV_ELF2EFI) $(GRUB_ELF_SO) $@
else
	@echo "$(MSG_LD) Đang liên kết thành tệp EFI khởi động: $@"
	@$(BOOT_LD) $(BOOT_LDFLAGS) -Wl,-entry:vnexos_grub_main -o $@ $^
endif
	@echo "$(MSG_LD) Đã liên kết thành công thành bộ nạp khởi động!"
	@boreas -sign -s $(KEY_FILE) $(PUB_FILE) $@ $@
	@sbsign --key $(CERT_DIR)/vnexos_db.key --cert $(CERT_DIR)/vnexos_db.crt --output $@ $@

clean:
	@rm -rf $(GRUB_BUILD_DIR)
	@echo "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
