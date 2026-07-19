# =========================================================
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Quản lý thư viện chung của dự án
# =========================================================

# Lấy hết mọi tệp mã nguồn trong thư viện chung
file(GLOB_RECURSE SHARED_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

include(${VNExos_CONFIG_DIR}/source_filter.cmake)

foreach(ARCH IN LISTS VNExos_ARCHS)
    set(ARCH_SOURCES ${SHARED_SOURCES})
    VNExosFilterAssemblySource("${ARCH}" ARCH_SOURCES)
    
    set(TARGET_SHARED "vnexos_shared_${ARCH}")
    add_library(${TARGET_SHARED} STATIC ${ARCH_SOURCES})
    
    # Kế thừa toàn bộ cờ kiến trúc thông qua INTERFACE library
    target_link_libraries(${TARGET_SHARED} PUBLIC vnexos_flags_${ARCH})
    
    # Export thư mục inc để các target khác thấy khi link với shared
    target_include_directories(${TARGET_SHARED} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()
