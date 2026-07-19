# =========================================================
# VNExos Bản Nguyên - HỆ ĐIỀU HÀNH VIỆT NAM
# VNExos Apeiron - VIETNAMESE OPERATING SYSTEM
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================
cmake_minimum_required(VERSION 4.2)

# Tích hợp cấu hình trình biên dịch mặc định cho dự án
set(VNExos_CONFIG_DIR "${CMAKE_SOURCE_DIR}/config")
set(CMAKE_TOOLCHAIN_FILE "${VNExos_CONFIG_DIR}/compiler.cmake")

# Khai báo dự án
project(VNExos_Apeiron LANGUAGES CXX C ASM)

# Lấy cấu hình môi trường
include(${VNExos_CONFIG_DIR}/environment.cmake)

# Sao chép toàn bộ tệp phần sụn nếu chưa có
if(NOT EXISTS "${VNExos_FIRMWARE_DIR}/.ready")
    file(COPY  "${VNExos_SYSTEM_FIRMWARE_DIR}/" DESTINATION "${VNExos_FIRMWARE_DIR}")
    file(WRITE "${VNExos_FIRMWARE_DIR}/.ready" "")

    message(STATUS "[ VNExos ] Sao chép phần sụn thành công!")
endif()

# Khởi tạo thư mục `sysroot`
if(NOT EXISTS "${VNExos_SYSROOT_DIR}")
    # Tạo các thư mục cần thiết
    file(MAKE_DIRECTORY "${VNExos_SYSROOT_CERT_DIR}")
    file(MAKE_DIRECTORY "${VNExos_SYSROOT_LOGO_DIR}")
    file(MAKE_DIRECTORY "${VNExos_SYSROOT_BOOT_DIR}")

    # Sao chép các tệp chứng chỉ
    file(COPY "${VNExos_CERT_DIR}/root.crt" DESTINATION "${VNExos_SYSROOT_CERT_DIR}/")
    file(COPY "${VNExos_CERT_DIR}/open.crt" DESTINATION "${VNExos_SYSROOT_CERT_DIR}/")
    file(COPY "${VNExos_CERT_DIR}/vnexos_db.crt" DESTINATION "${VNExos_SYSROOT_CERT_DIR}/")

    # Sao chép biểu trưng
    file(GLOB LOGO_FILES "${VNExos_ASSET_DIR}/*.bmp")
    if(LOGO_FILES) # Chỉ copy nếu tìm thấy ảnh
        file(COPY ${LOGO_FILES} DESTINATION "${VNExos_SYSROOT_LOGO_DIR}/")
    endif()
endif()

# Nhập toàn bộ các tệp cấu hình Vi xử lý
foreach(ARCH IN LISTS VNExos_ARCHS)
    include(${VNExos_ARCH_DIR}/${ARCH}.cmake)
endforeach()

# Khởi tạo mảng toàn cục rỗng
set_property(GLOBAL PROPERTY VNExos_ALL_TARGET "")

add_subdirectory(shared)
add_subdirectory(grub)
add_subdirectory(internal)

get_property(ALL_TARGETS GLOBAL PROPERTY VNExos_ALL_TARGET)