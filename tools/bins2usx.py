#!/usr/bin/env python3
# =========================================================
# CÔNG CỤ ĐÓNG GÓI TỆP NHỊ PHÂN THÀNH TỆP USX
#
# Copyright (c) 2026 VNExos
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# LÝ DO CẦN TOOL NÀY:
#   - Định dạng tệp Thực thi bảo mật đa năng (USX) vừa mới
#   được thiết kế và chưa có công cụ biên dịch nào hỗ trợ
# =========================================================
"""
Chuyển đổi tệp Nhị phân phẳng sang USX

Sử dụng:
    python3 bins2usx.py --bin  <danh sách các tệp nhị phân>
                        --map  <danh sách các tệp ánh xạ>
                        --arch <dach sách các vi xử lý tương ứng>
                        --out  <tệp đầu ra>
                       [--version x.x.x.x]
"""
import sys
import ctypes
import re
import os
import random

from enum import IntFlag
from rich import print as pprint
from hashlib import sha3_256 as sha

_BUILTIN_TYPES = {
    "void": "v", "bool": "b", "char": "c", "signed char": "a",
    "unsigned char": "h", "short": "s", "unsigned short": "t",
    "int": "i", "unsigned int": "j", "unsigned": "j",
    "long": "l", "unsigned long": "m",
    "long long": "x", "unsigned long long": "y",
    "__int128": "n", "unsigned __int128": "o",
    "float": "f", "double": "d", "long double": "e", "__float128": "g",
    "wchar_t": "w", "char16_t": "Ds", "char32_t": "Di", "char8_t": "Du",
}

_TOKEN_RE = re.compile(r"const|volatile|&&|&|\*|[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*")
_TOKEN_FN = re.compile(r"^([^(]+)\(([^)]*)\)$")
_TOKEN_SYM = re.compile(r"^__usx_([^_]+)_(.+)$")

_UINT8  = ctypes.c_uint8
_UINT16 = ctypes.c_uint16
_UINT32 = ctypes.c_uint32
_UINT64 = ctypes.c_uint64

def _mangle_name_type(qualified_name: str) -> str:
    """Phân mảnh các tham số Struct/Class/Enum, có thể bao gồm cả Namespace"""
    parts = qualified_name.split('::')
    encoded = "".join(f"{len(p)}{p}" for p in parts)
    if len(parts) == 1:
        return encoded
    return f"N{encoded}E" # Có namespace

def _mangle_type(type: str) -> str:
    """Phân mảnh các kiểu dữ liệu của tham số"""
    tokens = _TOKEN_RE.findall(type)
    if not tokens:
        print(f"Kiểu dữ liệu không hợp lệ: {type!r}")
        exit(1)

    # Phân tách kiểu dữ liệu ra
    i = 0
    quals = []
    words = []
    while i < len(tokens):
        if tokens[i] not in ("const", "volatile", "*", "&", "&&"):
            words.append(tokens[i])
        elif tokens[i] in ("const", "volatile"):
            quals.append(tokens[i])
        else:
            break
        i += 1
    if not words:
        print(f"Không tồn tại dữ liệu chính: {type!r}")
        exit(1)

    # Lấy kiểu dữ liệu gốc
    base = " ".join(words)
    if base in _BUILTIN_TYPES:
        cur = _BUILTIN_TYPES[base]
    else:
        if len(words) != 1:
            print("Kiểu dữ liệu định nghĩa không được có nhiều từ!")
            exit(1)
        cur = _mangle_name_type(base)

    # Bỏ Const hoặc Volatile vào chuỗi
    if "volatile" in quals:
        cur = "V" + cur
    if "const" in quals:
        cur = "K" + cur

    while i < len(tokens):
        tok = tokens[i]

        if tok == '*':
            cur = "P" + cur
        elif tok == '&&':
            cur = "O" + cur
        elif tok == '&':
            cur = "R" + cur
        elif tok in ("const", "volatile"):
            # Để xử lý cho các trường hợp dị giáo
            group = []
            while i < len(tokens) and tokens[i] in ("const", "volatile"):
                group.append(tokens[i])
                i += 1
            i -= 1
            if "volatile" in group:
                cur = "V" + cur
            if "const" in group:
                cur = "K" + cur
        else:
            print(f"Ký tự không hợp lệ: {tok!r} trong {type!r}")
            exit(1)

        i += 1

    return cur

