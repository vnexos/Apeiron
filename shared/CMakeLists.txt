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

set(VNExos_SHARED_SOURCES ${SHARED_SOURCES} PARENT_SCOPE)

set(VNExos_SHARED_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
