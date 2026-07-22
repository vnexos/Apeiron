# =========================================================
# VNExos Bản Nguyên - HỆ ĐIỀU HÀNH VIỆT NAM
# VNExos Apeiron - VIETNAMESE OPERATING SYSTEM
#
# Copyright (c) 2026 VNExos
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

# Thư viện chung
add_subdirectory(shared)

add_subdirectory(grub)
add_subdirectory(internal)

get_property(ALL_TARGETS GLOBAL PROPERTY VNExos_ALL_TARGET)

# Sinh ra tệp ảnh đĩa (disk.img)
set(VNEXOS_BOOT_UUID "2ef6ee17-2a14-4db9-8129-42a6dc48f8af")
set(VNEXOS_TYPE_UUID "c48d1722-723b-43d6-bead-f2b2623343dd")
set(VNEXOS_PART_UUID "9c7ba30b-e2ee-4bbe-9fe4-ab5f23fe2b9b")
set(DISK_IMG "${CMAKE_SOURCE_DIR}/disk.img")

add_custom_command(
    OUTPUT "${DISK_IMG}"
    COMMAND dd if=/dev/zero of="${DISK_IMG}" bs=1M count=1024 status=none
    COMMAND sgdisk -o "${DISK_IMG}" > /dev/null
    COMMAND sgdisk -n 1:2048:+30MB -t 1:ef00 -c 1:"Bộ nạp khởi động VNExos" -u 1:${VNEXOS_BOOT_UUID} "${DISK_IMG}" > /dev/null
    COMMAND sgdisk -n 2:0:0 -t 2:${VNEXOS_TYPE_UUID} -c 2:"Nhân lõi Apeiron" -u 2:${VNEXOS_PART_UUID} "${DISK_IMG}" > /dev/null
    COMMAND mkfs.vfat -F 32 --offset=2048 -s 1 -n "VNEXOS_BOOT" "${DISK_IMG}" 61440 > /dev/null 2>&1
    COMMAND mcopy -s -i "${DISK_IMG}@@1M" "${VNExos_SYSROOT_DIR}/*" ::/ 2>/dev/null
    COMMENT "[ VNExos ] Đã tạo thành công disk image: ${DISK_IMG}"
    DEPENDS ${ALL_TARGETS}
)

add_custom_target(disk_img ALL DEPENDS "${DISK_IMG}")