def mangle_function(function_name: str) -> str:
    """Phân mảnh tên hàm"""
    match = _TOKEN_FN.match(function_name)
    if not match:
        print(f"Tên hàm không hợp lệ: {function_name!r}")
        exit(1)

    function_name = match.group(1)
    function_args = "".join(_mangle_type(x) for x in match.group(2).split(','))

    return f"_Z{len(function_name)}{function_name}{function_args}"

_CRC32_TABLE = []
for i in range(256):
    crc = i
    for _ in range(8):
        if crc & 1:
            crc = (crc >> 1) ^ 0xEDB88320
        else:
            crc >>= 1
    _CRC32_TABLE.append(crc)

def calc_crc32(data: bytes) -> int:
    """Tính tổng kiểm của mảng Byte bằng thuật toán CRC32"""
    crc = 0xffffffff
    for byte in data:
        crc = _CRC32_TABLE[(crc ^ byte) & 0xff] ^ (crc >> 8)

    return (crc ^ 0xffffffff) & 0xffffffff

class USXSectionFlag(IntFlag):
    EXECUTABLE = 1 << 0  # 0b00000001
    WRITABLE   = 1 << 1  # 0b00000010
    ENCRYPTED  = 1 << 2  # 0b00000100
    COMPRESSED = 1 << 3  # 0b00001000
    ZERO_INIT  = 1 << 4  # 0b00010000
    TLS        = 1 << 5  # 0b00100000

class USXTargetArch(IntFlag):
    X86_64      = 0x0001
    AARCH64     = 0x0002
    RISCV64     = 0x0004
    X86         = 0x0010
    AARCH32     = 0x0020
    MIPS64      = 0x0040
    MIPS32      = 0x0080
    PPC64       = 0x0100
    SPARC64     = 0x0200
    S390X       = 0x0400
    LOONGARCH64 = 0x0800
    IA64        = 0x1000
    AVR         = 0x2000
    SUPERH      = 0x4000
    OTHER       = 0x8000

class USXHeader(ctypes.LittleEndianStructure):
    _pack_ = 1
    _layout_ = "ms"
    _fields_ = [
        ("magic_bytes", _UINT8 * 4),
        ("version", _UINT8),
        ("type", _UINT8),
        ("target_arch", _UINT16),

        ("app_version_offset", _UINT16),
        ("app_version_size", _UINT16),
        ("flags", _UINT16),
        ("header_size", _UINT16),

        ("entry_point", _UINT64),

        ("arch_table_offset", _UINT64),
        ("arch_table_count", _UINT16),
        ("arch_table_size", _UINT16),
        ("security_offset", _UINT64),
        ("security_size", _UINT32),
        ("string_table_offset", _UINT64),
        ("string_table_size", _UINT32),

        ("header_crc32", _UINT32),
    ]

class USXArch(ctypes.LittleEndianStructure):
    _pack_ = 1
    _layout_ = "ms"
    _fields_ = [
        ("arch_id", _UINT32),
        ("flags", _UINT8),
        ("reserved", _UINT8 * 3),

        ("section_table_offset", _UINT64),
        ("section_table_count", _UINT16),
        ("section_table_size", _UINT16),

        ("export_table_offset", _UINT64),
        ("export_table_count", _UINT32),
        ("export_table_size", _UINT32),

        ("import_table_offset", _UINT64),
        ("import_table_count", _UINT32),
        ("import_table_size", _UINT32),

        ("reserved2", _UINT32),
    ]

