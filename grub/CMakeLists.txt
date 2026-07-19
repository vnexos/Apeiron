# =========================================================
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Quản lý các tệp mã nguồn trong Bộ nạp mồi
# =========================================================

file(GLOB_RECURSE GRUB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)



foreach(ARCH IN LISTS VNExos_ARCHS)
    set(FUNC_NAME "VNExosBuildEfi_${ARCH}")
    if(COMMAND ${FUNC_NAME})
        if(ARCH STREQUAL "riscv64")
            set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker_grub_riscv64.ld")
            cmake_language(CALL ${FUNC_NAME} "BOOT" "vnexos_db" "open" "${GRUB_SOURCES}" FALSE "${LINKER_SCRIPT}")
        else()
            cmake_language(CALL ${FUNC_NAME} "BOOT" "vnexos_db" "open" "vnexos_grub_main" "${GRUB_SOURCES}" FALSE)
        endif()
    endif()
endforeach()