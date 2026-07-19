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

message("${VNExos_CONFIG_DIR}")