class USXSecurity(ctypes.LittleEndianStructure):
    _pack_ = 1
    _layout_ = "ms"
    _fields_ = [
        ("signature_offset", _UINT64),
        ("signature_size", _UINT32),

        ("kem_offset", _UINT64),
        ("kem_size", _UINT32),
    ]

class USXSection(ctypes.LittleEndianStructure):
    _pack_ = 1
    _layout_ = "ms"
    _fields_ = [
        ("name_offset", _UINT64),
        ("name_size", _UINT32),
        ("block_offset", _UINT64),
        ("block_size", _UINT32),

        ("initialization_vector", _UINT8 * 16),

        ("flags", _UINT16),
        ("reserved", _UINT8 * 6),
    ]
    def to_dict(self):
        return {
            field: list(getattr(self, field)) if hasattr(getattr(self, field), "_length_") 
                   else getattr(self, field)
            for field, _ in self._fields_
        }

class USXExport(ctypes.LittleEndianStructure):
    _pack_ = 1
    _layout_ = "ms"
    _fields_ = [
        ("name_offset", _UINT64),
        ("name_size", _UINT32),
        ("symbol_offset", _UINT64),
        ("symbol_type", _UINT8),
        ("reserved", _UINT8 * 3),
    ]

class USXImport(ctypes.LittleEndianStructure):
    _pack_ = 1
    _layout_ = "ms"
    _fields_ = [
        ("lib_name_offset", _UINT64),
        ("lib_name_size", _UINT32),

        ("symbol_name_offset", _UINT64),
        ("symbol_name_size", _UINT32),

        ("patch_offset", _UINT64),
    ]

KEY_ARGS = [
    '--bin', '--map', '--out', '--arch', '--version'
]

MAP_LINE_PATTERN = re.compile(
    r"^\s*(?P<vma>[0-9a-fA-F]+)\s+(?P<lma>[0-9a-fA-F]+)\s+(?P<size>[0-9a-fA-F]+)\s+(?P<align>[0-9a-fA-F]+)\s+(?P<name>.+)$"
)

SECTIONS = {
    ".text": { # Vùng chứa mã thực thi
        "name": "CODE",
        "flags": USXSectionFlag.EXECUTABLE,
    },
    ".rodata": { # Vùng chứa các hằng số (CONST) và không thể thực thi
        "name": "CNST",
        "flags": 0,
    },
    ".data": { # Vùng chứa các biến
        "name": "DATA",
        "flags": USXSectionFlag.WRITABLE,
    },
    ".bss": { # Vùng khởi tạo
        "name": "ZERO",
        "flags": USXSectionFlag.ZERO_INIT | USXSectionFlag.WRITABLE,
    },
}

