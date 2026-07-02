#!/usr/bin/env python3
# =========================================================
# CÔNG CỤ THÊM PHÂN VÙNG BẢO MẬT .sbat VÀO TỆP EFI
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# LÝ DO CẦN TOOL NÀY:
#   - Các trình biên dịch hiện đại hiện đã không còn hỗ trợ
#     xuất ra tệp EFI với một phân vùng `.sbat`.
#   - Đối với Secboot thì Shim sẽ kiểm tra phân vùng .sbat
#     trong tệp boot.
# =========================================================
"""
add_sbat.py - Thêm hoặc thay thế một phân vùng .sbat trong tệp nhị phân UEFI EFI.

Cách sử dụng:
    python3 add_sbat.py <tệp_efi> [--sbat <sbat_csv>] [--output <tệp_đầu_ra>]

Nếu không chỉ định --output, tệp EFI sẽ bị chỉnh sửa trực tiếp tại chỗ.
Nếu không chỉ định --sbat, một cấu hình dữ liệu SBAT mẫu có sẵn sẽ được dùng.
"""

import sys
import os
import struct
import argparse
import shutil
import pefile

# Cấu hình dữ liệu SBAT CSV mặc định
DEFAULT_SBAT = (
    "sbat,1,SBAT Version,sbat,1,"
    "https://github.com/rhboot/shim/blob/main/SBAT.md\n"
    "example.uefi,1,Example Corp,example,1,https://example.com\n"
)

# Các hằng số định dạng PE / COFF
IMAGE_SCN_CNT_INITIALIZED_DATA = 0x00000040
IMAGE_SCN_MEM_READ              = 0x40000000
SECTION_FLAGS = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ

SECTION_HEADER_SIZE = 40   # Số bytes cho mỗi mục tiêu đề phân vùng PE
SECTION_NAME        = b".sbat"


# Các hàm bổ trợ

def align_up(value: int, alignment: int) -> int:
    """Làm tròn giá trị *value* lên bội số chung gần nhất của *alignment*."""
    return (value + alignment - 1) & ~(alignment - 1)


def pad_section_name(name: bytes) -> bytes:
    """Trả về tên phân vùng chuẩn 8-byte, tự động chèn ký tự NUL vào bên phải."""
    return name[:8].ljust(8, b"\x00")


def find_sbat_section(pe: pefile.PE):
    """Tìm và trả về đối tượng phân vùng .sbat nếu đã có, ngược lại trả về None."""
    for section in pe.sections:
        if section.Name.rstrip(b"\x00") == SECTION_NAME:
            return section
    return None


# Ba chiến lược cập nhật dữ liệu riêng biệt

def _update_inplace(tmp_path: str, pe: pefile.PE, existing, sbat_data: bytes) -> None:
    """Ghi đè trực tiếp dữ liệu .sbat tại chỗ (dữ liệu mới vừa vặn với vùng nhớ cũ)."""
    old_raw_size = existing.SizeOfRawData
    raw_offset   = existing.PointerToRawData

    # Cập nhật trường kích thước ảo (VirtualSize) trong tiêu đề phân vùng
    existing.Misc_VirtualSize = len(sbat_data)
    pe.write(tmp_path)
    pe.close()

    # Ghi nội dung dữ liệu thực (tự động lấp đầy khoảng trống bằng ký tự trắng NUL)
    with open(tmp_path, "r+b") as f:
        f.seek(raw_offset)
        f.write(sbat_data.ljust(old_raw_size, b"\x00"))

    print(f"[+] Đã cập nhật dữ liệu .sbat trực tiếp tại chỗ ({len(sbat_data)} bytes).")


def _update_append(tmp_path: str, pe: pefile.PE, existing,
                   sbat_data: bytes, file_align: int, sect_align: int) -> None:
    """Di dời vị trí phân vùng .sbat xuống cuối tệp (dữ liệu mới lớn hơn vùng nhớ cũ)."""
    new_raw_size = align_up(len(sbat_data), file_align)
    pe.close()

    with open(tmp_path, "r+b") as f:
        f.seek(0, 2)
        new_raw_offset = align_up(f.tell(), file_align)
        f.write(b"\x00" * (new_raw_offset - f.tell()))  # Căn lề khoảng trống
        f.write(sbat_data.ljust(new_raw_size, b"\x00"))

    # Vá lại các thông số trong tiêu đề phân vùng
    pe2 = pefile.PE(tmp_path, fast_load=False)
    for s in pe2.sections:
        if s.Name.rstrip(b"\x00") == SECTION_NAME:
            s.PointerToRawData = new_raw_offset
            s.SizeOfRawData    = new_raw_size
            s.Misc_VirtualSize = len(sbat_data)
            break
    pe2.write(tmp_path)
    pe2.close()

    print(f"[+] Dữ liệu .sbat đã được di dời xuống cuối tệp "
          f"(vị trí offset=0x{new_raw_offset:08X}, kích thước={new_raw_size} bytes).")


