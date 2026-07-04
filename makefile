# =========================================================
# VNExos Bản Nguyên - HỆ ĐIỀU HÀNH VIỆT NAM
# VNExos Apeiron - VIETNAMESE OPERATING SYSTEM
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================
export
MAKEFLAGS += --no-print-directory

include config/color.mk
include config/general.mk

# Định nghĩa khung đóng/mở ngoặc vuông màu trắng, sau ngoặc vuông tự động RESET màu
BOX_OPEN  := $(COLOR_WHITE)[
BOX_CLOSE := $(COLOR_WHITE)]$(MODE_RESET)

# ==[ ĐỊNH NGHĨA TEXT BÊN TRONG CÁC MẪU THÔNG TIN ]=====================
# VNExos: VN (Màu Đỏ tươi) + Exos (Màu Lime) -> [ VNExos ]
# Sử dụng thêm MODE_BOLD để tên Hệ điều hành nổi bật hẳn lên
TXT_VNEXOS := $(MODE_BOLD)$(COLOR_RED)VN$(COLOR_LIME)Exos

# LD (Liên kết): Sử dụng màu AQUA (Xanh ngọc sáng) chuẩn công nghệ đồ họa
TXT_LD     := $(COLOR_AQUA)LD

# IMG (Tệp đĩa): Sử dụng màu YELLOW (Vàng chuẩn) cho tác vụ đóng gói đĩa
TXT_IMG    := $(COLOR_YELLOW)IMG

# QEMU (Giả lập): Sử dụng màu FUCHSIA (Hồng cánh sen/Tím sáng) tạo điểm nhấn khi chạy giả lập
TXT_QEMU   := $(COLOR_FUCHSIA)QEMU

# CLEAN (Dọn dẹp): Sử dụng màu MAROON (Đỏ sẫm) báo hiệu tác vụ xóa bỏ dữ liệu cũ
TXT_CLEAN  := $(COLOR_MAROON)CLEAN

# CXX (Biên dịch): Sử dụng màu BLUE (Xanh lam sáng) - Giữ nguyên tiền tố CXX chuyên nghiệp
TXT_CXX    := $(COLOR_BLUE)CXX

# ASM (Biên dịch hợp ngữ): Sử dụng màu RED (Đỏ tươi) để phân biệt với biên dịch C++
TXT_ASM    := $(COLOR_RED)ASM

# ==[ ĐÓNG GÓI THÀNH CÁC BIẾN MẪU HOÀN CHỈNH ]==========================
MSG_VNEXOS := $(BOX_OPEN) $(TXT_VNEXOS) $(BOX_CLOSE)
MSG_LD     := $(BOX_OPEN) $(TXT_LD)     $(BOX_CLOSE)
MSG_IMG    := $(BOX_OPEN) $(TXT_IMG)    $(BOX_CLOSE)
MSG_QEMU   := $(BOX_OPEN) $(TXT_QEMU)   $(BOX_CLOSE)
MSG_CLEAN  := $(BOX_OPEN) $(TXT_CLEAN)  $(BOX_CLOSE)
MSG_CXX    := $(BOX_OPEN) $(TXT_CXX)    $(BOX_CLOSE)
MSG_ASM    := $(BOX_OPEN) $(TXT_ASM)    $(BOX_CLOSE)

# ==[ CHUẨN BỊ CÁC BIẾN CẦN THIẾT ]=====================================
SUBDIRS    := grub internal

# Chuỗi UUID độc quyền cho VNExos
VNEXOS_BOOT_UUID := 2ef6ee17-2a14-4db9-8129-42a6dc48f8af
VNEXOS_TYPE_UUID := c48d1722-723b-43d6-bead-f2b2623343dd
VNEXOS_PART_UUID := 9c7ba30b-e2ee-4bbe-9fe4-ab5f23fe2b9b

# ==[ KỊCH BẢN XÂY DỰNG HỆ THỐNG ]======================================
.PHONY: all clean clean-all run $(SUBDIRS) shim
all: $(DISK_IMG)
	@echo "$(MSG_VNEXOS) Đã xây dựng xong chương trình!"

$(SUBDIRS):
	@$(MAKE) -C $@