def parse_arguments(argv) -> dict:
    """Chuyển đổi tham số truyền vào thành dạng dễ đọc hơn"""
    result = {}

    i = 1
    n = len(argv)

    while i < n:
        if argv[i] == '--bin':
            result['BIN'] = []
            while i + 1 < len(argv) and argv[i + 1] not in KEY_ARGS:
                result['BIN'].append(argv[i + 1])
                i += 1
        elif argv[i] == '--map':
            result['MAP'] = []
            while i + 1 < len(argv) and argv[i + 1] not in KEY_ARGS:
                result['MAP'].append(argv[i + 1])
                i += 1
        elif argv[i] == '--arch':
            result['ARCH'] = []
            while i + 1 < len(argv) and argv[i + 1] not in KEY_ARGS:
                result['ARCH'].append(argv[i + 1])
                i += 1
        elif argv[i] == '--out' and i + 1 < len(argv):
            i += 1
            result['OUT'] = argv[i]
        elif argv[i] == '--version' and i + 1 < len(argv):
            i += 1
            result['VERSION'] = argv[i]
        elif i < n:
            print(f"{i} - {argv[i]}")
            print('Sai tham số truyền vào. Dùng lệnh: ')
            print("""
    python3 bins2usx.py --bin  <danh sách các tệp nhị phân>
                        --map  <danh sách các tệp ánh xạ>
                        --arch <dach sách các vi xử lý tương ứng>
                        --out  <tệp đầu ra>
            """)
            return {}

        i += 1
    
    if not len(result['ARCH']) == len(result['BIN']):
        print('Danh sách tệp nhị phân và danh sách tệp ánh xạ không đều nhau. Dùng lệnh: ')
        print("""
    python3 bins2usx.py --bin  <danh sách các tệp nhị phân>
                        --map  <danh sách các tệp ánh xạ>
                        --arch <dach sách các vi xử lý tương ứng>
                        --out  <tệp đầu ra>
        """)
        return {}

    if not len(result['BIN']) == len(result['MAP']):
        print('Danh sách tệp nhị phân và danh sách tệp ánh xạ không đều nhau. Dùng lệnh: ')
        print("""
    python3 bins2usx.py --bin  <danh sách các tệp nhị phân>
                        --map  <danh sách các tệp ánh xạ>
                        --arch <dach sách các vi xử lý tương ứng>
                        --out  <tệp đầu ra>
        """)
        return {}

    return result

IMPORT_PTR_PREFIX = '__usx_imp_ptr_'

section_hashes = {}
data_check = []

