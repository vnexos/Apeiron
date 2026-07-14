#!/bin/bash

ARCH=$1

if [ -z "$ARCH" ]; then
    ARCH=x86_64
fi

QEMU="qemu-system-$ARCH"
FIRMWARE="./firmware"
if [[ "$ARCH" == "x86_64" ]]; then
    EDK2_TYPE="x64"
    EDK2_DIR="$FIRMWARE/$EDK2_TYPE"
    FD_CODE="OVMF_CODE_4M.secboot.fd"
    FD_VARS="OVMF_VARS_4M.fd"

    QEMU_ARCH_FLAGS="\
        -M q35 \
        -cpu host \
        -enable-kvm \
        -vga virtio"
elif [[ "$ARCH" == "aarch64" ]]; then
    EDK2_TYPE="aarch64"
    EDK2_DIR="$FIRMWARE/$EDK2_TYPE"
    FD_CODE="AAVMF_CODE.secboot.fd"
    FD_VARS="AAVMF_VARS.fd"

    QEMU_ARCH_FLAGS="\
        -M virt \
        -cpu cortex-a57 \
        -device virtio-gpu-pci"
elif [[ "$ARCH" == "riscv64" ]]; then
    EDK2_TYPE="riscv64"
    EDK2_DIR="$FIRMWARE/$EDK2_TYPE"
    FD_CODE="RISCV_VIRT_CODE.fd"
    FD_VARS="RISCV_VIRT_VARS.fd"

    QEMU_ARCH_FLAGS="\
        -M virt \
        -cpu rv64 \
        -device virtio-gpu-pci"
else
    echo "Không hỗ trợ dòng Vi xử lý: $ARCH"
    exit 1
fi

$QEMU $QEMU_ARCH_FLAGS \
    -drive if=pflash,format=raw,readonly=on,file=$EDK2_DIR/$FD_CODE \
    -drive if=pflash,format=raw,file=$EDK2_DIR/$FD_VARS \
    -m 2G \
    -net none \
    -serial stdio \
    -display sdl \
    -device qemu-xhci \
    -device usb-kbd \
    -monitor telnet:127.0.0.1:5555,server,nowait \
    -drive file=disk.img,format=raw,media=disk
