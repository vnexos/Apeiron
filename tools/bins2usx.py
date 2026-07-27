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

KEY_ARGS = [
    '--bin', '--map', '--out', '--arch'
]

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
    # TODO: Edit this
    return

def main():
    args = parse_arguments(sys.argv)
    if not (sys.argv[0].endswith('bins2usx.py') and len(args.keys()) >= 4):
        print('Thoát!')
        return
    print(args)

if __name__ == '__main__':
    print(sys.argv)
    main()