def read_map(arch_name: str, map_file: str, bin_file: str, output: str) -> list[dict]:
    """Đọc tệp ánh xạ và trả về kiểu dữ liệu của Python"""
    # Đọc tệp thành một danh sách phẳng
    parsed_map = []
    try:
        with open(map_file, "r", encoding='utf-8') as file:
            max_offset = 0 # Để bỏ qua các cấu trúc thừa trong tệp ánh xạ

            for line in file:
                if not line.strip():
                    continue

                # Kiểm tra định dạng của dòng hiện tại
                match = MAP_LINE_PATTERN.match(line)
                if not match:
                    continue

                # Chia các dòng bên trong thành những nhóm có gắn nhãn
                data = match.groupdict()
                currentOffset = int(data['lma'], 16)

                if max_offset < currentOffset:
                    max_offset = currentOffset

                # Bỏ qua các dòng có `size` là 0 hoặc là các dòng không theo thứ tự
                if int(data['size'], 16) == 0 or currentOffset < max_offset:
                    continue

                # Phân loại dựa trên cấu trúc của tên
                name = data['name'].strip()
                if name.startswith(".") and ".o" not in name:
                    type = "SECTION"
                elif ".o:(" in name:
                    type = "NESTED"
                else:
                    type = "SYMBOL"
                
                parsed_map.append({
                    "name": name,
                    "type": type,
                    "offset": currentOffset,
                    "size": int(data['size'], 16),
                    "align": int(data['align'], 16)
                })
    except Exception:
        print('Lỗi xảy ra trong quá trình đọc tệp ánh xạ.')
        exit(1)

    # Đọc danh sách phẳng vào một cấu trúc tường minh
    result = {
        # Cấu trúc của phân vùng
        # - name: tên phân vùng
        # - block_offset: vị trí phân vùng
        # - block_size: kích thước phân vùng
        # - inialization_vector: mảng khởi tạo 16 byte
        # - flags: cờ trạng thái
        "SECTIONS": [],
        # Cấu trúc của bảng Xuất
        # - name: tên phần tử
        # - offset: vị trí của phần tử
        # - type: phân loại phần tử (hàm/biến)
        "EXPORTS": [],
        # Cấu trúc của bảng Nhập
        # - lib: tên thư viện
        # - name: tên phần tử
        # - offset: vị trí của phần tử cần được vá vào
        "IMPORTS": []
    }

    i = 0
    n = len(parsed_map)
    tmp_import = {}

    # Chuyển đổi 1 lần nữa thành dạng tường minh hơn cho cấu trúc USX
    while i < n:
        # Nếu là phân vùng thì chỉ cần đơn giản là đẩy vào
        if parsed_map[i]['type'] == 'SECTION':
            if parsed_map[i]['name'] in SECTIONS:
                result['SECTIONS'].append({
                    "name": SECTIONS[parsed_map[i]['name']]['name'], # Đổi tên phân vùng
                    "block_offset": parsed_map[i]['offset'],
                    "block_size": parsed_map[i]['size'],
                    "inialization_vector": [],
                    "flags": int(SECTIONS[parsed_map[i]['name']]['flags']) # Các cờ của phân vùng
                })
            elif parsed_map[i]['name'] == '.vnexos_usx_import':
                i += 1
                while i < n and not parsed_map[i]['type'] == 'SECTION':
                    match = _TOKEN_SYM.match(parsed_map[i]['name'])
                    if match:
                        name = re.search(r'^([^(]+)\(?', match.group(2)).group(1)
                        tblName = match.group(1)

                        if name not in tmp_import:
                            tmp_import[name] = {}

                        if tblName == 'sym':
                            tmp_import[name]['name'] = mangle_function(match.group(2))
                        elif tblName == 'lib':
                            map_elem = parsed_map[i]
                            tmp_import[name]['lib_name'] = map_elem['offset']
                            tmp_import[name]['lib_size'] = map_elem['size']
                    i += 1
                i -= 1

        elif parsed_map[i]['type'] == 'NESTED':
            # Tìm tên phân vùng trong dấu ngoặc đơn
            name = re.search(
                r"\(([^)]+)\)", parsed_map[i]['name']
            ).group(1)
            if ".vnexos_usx_export" in name:
                # Phân loại phân vùng xuất
                if name.startswith(".text"):
                    type = 0
                elif name.startswith(".data"):
                    type = 1
                else:
                    type = 2
                
                i += 1
                while i < n and parsed_map[i]['type'] == 'SYMBOL':
                    # Tên hàm/tên biến
                    if type == 0:
                        symname = mangle_function(parsed_map[i]['name'])
                    else:
                        symname = parsed_map[i]['name']

                    result['EXPORTS'].append({
                        "name": symname,
                        "offset": parsed_map[i]['offset'],
                        "type": type
                    })
                    i += 1
                i -= 1
            elif ".vnexos_usx_import.ptr" in name:
                i += 1
                while i < n and parsed_map[i]['type'] == 'SYMBOL':
                    key_name = parsed_map[i]['name']

                    if key_name not in tmp_import:
                        tmp_import[key_name] = {}

                    tmp_import[key_name]['offset'] = parsed_map[i]['offset']
                    i += 1
                i -= 1
        i += 1

    try:
        with open(bin_file, "rb") as file:
            # Đọc tệp nhị phân để lấy tên thư viện
            for import_field in tmp_import:
                offset = tmp_import[import_field]['lib_name']
                size = tmp_import[import_field]['lib_size']

                del tmp_import[import_field]['lib_name']
                del tmp_import[import_field]['lib_size']

                file.seek(offset)
                raw = file.read(size - 1)

                tmp_import[import_field]['lib'] = raw.decode('utf-8')

                result["IMPORTS"].append(tmp_import[import_field])

            # Ghi các phân vùng vào tệp tạm
            with open(f"{output}.tmp", "ab") as tmp_file:
                section_hashes[arch_name] = {}
                for section in result['SECTIONS']:
                    if section['name'] == 'ZERO':
                        continue

                    file.seek(section["block_offset"])
                    raw = file.read(section["block_size"])

                    section_hash = sha(raw).digest()
                    section_hashes[arch_name][section['name']] = section_hash

                    if section_hash in data_check:
                        continue

                    data_check.append(section_hash)
                    
                    aligned_block_size = align8(section['block_size'])
                    while len(raw) < aligned_block_size:
                        raw += b'\0'

                    tmp_file.write(raw)

    except Exception as e:
        print(f"Lỗi xảy ra trong quá trình đọc tệp nhị phân. {e}")
        exit(1)

    return result

