# =========================================================
# Copyright (c) 2026 VNExos
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Triển khai các hàm lọc mã Assembly theo từng Vi xử lý
# =========================================================

macro(VNExosFilterAssemblySource ARCH SRC_FILES)
    string(TOLOWER "${ARCH}" _LOWER_ARCH)
    set(_FILTERED_SRC_FILES "")
    foreach(_FILE IN LISTS ${SRC_FILES})
        # Kiểm tra file có dạng .<arch>.(s|S|asm|ASM) hay không
        if(_FILE MATCHES "\\.([a-zA-Z0-9_-]+)\\.([sS]|ASM|asm)$")
            string(TOLOWER "${CMAKE_MATCH_1}" _FILE_ARCH)
            # Chỉ giữ lại nếu đúng kiến trúc đang biên dịch
            if(_FILE_ARCH STREQUAL _LOWER_ARCH)
                list(APPEND _FILTERED_SRC_FILES "${_FILE}")
            endif()
        else()
            # File C/C++ hoặc file ASM dùng chung không mang nhãn kiến trúc
            list(APPEND _FILTERED_SRC_FILES "${_FILE}")
        endif()
    endforeach()
    set(${SRC_FILES} "${_FILTERED_SRC_FILES}")
endmacro()