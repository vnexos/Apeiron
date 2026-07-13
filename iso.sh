#!/usr/bin/env bash
# =========================================================
# VNExos Bản Nguyên - ĐÓNG GÓI ISO KHỞI ĐỘNG UEFI
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SYSROOT="${SCRIPT_DIR}/sysroot"
OUTPUT="${SCRIPT_DIR}/apeiron.iso"
EFI_IMG="efiboot.img"

# Kiểm tra các công cụ cần thiết
for cmd in dd mkfs.vfat mmd mcopy xorriso; do
    if ! command -v "$cmd" &>/dev/null; then
        echo "Lỗi: Không tìm thấy công cụ '$cmd'. Hãy cài đặt trước khi chạy." >&2
        exit 1
    fi
done

# Kiểm tra sysroot tồn tại
if [ ! -d "$SYSROOT/EFI/BOOT" ]; then
    echo "Lỗi: Không tìm thấy $SYSROOT/EFI/BOOT. Hãy build trước (make all)." >&2
    exit 1
fi

echo "=== Bắt đầu đóng gói ISO khởi động UEFI ==="

# --- Bước 1: Tạo cấu trúc ISO staging ---
ISO_STAGING="${SCRIPT_DIR}/iso_staging"
rm -rf "$ISO_STAGING"
mkdir -p "$ISO_STAGING"

# Sao chép toàn bộ sysroot vào thư mục staging
cp -r "$SYSROOT"/* "$ISO_STAGING"/

# --- Bước 2: Tạo file ảnh phân vùng EFI (FAT32) ---
# Tính toán kích thước cần thiết: tổng dung lượng sysroot + 1MB dự phòng
SYSROOT_SIZE_KB=$(du -sk "$SYSROOT" | cut -f1)
EFI_SIZE_MB=$(( (SYSROOT_SIZE_KB / 1024) + 4 ))
# Tối thiểu 4MB để FAT32 hoạt động ổn định
if [ "$EFI_SIZE_MB" -lt 4 ]; then
    EFI_SIZE_MB=4
fi

echo "  [IMG] Tạo ảnh EFI ${EFI_SIZE_MB}MB..."
dd if=/dev/zero of="$EFI_IMG" bs=1M count="$EFI_SIZE_MB" status=none
mkfs.vfat -F 12 "$EFI_IMG" > /dev/null

# Tạo cấu trúc thư mục EFI/BOOT bên trong file ảnh
mmd -i "$EFI_IMG" ::/EFI ::/EFI/BOOT

# Copy các file EFI vào ảnh
mcopy -i "$EFI_IMG" "$SYSROOT"/EFI/BOOT/* ::/EFI/BOOT/

# Copy kernel nếu tồn tại
if [ -f "$SYSROOT/Apeiron.kern" ]; then
    mcopy -i "$EFI_IMG" "$SYSROOT/Apeiron.kern" ::/
fi

# Copy assets nếu tồn tại
if [ -d "$SYSROOT/assets" ]; then
    mmd -i "$EFI_IMG" ::/assets || true
    # Tạo cấu trúc thư mục con trong assets
    find "$SYSROOT/assets" -type d | while read -r dir; do
        rel="${dir#$SYSROOT/}"
        if [ "$rel" != "assets" ]; then
            mmd -i "$EFI_IMG" "::/${rel}" 2>/dev/null || true
        fi
    done
    # Copy toàn bộ file
    find "$SYSROOT/assets" -type f | while read -r file; do
        rel="${file#$SYSROOT/}"
        dest_dir="$(dirname "$rel")"
        mcopy -i "$EFI_IMG" "$file" "::/${rel}" 2>/dev/null || true
    done
fi

# Đặt file EFI image vào staging
mv "$EFI_IMG" "$ISO_STAGING/"

# --- Bước 3: Đóng gói ISO với xorriso ---
echo "  [ISO] Đang tạo ISO hybrid UEFI..."
xorriso -as mkisofs \
    -R -J -joliet-long \
    -V "VNEXOS_BOOT" \
    -append_partition 2 0xef "$ISO_STAGING/$EFI_IMG" \
    -appended_part_as_gpt \
    -eltorito-alt-boot \
    -e "$EFI_IMG" \
    -no-emul-boot \
    -o "$OUTPUT" \
    "$ISO_STAGING"

# --- Bước 4: Dọn dẹp ---
rm -rf "$ISO_STAGING"

echo "=== Hoàn tất! ==="
echo "  Tệp ISO: $OUTPUT"
echo "  Kích thước: $(du -h "$OUTPUT" | cut -f1)"
echo ""