def parse_arch(args):
    """Gom toàn bộ các thông tin lại để chuẩn bị cho giai đoạn đóng gói"""
    result = {
        "archs": {}
    }

    string_table = ""

    def add_string(string: str):
        nonlocal string_table
        if string not in string_table:
            string_table += string
        return (string_table.find(string), len(string))

    (v_offset, v_length) = add_string(args['VERSION'])

    for i, arch in enumerate(args['ARCH']):
        arch_data = {
            "sections": [],
            "exports": [],
            "imports": []
        }

        map = args['MAP'][i]
        for section in map['SECTIONS']:
            section_struct = USXSection()

            (offset, length) = add_string(section['name'])

            section_struct.name_offset = offset
            section_struct.name_size = length
            section_struct.block_offset = section['block_offset']
            section_struct.block_size = section['block_size']
            section_struct.flags = section['flags']

            arch_data["sections"].append(section_struct)

        for export in map['EXPORTS']:
            export_struct = USXExport()

            (offset, length) = add_string(export['name'])

            export_struct.name_offset = string_table.find(export['name'])
            export_struct.name_size = len(export['name'])
            export_struct.symbol_offset = export['offset']
            export_struct.symbol_type = export['type']

            arch_data["exports"].append(export_struct)

        for import_data in map['IMPORTS']:
            import_struct = USXImport()

            (lib_offset, lib_len) = add_string(import_data['lib'])
            (name_offset, name_len) = add_string(import_data['name'])

            import_struct.lib_name_offset = lib_offset
            import_struct.lib_name_size = lib_len
            import_struct.symbol_name_offset = name_offset
            import_struct.symbol_name_size = name_len
            import_struct.patch_offset = import_data['offset']

            arch_data["imports"].append(import_struct)

        result['archs'][arch] = arch_data

    result['string_table'] = string_table
    return (result, v_offset, v_length)

def align8(n: int) -> int:
    """Căn lên 8 lề cho một số nguyên"""
    return (n + 7) & (~7)

def calc_section_headers_and_blocks_end_offset(current_index: int, usx_info: dict, tmp_file) -> tuple[int, any]:
    """Tính toán vị trí kết thúc của toàn bộ bảng phân vùng và các khối phân vùng"""
    current_index += len(usx_info['archs']) * ctypes.sizeof(USXArch)
    data_check = []
    result = {
        "section": current_index,
        "blocks": current_index,
        "imports": 0,
        "exports": 0
    }

    def calc_data(data_arr: list):
        nonlocal current_index
        nonlocal data_check

        # Dồn các Byte trong mảng dữ liệu
        raw = bytes()
        for data in data_arr:
            data_bytes = bytes(data)
            raw += data_bytes
        # Mã hóa để làm khóa
        hash = sha(raw).digest()
        # Kiểm tra khối dữ liệu đã nằm trong danh sách kiểm tra chưa
        if hash in data_check:
            return
        # Nếu chưa thì thêm khóa vào danh sách để tránh thêm trùng lặp
        data_check.append(hash)
        current_index += len(raw)

    total_size = 0
    for arch in usx_info['archs']:
        arch_data = usx_info['archs'][arch]

        # Tính tổng kích thước của khối phân vùng
        total_section_size = len(arch_data['sections']) * ctypes.sizeof(USXSection)
        current_index += total_section_size

        # Tính vị trí của khối nội dung phân vùng
        result["blocks"] += total_section_size

        # Tính vị trí của khối xuất
        tmp_index = current_index
        calc_data(arch_data['imports'])
        result['exports'] += current_index - tmp_index

        # Tính kích thước khối xuất
        tmp_index = current_index
        calc_data(arch_data['exports'])
        total_size += current_index - tmp_index


    # Kích thước của khối nội dung phân vùng (đã được ghi vào tệp tạm)
    block_size = os.path.getsize(tmp_file)

    # Tính toán lại kích thước của khối nhập và khối xuất
    result['imports'] += result['blocks'] + block_size
    result['exports'] += result['imports']

    # Trả về vị trí cho các khối sau khối xuất và bản đồ cho từng khối
    return (result['exports'] + total_size, result)

