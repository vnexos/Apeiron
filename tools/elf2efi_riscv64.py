#!/usr/bin/env python3
# =========================================================
# CÔNG CỤ CHUYỂN ĐỔI ELF -> PE/EFI CHO RISC-V 64-BIT
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# LÝ DO CẦN TOOL NÀY:
#   - LLVM/Clang chưa hỗ trợ xuất COFF/PE cho riscv64
#   - GNU objcopy (pei-riscv64-little) tạo PE bị lỗi header
#   - UEFI EDK2 RISC-V loader yêu cầu PE32+ hợp lệ có .reloc section
# =========================================================
"""
Chuyển đổi ELF executable RISC-V 64-bit sang PE32+ EFI Application.

Sử dụng:
    python3 elf2efi_riscv64.py <input.elf> <output.efi>
"""
import struct
import sys
import os


def align_up(value: int, alignment: int) -> int:
    """Căn chỉnh giá trị lên bội số của alignment."""
    return (value + alignment - 1) & ~(alignment - 1)


def create_riscv_efi(elf_path: str, output_path: str) -> None:
    """
    Chuyển đổi ELF executable RISC-V 64-bit sang PE32+ EFI Application.

    Lưu ý: ELF phải được build với linker script đặt code từ địa chỉ > 0
    (ví dụ: 0x1000) để tránh overlap với PE headers.
    """
    with open(elf_path, 'rb') as f:
        elf = bytearray(f.read())

    # Kiểm tra ELF magic
    if elf[:4] != b'\x7fELF':
        raise ValueError(f"Không phải ELF file: {elf_path}")

    # Parse ELF64 header
    e_entry = struct.unpack_from('<Q', elf, 24)[0]
    e_phoff = struct.unpack_from('<Q', elf, 32)[0]
    e_phnum = struct.unpack_from('<H', elf, 56)[0]

    # Thu thập PT_LOAD segments
    load_segs = []
    for i in range(e_phnum):
        ph = e_phoff + i * 56  # ELF64 phdr = 56 bytes
        p_type   = struct.unpack_from('<I', elf, ph)[0]
        if p_type == 1:  # PT_LOAD
            p_flags  = struct.unpack_from('<I', elf, ph + 4)[0]
            p_offset = struct.unpack_from('<Q', elf, ph + 8)[0]
            p_vaddr  = struct.unpack_from('<Q', elf, ph + 16)[0]
            p_filesz = struct.unpack_from('<Q', elf, ph + 32)[0]
            p_memsz  = struct.unpack_from('<Q', elf, ph + 40)[0]
            load_segs.append((p_flags, p_offset, p_vaddr, p_filesz, p_memsz))

    if not load_segs:
        raise ValueError("Không tìm thấy PT_LOAD segment trong ELF")

    # === Hằng số PE ===
    FILE_ALIGN       = 0x200    # Căn chỉnh dữ liệu file: 512 bytes
    SECT_ALIGN       = 0x1000   # Căn chỉnh section trong bộ nhớ: 4KB
    IMAGE_BASE       = 0x0      # UEFI chọn địa chỉ load (ASLR-safe với .reloc)
    MACHINE_RISCV64  = 0x5064   # IMAGE_FILE_MACHINE_RISCV64
    PE_MAGIC_PE32P   = 0x20b    # PE32+ (64-bit)
    SUBSYS_EFI_APP   = 10       # EFI Application

    # Section characteristics
    CHARS_CODE  = 0x60000020    # CNT_CODE | MEM_EXECUTE | MEM_READ
    CHARS_RDATA = 0x40000040    # CNT_INITIALIZED_DATA | MEM_READ
    CHARS_DATA  = 0xC0000040    # CNT_INITIALIZED_DATA | MEM_READ | MEM_WRITE
    CHARS_RELOC = 0x42000040    # CNT_INITIALIZED_DATA | MEM_READ | MEM_DISCARDABLE

    # Tạo PE sections từ ELF segments
    sections = []
    for (flags, offset, vaddr, filesz, memsz) in load_segs:
        raw = bytes(elf[offset:offset + filesz])
        padded = raw + b'\x00' * (align_up(len(raw), FILE_ALIGN) - len(raw))

        if flags & 0x1:   # Execute flag
            name = b'.text\x00\x00\x00'
            chars = CHARS_CODE
        elif flags & 0x2:  # Write flag
            name = b'.data\x00\x00\x00'
            chars = CHARS_DATA
        else:              # Read only
            name = b'.rdata\x00\x00'
            chars = CHARS_RDATA

        sections.append({
            'name':  name,
            'vaddr': vaddr,
            'vsize': memsz,
            'data':  padded,
            'chars': chars,
        })

    # Thêm .reloc section rỗng — BẮT BUỘC để UEFI loader không reject image
    # PE Base Relocation Block: 8 bytes = 1 empty block (không có relocation entries)
    # UEFI không cần relocate code vì RISC-V dùng PC-relative addressing (auipc)
    reloc_raw = struct.pack('<II', 0x0, 0x8)  # VirtualAddress=0, SizeOfBlock=8 (empty)
    reloc_padded = reloc_raw + b'\x00' * (align_up(len(reloc_raw), FILE_ALIGN) - len(reloc_raw))
    reloc_vaddr = align_up(sections[-1]['vaddr'] + sections[-1]['vsize'], SECT_ALIGN)
    sections.append({
        'name':  b'.reloc\x00\x00',
        'vaddr': reloc_vaddr,
        'vsize': len(reloc_raw),
        'data':  reloc_padded,
        'chars': CHARS_RELOC,
    })

    num_sections   = len(sections)
    OPT_HDR_SIZE   = 240  # sizeof(EFI_IMAGE_OPTIONAL_HEADER64) = 112 base + 128 data dirs

    # Tính kích thước headers và file offsets
    raw_hdr_size   = 64 + 4 + 20 + OPT_HDR_SIZE + num_sections * 40
    size_of_hdrs   = align_up(raw_hdr_size, FILE_ALIGN)
    current_offset = size_of_hdrs
    for sec in sections:
        sec['file_offset'] = current_offset
        current_offset += len(sec['data'])

    last_sec       = sections[-1]
    size_of_image  = align_up(last_sec['vaddr'] + last_sec['vsize'], SECT_ALIGN)
    reloc_sec      = sections[-1]  # .reloc section

    # Tính code size và data size
    code_size = sum(align_up(len(s['data']), FILE_ALIGN) for s in sections if s['chars'] & 0x20)
    data_size = sum(align_up(len(s['data']), FILE_ALIGN) for s in sections if not (s['chars'] & 0x20))
    base_of_code = next(s['vaddr'] for s in sections if s['chars'] & 0x20)

    # === Xây dựng PE file ===
    pe = bytearray()

    # DOS stub (64 bytes) — tối giản, chỉ có MZ signature và e_lfanew
    dos = bytearray(64)
    dos[0:2] = b'MZ'
    struct.pack_into('<I', dos, 60, 64)   # e_lfanew: PE signature tại offset 64
    pe += dos

    # PE signature
    pe += b'PE\x00\x00'

    # COFF File Header (20 bytes)
    pe += struct.pack('<HHIIIHH',
        MACHINE_RISCV64,    # Machine
        num_sections,       # NumberOfSections
        0,                  # TimeDateStamp (0 = reproducible build)
        0,                  # PointerToSymbolTable (không dùng)
        0,                  # NumberOfSymbols (không dùng)
        OPT_HDR_SIZE,       # SizeOfOptionalHeader
        0x0022,             # Characteristics: IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE
    )

    # Optional Header PE32+ (240 bytes)
    opt = struct.pack('<HBB', PE_MAGIC_PE32P, 2, 0)  # Magic, MajorLinker, MinorLinker
    opt += struct.pack('<III', code_size, data_size, 0)  # Sizes
    opt += struct.pack('<I', e_entry)         # AddressOfEntryPoint (RVA)
    opt += struct.pack('<I', base_of_code)    # BaseOfCode (RVA)
    opt += struct.pack('<Q', IMAGE_BASE)      # ImageBase
    opt += struct.pack('<II', SECT_ALIGN, FILE_ALIGN)  # Alignments
    opt += struct.pack('<HHHHHH', 0, 0, 0, 0, 0, 0)   # OS/Image/Subsystem versions
    opt += struct.pack('<I', 0)               # Win32VersionValue
    opt += struct.pack('<I', size_of_image)   # SizeOfImage
    opt += struct.pack('<I', size_of_hdrs)    # SizeOfHeaders
    opt += struct.pack('<I', 0)               # CheckSum (0 = unchecked)
    opt += struct.pack('<H', SUBSYS_EFI_APP)  # Subsystem: EFI_APPLICATION
    opt += struct.pack('<H', 0)               # DllCharacteristics
    opt += struct.pack('<Q', 0)               # SizeOfStackReserve
    opt += struct.pack('<Q', 0)               # SizeOfStackCommit
    opt += struct.pack('<Q', 0)               # SizeOfHeapReserve
    opt += struct.pack('<Q', 0)               # SizeOfHeapCommit
    opt += struct.pack('<I', 0)               # LoaderFlags
    opt += struct.pack('<I', 16)              # NumberOfRvaAndSizes

    # Data Directories (16 entries × 8 bytes = 128 bytes)
    for i in range(16):
        if i == 5:  # Base Relocation Directory
            opt += struct.pack('<II', reloc_sec['vaddr'], 8)
        else:
            opt += struct.pack('<II', 0, 0)

    assert len(opt) == OPT_HDR_SIZE, f"Optional header size lỗi: {len(opt)} != {OPT_HDR_SIZE}"
    pe += opt

    # Section Table (40 bytes mỗi section)
    for sec in sections:
        pe += sec['name'][:8]
        pe += struct.pack('<II', sec['vsize'], sec['vaddr'])
        pe += struct.pack('<II', len(sec['data']), sec['file_offset'])
        pe += struct.pack('<II', 0, 0)        # PointerToRelocations, PointerToLineNumbers
        pe += struct.pack('<HHI', 0, 0, sec['chars'])  # Counts + Characteristics

    # Padding headers đến FileAlignment
    pe += b'\x00' * (size_of_hdrs - len(pe))

    # Dữ liệu từng section
    for sec in sections:
        assert len(pe) == sec['file_offset'], (
            f"File offset không khớp tại '{sec['name']}': "
            f"0x{len(pe):x} != 0x{sec['file_offset']:x}"
        )
        pe += sec['data']

    with open(output_path, 'wb') as f:
        f.write(pe)

    print(f"  ✓ {output_path}")
    print(f"    Sections: {num_sections}, SizeOfImage=0x{size_of_image:x}, Entry=0x{e_entry:x}")


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f"Sử dụng: {sys.argv[0]} <input.elf> <output.efi>")
        sys.exit(1)
    create_riscv_efi(sys.argv[1], sys.argv[2])
