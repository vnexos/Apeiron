#!/bin/bash

SYSROOT="$2"
KERN_FILE="$SYSROOT"/apeiron.kern
CERT_PATH=$3
ROOT_KEY_PATH="$4"

shopt -s nullglob

TARGET_DIR="$1"
grub_files=("$TARGET_DIR"/BOOT*.EFI)
boot_files=("$TARGET_DIR"/vnexos*.efi)

shopt -u nullglob

boreas -encrypt -g "$SYSROOT"/key.sec "$SYSROOT"/key.pub

boreas -encrypt -e "$SYSROOT"/key.pub "$KERN_FILE" "$KERN_FILE"

boreas -sign -s "$ROOT_KEY_PATH" "$CERT_PATH" "$KERN_FILE" "$KERN_FILE" -t usx

grub_hash=""
boot_hash=""

# Duyệt từng tệp trong danh sách
for file in "${grub_files[@]}"; do
    grub_hash="$grub_hash$(boreas -shav 256 "$file")"
done

for file in "${boot_files[@]}"; do
    boot_hash="$boot_hash$(boreas -shav 256 "$file")"
done

ROOT_CERT_HASH=$(boreas -shav 256 "$CERT_PATH")
APP_HASH=$(boreas -shav 256 "$(boreas -shav 256 "$grub_hash")$(boreas -shav 256 "$boot_hash")$(boreas -shav 256 "$KERN_FILE")")

(
    FINAL_KEY=$(boreas -shav 256 "$ROOT_CERT_HASH$APP_HASH")
    boreas -aes256 "$SYSROOT"/key.sec "$SYSROOT"/key.sec "$FINAL_KEY"
)

sync