def pack_usx(args):
    """Đóng gói toàn bộ các thông tin thành tệp USX"""
    current_index = 0

    header = USXHeader()
    header.magic_bytes = (_UINT8 * 4)(*b'USX\x00')
    header.version     = 1
    header.type        = 0
    header.target_arch = 0

    # Đánh dấu các Vi xử lý mà tệp USX hỗ trợ
    for arch in args['ARCH']:
        upper_arch = arch.upper()

        if hasattr(USXTargetArch, upper_arch):
            header.target_arch |= USXTargetArch[upper_arch]
        else:
            header.target_arch |= USXTargetArch['OTHER']

    (parsed_arch, version_offset, version_size) = parse_arch(args)

    header.app_version_offset = version_offset
    header.app_version_size = version_size
    header.flags = 0b1000 # Tệp thực thi độc lập vị trí
    header.header_size = ctypes.sizeof(USXHeader)

    header.entry_point = 0 # Bằng 0 vì độc lập vị trí

    current_index += header.header_size
    header.arch_table_offset = current_index
    header.arch_table_count = len(args['ARCH'])
    header.arch_table_size = ctypes.sizeof(USXArch)
    
    (current_index, map) = calc_section_headers_and_blocks_end_offset(current_index, parsed_arch, f"{args['OUT']}.tmp")
    header.security_offset = current_index
    header.security_size = ctypes.sizeof(USXSecurity)

    header.string_table_offset = current_index + header.security_size
    header.string_table_size = len(parsed_arch['string_table'])

    # Tính tổng kiểm của khối tiêu đề
    header.header_crc32 = calc_crc32(bytes(header)[:-4])

    data_check = {}

    def concat_arr(data_arr) -> bytes:
        raw = bytes()
        for data in data_arr:
            data_raw = bytes(data)
            raw += data_raw
        return raw

    def gen_iv():
        res = []
        for _ in range(16):
            res.append(random.randint(0, 0xff))
        return res

    try:
        with open(args['OUT'], "wb") as file:
            file.write(bytes(header))

            ie_hash = {
                'id': bytes(),
                'ed': bytes()
            }

            # Thêm bảng vi xử lý vào tệp
            for arch in args['ARCH']:
                ie_hash[arch] = {}
                arch_data = USXArch()

                arch_data.arch_id = USXTargetArch[arch.upper()]
                arch_data.flags = 0

                # Thông tin của bảng phân vùng
                arch_data.section_table_offset = map['section']
                arch_data.section_table_count = len(parsed_arch['archs'][arch]['sections'])
                arch_data.section_table_size = ctypes.sizeof(USXSection)
                map['section'] += arch_data.section_table_count * arch_data.section_table_size

                # Tính toán thông tin của bảng nhập
                tmp_byte = concat_arr(parsed_arch['archs'][arch]['imports'])
                ie_hash[arch]['i'] = sha(tmp_byte).digest()
                if ie_hash[arch]['i'] not in data_check:
                    ie_hash['id'] += tmp_byte
                    data_check[ie_hash[arch]['i']] = {
                        'offset': map['imports'],
                        'count': len(parsed_arch['archs'][arch]['imports']),
                        'size': ctypes.sizeof(USXImport)
                    }
                    map['imports'] += data_check[ie_hash[arch]['i']]['count'] * data_check[ie_hash[arch]['i']]['size']
                arch_data.import_table_offset = data_check[ie_hash[arch]['i']]['offset']
                arch_data.import_table_count = data_check[ie_hash[arch]['i']]['count']
                arch_data.import_table_size = data_check[ie_hash[arch]['i']]['size']

                # Tính toán thông tin của bảng xuất
                tmp_byte = concat_arr(parsed_arch['archs'][arch]['exports'])
                ie_hash[arch]['e'] = sha(tmp_byte).digest()
                if ie_hash[arch]['e'] not in data_check:
                    ie_hash["ed"] += tmp_byte
                    data_check[ie_hash[arch]['e']] = {
                        'offset': map['exports'],
                        'count': len(parsed_arch['archs'][arch]['exports']),
                        'size': ctypes.sizeof(USXExport)
                    }
                    map['exports'] += data_check[ie_hash[arch]['e']]['count'] * data_check[ie_hash[arch]['e']]['size']
                arch_data.export_table_offset = data_check[ie_hash[arch]['e']]['offset']
                arch_data.export_table_count = data_check[ie_hash[arch]['e']]['count']
                arch_data.export_table_size = data_check[ie_hash[arch]['e']]['size']

                file.write(bytes(arch_data))

            # Ghi thêm các bảng phân vùng vào tệp
            for i, arch in enumerate(args['ARCH']):
                for j, section in enumerate(args['MAP'][i]["SECTIONS"]):
                    section_data = parsed_arch["archs"][arch]['sections'][j]
                    if section['name'] == 'ZERO':
                        section_data.block_offset = 0
                    else:
                        # Lấy mã băm làm khóa để tiện so sánh và lưu trữ các khối trùng lặp
                        hash = section_hashes[arch][section['name']]
                        if hash not in data_check:
                            data_check[hash] = {
                                'block_offset': map['blocks'],
                                'iv': (_UINT8 * 16)(*gen_iv())
                            }
                            map['blocks'] += align8(section_data.block_size)

                        # Tái sử dụng dữ liệu đã được lưu vào bảng băm
                        section_data.block_offset = data_check[hash]['block_offset']
                        section_data.initialization_vector = data_check[hash]['iv']
                    file.write(bytes(section_data))

            # Ghi thêm tệp tạm vào tệp kết quả
            with open(f"{args['OUT']}.tmp", "rb") as tmp_file:
                while True:
                    chunk = tmp_file.read(1024)
                    if not chunk:
                        break
                    file.write(chunk)

            # Ghi thêm các khối nhập và xuất vào tệp
            file.write(ie_hash['id'])
            file.write(ie_hash['ed'])

            # Ghi thêm bảng bảo mật trống vào tệp
            security = USXSecurity()
            file.write(bytes(security))

            # Ghi bảng chuỗi vào cuối tệp (căn lề 8)
            string_table = bytes(parsed_arch["string_table"], encoding="utf-8")
            align_table_size = align8(len(string_table))

            while len(string_table) < align_table_size:
                string_table += b'\0'

            file.write(string_table)
        os.remove(f"{args['OUT']}.tmp")

    except Exception as e:
        print(f"Có lỗi xãy ra trong quá trình ghi tệp kết quả! {e}")
        exit(1)

def main():
    args = parse_arguments(sys.argv)
    if not (sys.argv[0].endswith('bins2usx.py') and len(args.keys()) >= 4):
        exit(1)

    tmp_path = f"{args['OUT']}.tmp"
    if os.path.exists(tmp_path):
        os.remove(tmp_path)

    global data_check, section_hashes
    data_check.clear()
    section_hashes.clear()
    
    for i, mapFile in enumerate(args['MAP']):
        args['MAP'][i] = read_map(args['ARCH'][i], mapFile, args['BIN'][i], args["OUT"])

    pack_usx(args)

if __name__ == '__main__':
    main()
