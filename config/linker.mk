# =========================================================
# MẪU BIÊN DỊCH CÁC TỆP MÃ LỆNH
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

define EFI_LINK_TEMPLATE

ifeq ($(7),1)
TAILX64 := X64.EFI
TAILAA64 := AA64.EFI
TAILRISCV64 := RISCV64.EFI
else
TAILX64 := x64.efi
TAILAA64 := aa64.efi
TAILRISCV64 := riscv64.efi
endif
RISCV_ELF2EFI := python3 $$(ROOT_DIR)/tools/elf2efi_riscv64.py

$(1)/%$$(TAILX64): $(2)
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_LD) Đang liên kết thành tệp EFI khởi động: $$@"
	@clang++ --target=x86_64-unknown-windows -nostdlib -fuse-ld=lld-link -Wl,-entry:$(5) -Wl,-subsystem:efi_application -Wl,-nodefaultlib -Wl,-dll -Wl,-dynamicbase -Wl,-noimplib -o $$@ $$^
	@echo "$$(MSG_LD) Đã liên kết thành công thành bộ nạp khởi động!"
	@boreas -sign -s $$(KEY_FILE) $$(PUB_FILE) $$@ $$@
	@sbsign --key $$(CERT_DIR)/vnexos_db.key --cert $$(CERT_DIR)/vnexos_db.crt --output $$@ $$@

$(1)/%$$(TAILAA64): $(3)
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_LD) Đang liên kết thành tệp EFI khởi động: $$@"
	@clang++ --target=aarch64-unknown-windows -nostdlib -fuse-ld=lld-link -Wl,-entry:$(5) -Wl,-subsystem:efi_application -Wl,-nodefaultlib -Wl,-dll -Wl,-dynamicbase -Wl,-noimplib -o $$@ $$^
	@echo "$$(MSG_LD) Đã liên kết thành công thành bộ nạp khởi động!"
	@boreas -sign -s $$(KEY_FILE) $$(PUB_FILE) $$@ $$@
	@sbsign --key $$(CERT_DIR)/vnexos_db.key --cert $$(CERT_DIR)/vnexos_db.crt --output $$@ $$@

$(1)/%$$(TAILRISCV64): $(4)
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_LD) Đang liên kết ELF trung gian: $$(BUILD_DIR)/tmp.elf"
	@ld.lld -nostdlib -m elf64lriscv -T$(6) -o $$(BUILD_DIR)/tmp.elf $$^
	@echo "$$(MSG_LD) Đang chuyển đổi ELF -> PE/UEFI: $$@"
	@$$(RISCV_ELF2EFI) $$(BUILD_DIR)/tmp.elf $$@
	@rm -f $$(BUILD_DIR)/tmp.elf
	@echo "$$(MSG_LD) Đã liên kết thành công thành bộ nạp khởi động!"
	@boreas -sign -s $$(KEY_FILE) $$(PUB_FILE) $$@ $$@
	@sbsign --key $$(CERT_DIR)/vnexos_db.key --cert $$(CERT_DIR)/vnexos_db.crt --output $$@ $$@

endef
