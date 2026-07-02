#!/bin/bash

# Định nghĩa các biến môi trường (Giống cấu trúc cũ của cậu)
TPM_DIR="/tmp/tpm_state"
TPM_SOCK="${TPM_DIR}/sock"

echo "===================================================="
echo "[+] Khởi động VNExos SWTPM Console..."
echo "===================================================="

# 1. Tự động dọn dẹp (Reset) tiến trình và dữ liệu cũ nếu có
if [ -d "$TPM_DIR" ]; then
    echo "[!] Phát hiện thư mục lưu trữ cũ. Đang tiến hành RESET sạch sẽ..."
    killall swtpm 2>/dev/null
    rm -rf "$TPM_DIR"
fi

# 2. Tạo mới thư mục chứa trạng thái NVRAM ảo
mkdir -p "$TPM_DIR"

echo "[+] Thư mục trạng thái: $TPM_DIR"
echo "[+] File Socket kết nối: $TPM_SOCK"
echo "[+] TRẠNG THÁI: Đang chạy hiển thị trực tiếp (Bấm Ctrl+C để tắt)"
echo "----------------------------------------------------"

# 3. Chạy swtpm trực tiếp trên Console (Đã bỏ --daemon, thêm --log)
swtpm socket --tpmstate dir="$TPM_DIR" \
             --tpm2 \
             --ctrl type=unixio,path="$TPM_SOCK" \
             --log level=20