# Cấu hình môi trường cho Shim
EFI_BIN_LOWER := $(shell echo $(EFI_BIN_NAME) | tr 'A-Z' 'a-z')
GRUB_NAME     := $(EFI_BIN_DIR)/$(patsubst boot%,grub%,$(EFI_BIN_LOWER))
MM_NAME       := $(EFI_BIN_DIR)/$(patsubst boot%,mm%,$(EFI_BIN_LOWER))
shim:
ifeq ($(ALLOW_SHIM),1)
	@$(TOOLS_ADDSBAT) $(EFI_BIN)
	@sbsign --key $(PRIV_FILE) --cert $(PUB_FILE) --output $(EFI_BIN) $(EFI_BIN)
	@cp $(DER_FILE) $(SYSROOT_DIR)/
	@mv $(EFI_BIN) $(GRUB_NAME)
	@cp $(SHIM_DIR)/* $(EFI_BIN_DIR)/
	@echo "$(MSG_VNEXOS) Đã sao chép xong file shim!"
endif

$(DISK_IMG): $(SUBDIRS) shim
# Tạo tệp ảnh đĩa trống kích thước 1 GiB (1024 MiB)
	@dd if=/dev/zero of=$(DISK_IMG) bs=1M count=1024 status=none

# Khởi tạo bảng phân vùng GPT trống trên tệp đĩa
	@sgdisk -o $(DISK_IMG) > /dev/null

# Tạo phân vùng 1 (UEFI): Bắt đầu tại sector 2048, nặng 30MB, mã loại ef00 (EFI System Partition)
	@sgdisk -n 1:2048:+30MB -t 1:ef00 -c 1:"Bộ nạp khởi động VNExos" -u 1:$(VNEXOS_BOOT_UUID) $(DISK_IMG) > /dev/null

# Tạo phân vùng 2 (Apeiron): Lấy hết dung lượng còn lại, mã loại 8300, ép nạp UUID
# Phân vùng này để trống hoàn toàn (Raw), không format
	@sgdisk -n 2:0:0 -t 2:$(VNEXOS_TYPE_UUID) -c 2:"Nhân lõi Apeiron" -u 2:$(VNEXOS_PART_UUID) $(DISK_IMG) > /dev/null

# Định dạng FAT32 cho phân vùng 1 (Bộ nạp khởi động UEFI)
# 61440 = số sector của phân vùng 1 (30MB / 512B mỗi sector)
# Giới hạn rõ ràng để FAT32 không tràn sang phân vùng 2
	@mkfs.vfat -F 32 --offset=2048 -s 1 -n "VNEXOS_BOOT" $(DISK_IMG) 61440 > /dev/null

# Tạo cấu trúc thư mục và sao chép Bộ nạp khởi động vào phân vùng 1 thông qua mtools
	@mcopy -s -i $(DISK_IMG)@@1M $(SYSROOT_DIR)/* ::/
# 	@mcopy -i $(DISK_IMG)@@1M $(CERT_DIR)/Microsoft_KEK.cer ::/Microsoft_KEK.cer
# 	@mcopy -i $(DISK_IMG)@@1M $(CERT_DIR)/Microsoft_Production_PCA.cer ::/Microsoft_Production_PCA.cer

	@echo "$(MSG_IMG) Đã tạo disk image $(DISK_IMG)"

# Dấu hiệu đã sao chép firmware - Make kiểm tra file này thay vì thư mục firmware/
# Chỉ chạy một lần duy nhất; xóa firmware/.ready để buộc sao chép lại
firmware/.ready:
	@mkdir -p $(dir $(EDK2_DIR))
	@cp -r /usr/share/edk2/* $(dir $(EDK2_DIR)) || true
	@touch $@

run: all firmware/.ready
	@echo "$(MSG_QEMU) Đang khởi chạy hệ thống..."
	@if [ ! -S "$(TPM_SOCK)" ]; then \
		mkdir -p "$(TPM_DIR)" && \
		swtpm socket --tpmstate dir="$(TPM_DIR)" --tpm2 --ctrl type=unixio,path="$(TPM_SOCK)" --daemon; \
	fi
	@$(QEMU) $(QEMU_FLAGS) \
		-drive file=$(DISK_IMG),format=raw,media=disk

clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	@rm -f $(DISK_IMG)
	@rm -rf $(BUILD_DIR) $(SYSROOT_DIR)
	@echo "$(MSG_CLEAN) Dự án đã được làm sạch sâu!"

clean-all:
	@rm -rf $(ROOT_DIR)/build $(SYSROOT_DIR)
	@rm -f $(DISK_IMG)
	@echo "$(MSG_CLEAN) Toàn bộ dự án đã được làm sạch hoàn toàn!"
