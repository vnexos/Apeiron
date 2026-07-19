# =========================================================
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Triển khai các hàm lọc mã Assembly theo từng Vi xử lý
# =========================================================

macro(VNExosFilterAssemblySource ARCH SRC_FILES)
    foreach(ARCH_FILTER IN LISTS VNExos_ARCHS)
        if(NOT ARCH_FILTER STREQUAL "${ARCH}")
            string(TOLOWER "${ARCH_FILTER}" LOWER_ARCH_FILTER)
            string(TOUPPER "${ARCH_FILTER}" UPPER_ARCH_FILTER)

            list(FILTER SRC_FILES EXCLUDE REGEX "\\.(${LOWER_ARCH_FILTER}|${UPPER_ARCH_FILTER})\\.([sS]|ASM|asm)$")
        endif()
    endforeach()
endmacro()