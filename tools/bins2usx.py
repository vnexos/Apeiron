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
"""
import sys
import struct
import re

from enum import IntFlag
from rich import print as pprint

class USXSectionFlag(IntFlag):
    EXECUTABLE = 1 << 0  # 0b00000001
    WRITABLE   = 1 << 1  # 0b00000010
    ENCRYPTED  = 1 << 2  # 0b00000100
    COMPRESSED = 1 << 3  # 0b00001000
    ZERO_INIT  = 1 << 4  # 0b00010000
    TLS        = 1 << 5  # 0b00100000

KEY_ARGS = [
    '--bin', '--map', '--out', '--arch'
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
            while argv[i + 1] not in KEY_ARGS and i + 1 < len(argv):
                result['BIN'].append(argv[i + 1])
                i += 1
        elif argv[i] == '--map':
            result['MAP'] = []
            while argv[i + 1] not in KEY_ARGS and i + 1 < len(argv):
                result['MAP'].append(argv[i + 1])
                i += 1
        elif argv[i] == '--arch':
            result['ARCH'] = []
            while argv[i + 1] not in KEY_ARGS and i + 1 < len(argv):
                result['ARCH'].append(argv[i + 1])
                i += 1
        elif argv[i] == '--out' and i + 1 < len(argv):
            i += 1
            result['OUT'] = argv[i]
            i += 1
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

def read_map(map_file: str) -> list[dict]:
    """Đọc tệp ánh xạ và trả về kiểu dữ liệu của Python"""
    # Đọc tệp thành một danh sách phẳng
    parsed_map = []
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

    # Chuyển đổi 1 lần nữa thành dạng tường minh hơn cho cấu trúc USX
    while i < n:
        # Nếu là phân vùng thì chỉ cần đơn giản là đẩy vào
        if parsed_map[i]['type'] == 'SECTION':
            if parsed_map[i]['name'] in SECTIONS:
                result['SECTIONS'].append({
                    "name": SECTIONS[parsed_map[i]['name']]['name'],
                    "block_offset": parsed_map[i]['offset'],
                    "block_size": parsed_map[i]['size'],
                    "inialization_vector": [],
                    "flags": int(SECTIONS[parsed_map[i]['name']]['flags'])
                })
        elif parsed_map[i]['type'] == 'NESTED':
            name = re.search(
                r"\(([^)]+)\)", parsed_map[i]['name']
            ).group(1)
            if ".vnexos_usx_export" in name:
                if name.startswith(".text"):
                    type = 0
                elif name.startswith(".data"):
                    type = 1
                else:
                    type = 2
                
                i += 1
                while i < n and parsed_map[i]['type'] == 'SYMBOL':
                    symname = re.search(
                        r"([^(]+)", parsed_map[i]['name']
                    ).group(1)

                    result['EXPORTS'].append({
                        "name": symname,
                        "offset": parsed_map[i]['offset'],
                        "type": type
                    })
                    i += 1
                i -= 1

        i += 1

    return result

def main():
    args = parse_arguments(sys.argv)
    if not (sys.argv[0].endswith('bins2usx.py') and len(args.keys()) >= 4):
        print('Thoát!')
        return
    
    for i,mapFile in enumerate(args['MAP']):
        args['MAP'][i] = read_map(mapFile)
    pprint(args['MAP'][0])

if __name__ == '__main__':
    main()