def _add_new_section(tmp_path: str, pe: pefile.PE,
                     sbat_data: bytes, file_align: int, sect_align: int) -> None:
    """Chèn một phân vùng .sbat hoàn toàn mới (trước đó tệp chưa từng có phân vùng này)."""
    num_sections = pe.FILE_HEADER.NumberOfSections

    # Vị trí dữ liệu thô (Raw offset) = Ngay sau dữ liệu thô của phân vùng cuối cùng, được căn lề
    last_section   = max(pe.sections, key=lambda s: s.PointerToRawData)
    new_raw_offset = align_up(
        last_section.PointerToRawData + last_section.SizeOfRawData,
        file_align,
    )

    # Địa chỉ ảo (Virtual address) = Nằm sau vùng nhớ ảo của phân vùng cuối cùng, được căn lề
    last_va_end = max(
        s.VirtualAddress + align_up(
            s.Misc_VirtualSize or s.SizeOfRawData, sect_align
        )
        for s in pe.sections
    )
    new_va       = align_up(last_va_end, sect_align)
    new_raw_size = align_up(len(sbat_data), file_align)

    # Xác định vị trí của tiêu đề phân vùng mới sẽ được ghi trong bảng tiêu đề
    first_hdr_offset = (
        pe.DOS_HEADER.e_lfanew          # Vị trí trỏ tới chữ ký PE
        + 4                              # Vòng qua chuỗi chữ ký "PE\0\0"
        + pefile.Structure(
            pefile.PE.__IMAGE_FILE_HEADER_format__
          ).sizeof()
        + pe.FILE_HEADER.SizeOfOptionalHeader
    )
    new_hdr_offset = first_hdr_offset + num_sections * SECTION_HEADER_SIZE

    # Bảo vệ: Đảm bảo bảng tiêu đề phân vùng không đè lên dữ liệu thô của phân vùng đầu tiên
    earliest_raw = min(s.PointerToRawData for s in pe.sections)
    if new_hdr_offset + SECTION_HEADER_SIZE > earliest_raw:
        pe.close()
        raise RuntimeError(
            "Không còn đủ khoảng trống trong cấu trúc tiêu đề PE để chèn thêm phân vùng mới. "
            "Tệp nhị phân này không có vùng đệm trống giữa bảng tiêu đề và dữ liệu phân vùng thô đầu tiên."
        )

    # Đóng gói cấu trúc tiêu đề phân vùng chuẩn 40-byte
    new_header = struct.pack(
        "<8sIIIIIIHHI",
        pad_section_name(SECTION_NAME),  # Tên phân vùng     8s
        len(sbat_data),                  # VirtualSize       I
        new_va,                          # VirtualAddress    I
        new_raw_size,                    # SizeOfRawData     I
        new_raw_offset,                  # PointerToRawData  I
        0,                               # PointerToRelocs   I
        0,                               # PointerToLinenums I
        0,                               # NumberOfRelocs    H
        0,                               # NumberOfLinenums  H
        SECTION_FLAGS,                   # Cờ thuộc tính     I
    )
    assert len(new_header) == SECTION_HEADER_SIZE, "LỖI HỆ THỐNG: Kích thước tiêu đề không khớp"

    pe.close()

    with open(tmp_path, "r+b") as f:
        # 1. Ghi mục tiêu đề phân vùng mới vào bảng
        f.seek(new_hdr_offset)
        f.write(new_header)

        # 2. Ghi đệm tệp bằng byte trống cho đến vị trí offset thô mới, rồi ghi dữ liệu thực vào
        f.seek(0, 2)
        eof = f.tell()
        if eof < new_raw_offset:
            f.write(b"\x00" * (new_raw_offset - eof))
        f.seek(new_raw_offset)
        f.write(sbat_data.ljust(new_raw_size, b"\x00"))

    # Mở lại tệp để tăng chỉ số số lượng phân vùng và cập nhật tổng kích thước tệp ảnh bộ nhớ
    pe2 = pefile.PE(tmp_path, fast_load=False)
    pe2.FILE_HEADER.NumberOfSections  += 1
    pe2.OPTIONAL_HEADER.SizeOfImage    = align_up(
        new_va + new_raw_size, sect_align
    )
    pe2.write(tmp_path)
    pe2.close()

    print(
        f"[+] Đã tạo và chèn thêm phân vùng .sbat mới: "
        f"Địa chỉ ảo RVA=0x{new_va:08X}, "
        f"Vị trí tệp FileOffset=0x{new_raw_offset:08X}, "
        f"Kích thước thô RawSize={new_raw_size} bytes."
    )


