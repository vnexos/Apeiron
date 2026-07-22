#!/usr/bin/env python3
# =========================================================
# CÔNG CỤ CHUYỂN ĐỔI ELF -> PE/EFI CHO RISC-V 64-BIT
#
# Copyright (c) 2026 VNExos
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# LÝ DO CẦN TOOL NÀY:
#   - LLVM/Clang chưa hỗ trợ xuất COFF/PE cho riscv64
#   - GNU objcopy (pei-riscv64-little) tạo PE bị lỗi tiêu đề
#   - UEFI EDK2 RISC-V loader yêu cầu PE32+ hợp lệ có phân vùng .reloc
# =========================================================
"""
Chuyển đổi tệp thực thi ELF RISC-V 64-bit sang Ứng dụng PE32+ EFI.

Sử dụng:
    python3 elf2efi_riscv64.py <tệp_vào.elf> <tệp_ra.efi>
"""
import struct
import sys
import os


def align_up(value: int, alignment: int) -> int:
    """Căn chỉnh giá trị lên bội số của căn lề (alignment)."""
    return (value + alignment - 1) & ~(alignment - 1)


def create_riscv_efi(elf_path: str, output_path: str) -> None:
    """
    Chuyển đổi tệp thực thi ELF RISC-V 64-bit sang Ứng dụng PE32+ EFI.

    Lưu ý: ELF phải được xây dựng với kịch bản liên kết (linker script) đặt mã máy từ địa chỉ > 0
    (ví dụ: 0x1000) để tránh đè lên các tiêu đề PE.
    """
    with open(elf_path, 'rb') as f:
        elf = bytearray(f.read())

    # Kiểm tra dấu hiệu nhận biết (magic) của ELF
    if elf[:4] != b'\x7fELF':
        raise ValueError(f"Không phải tệp ELF: {elf_path}")

    # Phân tích tiêu đề ELF64
    e_entry = struct.unpack_from('<Q', elf, 24)[0]
    e_phoff = struct.unpack_from('<Q', elf, 32)[0]
    e_phnum = struct.unpack_from('<H', elf, 56)[0]

    # Thu thập các phân đoạn PT_LOAD
    load_segs = []
    for i in range(e_phnum):
        ph = e_phoff + i * 56  # Tiêu đề phân đoạn ELF64 phdr = 56 bytes
        p_type   = struct.unpack_from('<I', elf, ph)[0]
        if p_type == 1:  # PT_LOAD (Phân đoạn nạp)
            p_flags  = struct.unpack_from('<I', elf, ph + 4)[0]
            p_offset = struct.unpack_from('<Q', elf, ph + 8)[0]
            p_vaddr  = struct.unpack_from('<Q', elf, ph + 16)[0]
            p_filesz = struct.unpack_from('<Q', elf, ph + 32)[0]
            p_memsz  = struct.unpack_from('<Q', elf, ph + 40)[0]
            load_segs.append((p_flags, p_offset, p_vaddr, p_filesz, p_memsz))

    if not load_segs:
        raise ValueError("Không tìm thấy phân đoạn PT_LOAD trong tệp ELF")

    # === Hằng số PE ===
    FILE_ALIGN       = 0x200    # Căn chỉnh dữ liệu tệp: 512 bytes
    SECT_ALIGN       = 0x1000   # Căn chỉnh phân vùng trong bộ nhớ: 4KB
    IMAGE_BASE       = 0x0      # UEFI tự chọn địa chỉ nạp (An toàn với ASLR nhờ phân vùng .reloc)
    MACHINE_RISCV64  = 0x5064   # Mã máy kiến trúc IMAGE_FILE_MACHINE_RISCV64
    PE_MAGIC_PE32P   = 0x20b    # Định dạng PE32+ (64-bit)
    SUBSYS_EFI_APP   = 10       # Phân hệ: Ứng dụng EFI (EFI Application)

    # Thuộc tính đặc trưng của phân vùng (Section characteristics)
    CHARS_CODE  = 0x60000020    # Chứa mã máy | Cho phép thực thi | Cho phép đọc
    CHARS_RDATA = 0x40000040    # Chứa dữ liệu đã khởi tạo | Chỉ cho phép đọc
    CHARS_DATA  = 0xC0000040    # Chứa dữ liệu đã khởi tạo | Cho phép đọc | Cho phép ghi
    CHARS_RELOC = 0x42000040    # Chứa dữ liệu đã khởi tạo | Cho phép đọc | Có thể giải phóng khỏi bộ nhớ

    # Tạo các phân vùng PE từ các phân đoạn ELF
    sections = []
    for (flags, offset, vaddr, filesz, memsz) in load_segs:
        raw = bytes(elf[offset:offset + filesz])
        # Mở rộng raw lên memsz (zero-fill cho .bss) rồi mới căn chỉnh FILE_ALIGN
        raw_full = raw + b'\x00' * (memsz - filesz)
        padded = raw_full + b'\x00' * (align_up(len(raw_full), FILE_ALIGN) - len(raw_full))

        if flags & 0x1:   # Cờ thực thi (Execute flag)
            name = b'.text\x00\x00\x00'
            chars = CHARS_CODE
        elif flags & 0x2:  # Cờ ghi (Write flag)
            name = b'.data\x00\x00\x00'
            chars = CHARS_DATA
        else:              # Chỉ cho phép đọc (Read only)
            name = b'.rdata\x00\x00'
            chars = CHARS_RDATA

        sections.append({
            'name':  name,
            'vaddr': vaddr,
            'vsize': memsz,
            'data':  padded,
            'chars': chars,
        })

    # Thêm phân vùng .reloc rỗng — BẮT BUỘC để trình nạp UEFI không từ chối tệp ảnh nhị phân
    # Khối định vị lại cơ sở PE (PE Base Relocation Block): 8 bytes = 1 khối trống (không chứa thực thể định vị lại)
    # UEFI không cần định vị lại mã máy vì RISC-V sử dụng cơ chế định địa chỉ tương đối theo con trỏ lệnh PC (auipc)
    reloc_raw = struct.pack('<II', 0x0, 0x8)  # Địa chỉ ảo VirtualAddress=0, Kích thước khối SizeOfBlock=8 (khối trống)
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
    OPT_HDR_SIZE   = 240  # Kích thước cấu trúc tiêu đề tùy chọn của EFI = 112 bytes cơ bản + 128 bytes danh mục dữ liệu

    # Tính kích thước các tiêu đề và vị trí lệch (offset) của tệp
    raw_hdr_size   = 64 + 4 + 20 + OPT_HDR_SIZE + num_sections * 40
    size_of_hdrs   = align_up(raw_hdr_size, FILE_ALIGN)
    current_offset = size_of_hdrs
    for sec in sections:
        sec['file_offset'] = current_offset
        current_offset += len(sec['data'])

    last_sec       = sections[-1]
    size_of_image  = align_up(last_sec['vaddr'] + last_sec['vsize'], SECT_ALIGN)
    reloc_sec      = sections[-1]  # Phân vùng định vị lại .reloc

    # Tính kích thước vùng mã máy và kích thước vùng dữ liệu
    code_size = sum(align_up(len(s['data']), FILE_ALIGN) for s in sections if s['chars'] & 0x20)
    data_size = sum(align_up(len(s['data']), FILE_ALIGN) for s in sections if not (s['chars'] & 0x20))
    base_of_code = next(s['vaddr'] for s in sections if s['chars'] & 0x20)

    # === Xây dựng tệp PE ===
    pe = bytearray()

    # Khung đệm DOS stub (64 bytes) — tối giản, chỉ có chữ ký ký tự 'MZ' và trường vị trí tiêu đề e_lfanew
    dos = bytearray(64)
    dos[0:2] = b'MZ'
    struct.pack_into('<I', dos, 60, 64)   # e_lfanew: Chữ ký PE nằm tại vị trí offset số 64
    pe += dos

    # Chữ ký hiệu PE
    pe += b'PE\x00\x00'

    # Tiêu đề tệp COFF (20 bytes)
    pe += struct.pack('<HHIIIHH',
        MACHINE_RISCV64,    # Kiến trúc máy (Machine)
        num_sections,       # Số lượng phân vùng (NumberOfSections)
        0,                  # Nhãn thời gian (TimeDateStamp) (0 = giúp tạo tệp giống hệt nhau ở các lần biên dịch sau)
        0,                  # Con trỏ tới bảng ký hiệu (PointerToSymbolTable) (không dùng)
        0,                  # Số lượng ký hiệu (NumberOfSymbols) (không dùng)
        OPT_HDR_SIZE,       # Kích thước tiêu đề tùy chọn (SizeOfOptionalHeader)
        0x0022,             # Đặc điểm đặc trưng: Tệp ảnh có thể thực thi | Hỗ trợ không gian địa chỉ lớn hơn 2GB
    )

    # Tiêu đề tùy chọn PE32+ (Optional Header - 240 bytes)
    opt = struct.pack('<HBB', PE_MAGIC_PE32P, 2, 0)  # Mã nhận biết Magic, Phiên bản lớn trình liên kết, Phiên bản nhỏ trình liên kết
    opt += struct.pack('<III', code_size, data_size, 0)  # Kích thước các vùng dữ liệu
    opt += struct.pack('<I', e_entry)         # Địa chỉ ảo của điểm nhập (AddressOfEntryPoint - RVA)
    opt += struct.pack('<I', base_of_code)    # Địa chỉ ảo cơ sở của mã máy (BaseOfCode - RVA)
    opt += struct.pack('<Q', IMAGE_BASE)      # Địa chỉ cơ sở nạp tệp ảnh (ImageBase)
    opt += struct.pack('<II', SECT_ALIGN, FILE_ALIGN)  # Các thông số căn chỉnh (Alignments)
    opt += struct.pack('<HHHHHH', 0, 0, 0, 0, 0, 0)   # Phiên bản hệ điều hành/Tệp ảnh/Phân hệ
    opt += struct.pack('<I', 0)               # Giá trị phiên bản Win32 (Không dùng)
    opt += struct.pack('<I', size_of_image)   # Kích thước tổng thể tệp ảnh trong bộ nhớ (SizeOfImage)
    opt += struct.pack('<I', size_of_hdrs)    # Kích thước tổng tất cả các tiêu đề trong tệp (SizeOfHeaders)
    opt += struct.pack('<I', 0)               # Giá trị mã kiểm tra lỗi CheckSum (0 = bỏ qua kiểm tra)
    opt += struct.pack('<H', SUBSYS_EFI_APP)  # Phân hệ: Ứng dụng lớp EFI (EFI_APPLICATION)
    opt += struct.pack('<H', 0)               # Các đặc tính thư viện liên kết động DllCharacteristics
    opt += struct.pack('<Q', 0)               # Kích thước bộ nhớ ngăn xếp dự trữ (SizeOfStackReserve)
    opt += struct.pack('<Q', 0)               # Kích thước bộ nhớ ngăn xếp thực nạp (SizeOfStackCommit)
    opt += struct.pack('<Q', 0)               # Kích thước bộ nhớ vùng nhớ Heap dự trữ (SizeOfHeapReserve)
    opt += struct.pack('<Q', 0)               # Kích thước bộ nhớ vùng nhớ Heap thực nạp (SizeOfHeapCommit)
    opt += struct.pack('<I', 0)               # Các cờ của trình nạp (LoaderFlags)
    opt += struct.pack('<I', 16)              # Số lượng thư mục địa chỉ ảo và kích thước (NumberOfRvaAndSizes)

    # Các thư mục danh mục dữ liệu (Data Directories - 16 thực thể × 8 bytes = 128 bytes)
    for i in range(16):
        if i == 5:  # Thư mục định vị lại cơ sở (Base Relocation Directory)
            opt += struct.pack('<II', reloc_sec['vaddr'], 8)
        else:
            opt += struct.pack('<II', 0, 0)

    assert len(opt) == OPT_HDR_SIZE, f"Kích thước tiêu đề tùy chọn bị lỗi: {len(opt)} != {OPT_HDR_SIZE}"
    pe += opt

    # Bảng phân vùng (Section Table - 40 bytes cho mỗi phân vùng)
    for sec in sections:
        pe += sec['name'][:8]
        pe += struct.pack('<II', sec['vsize'], sec['vaddr'])
        pe += struct.pack('<II', len(sec['data']), sec['file_offset'])
        pe += struct.pack('<II', 0, 0)        # Con trỏ tới danh sách định vị lại, Con trỏ tới danh sách số dòng
        pe += struct.pack('<HHI', 0, 0, sec['chars'])  # Các biến đếm + Thuộc tính đặc trưng phân vùng

    # Ghi đệm các tiêu đề bằng byte 0 cho đến khi đạt kích thước FileAlignment
    pe += b'\x00' * (size_of_hdrs - len(pe))

    # Đẩy dữ liệu của từng phân vùng thực tế vào tệp ảnh
    for sec in sections:
        assert len(pe) == sec['file_offset'], (
            f"Vị trí lệch không khớp tại phân vùng '{sec['name']}': "
            f"0x{len(pe):x} != 0x{sec['file_offset']:x}"
        )
        pe += sec['data']

    with open(output_path, 'wb') as f:
        f.write(pe)

    print(f"  ✓ {output_path}")
    print(f"    Số phân vùng: {num_sections}, Kích thước tệp ảnh bộ nhớ=0x{size_of_image:x}, Điểm nhập=0x{e_entry:x}")


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f"Sử dụng: {sys.argv[0]} <tệp_vào.elf> <tệp_ra.efi>")
        sys.exit(1)
    create_riscv_efi(sys.argv[1], sys.argv[2])