# Hàm xử lý chính công khai

def add_or_replace_sbat(efi_path: str, sbat_data: bytes, output_path: str) -> None:
    """
    Thêm phân vùng .sbat vào tệp *efi_path* (định dạng PE/COFF EFI) và ghi kết quả
    ra tệp *output_path*. Nếu phân vùng .sbat đã có sẵn, nội dung cũ sẽ bị thay thế.
    Tệp gốc sẽ không bị chỉnh sửa trừ khi đường dẫn *output_path* trùng với *efi_path*.
    """
    tmp_path = output_path + ".tmp"
    shutil.copy2(efi_path, tmp_path)

    try:
        pe = pefile.PE(tmp_path, fast_load=False)
        file_align = pe.OPTIONAL_HEADER.FileAlignment
        sect_align = pe.OPTIONAL_HEADER.SectionAlignment

        existing = find_sbat_section(pe)

        if existing:
            print(
                f"[*] Phát hiện phân vùng .sbat cũ tại địa chỉ "
                f"RVA=0x{existing.VirtualAddress:08X} — Tiến hành thay thế nội dung."
            )
            new_raw_size = align_up(len(sbat_data), file_align)
            if new_raw_size <= existing.SizeOfRawData:
                _update_inplace(tmp_path, pe, existing, sbat_data)
            else:
                _update_append(tmp_path, pe, existing, sbat_data,
                               file_align, sect_align)
        else:
            print("[*] Không tìm thấy phân vùng .sbat có sẵn — Tiến hành tạo mới.")
            _add_new_section(tmp_path, pe, sbat_data, file_align, sect_align)

    except Exception:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)
        raise

    os.replace(tmp_path, output_path)
    print(f"[+] Đã ghi tệp thành công ra: {output_path}")


# Giao diện dòng lệnh CLI

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Thêm hoặc thay thế một phân vùng .sbat trong tệp nhị phân UEFI EFI.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Ví dụ sử dụng:
  # Thêm cấu hình .sbat mặc định (chỉnh sửa trực tiếp trên tệp cũ)
  python3 add_sbat.py bootx64.efi

  # Đọc cấu hình .sbat từ tệp CSV bên ngoài và xuất ra tệp mới
  python3 add_sbat.py bootx64.efi --sbat my_sbat.csv --output bootx64_patched.efi
""",
    )
    parser.add_argument("efi_file",
                        help="Đường dẫn tới tệp EFI đầu vào.")
    parser.add_argument("--sbat", metavar="FILE_CSV",
                        help="Đường dẫn tới tệp văn bản cấu hình SBAT CSV "
                             "(Mặc định: Sử dụng cấu hình mẫu có sẵn).")
    parser.add_argument("--output", "-o", metavar="FILE_RA",
                        help="Đường dẫn tệp đầu ra (Mặc định: Ghi đè trực tiếp lên tệp đầu vào).")
    args = parser.parse_args()

    if not os.path.isfile(args.efi_file):
        print(f"[!] Không tìm thấy tệp: {args.efi_file}", file=sys.stderr)
        sys.exit(1)

    # Đọc nội dung dữ liệu thực SBAT
    if args.sbat:
        with open(args.sbat, "r", encoding="utf-8") as f:
            sbat_text = f.read()
    else:
        sbat_text = DEFAULT_SBAT
        print("[*] Không chỉ định tệp --sbat; hệ thống sử dụng cấu hình mẫu mặc định:")
        print(sbat_text)

    # Quy định dữ liệu SBAT bắt buộc phải kết thúc bằng ký tự rỗng NUL ở định dạng UTF-8
    sbat_bytes = sbat_text.encode("utf-8")
    if not sbat_bytes.endswith(b"\x00"):
        sbat_bytes += b"\x00"

    output_path = args.output if args.output else args.efi_file
    add_or_replace_sbat(args.efi_file, sbat_bytes, output_path)


if __name__ == "__main__":
    main()