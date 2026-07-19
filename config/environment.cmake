# =========================================================
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Cấu hình cho môi trường biên dịch trong dự án
# =========================================================

# Sử dụng chuẩn 23 cho C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sử dụng chuẩn 17 cho C
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Mặc định Hệ điều hành sẽ hỗ trợ cho toàn bộ các Vi xử lý sau
set(VNExos_ARCHS "x86_64;aarch64;riscv64" CACHE STRING "Danh sách các kiến trúc cần biên dịch")

# Cấu hình các tệp hệ thống
set(VNExos_SYSTEM_FIRMWARE_DIR "/usr/share/edk2"                 CACHE PATH "Thư mục hệ thống chứa các tệp phần sụn")

# Cấu hình các tệp chung
set(VNExos_CERT_DIR "${CMAKE_SOURCE_DIR}/cert"                   CACHE PATH "Thư mục chứa các tệp chứng chỉ")
set(VNExos_ARCH_DIR "${VNExos_CONFIG_DIR}/arch"                  CACHE PATH "Thư mục chứa các cấu hình đại diện Vi xử lý")
set(VNExos_FIRMWARE_DIR "${CMAKE_SOURCE_DIR}/firmware"           CACHE PATH "Thư mục chứa các tệp phần sụn")
set(VNExos_TOOL_DIR "${CMAKE_SOURCE_DIR}/tools"                  CACHE PATH "Thư mục chứa các công cụ kịch bản đơn giản")
set(VNExos_ASSET_DIR "${CMAKE_SOURCE_DIR}/assets"                CACHE PATH "Thư mục chứa tài nguyên")

# Cấu hình các tệp đầu ra
set(VNExos_SYSROOT_DIR "${CMAKE_SOURCE_DIR}/sysroot"             CACHE PATH "Thư mục chứa các tệp trong phân vùng UEFI trên ổ đĩa")
set(VNExos_SYSROOT_CERT_DIR "${VNExos_SYSROOT_DIR}/certs"        CACHE PATH "Thư mục chứa các chứng chỉ số")
set(VNExos_SYSROOT_KERN_DIR "${VNExos_SYSROOT_DIR}"              CACHE PATH "Thư mục chứa nhân lõi Hệ điều hành")
set(VNExos_SYSROOT_BOOT_DIR "${VNExos_SYSROOT_DIR}/EFI/BOOT"     CACHE PATH "Thư mục chứa các chương trình khởi động của UEFI")
set(VNExos_SYSROOT_LOGO_DIR "${VNExos_SYSROOT_DIR}/assets/logos" CACHE PATH "Thư mục chứa các biểu trưng")

# Cấu hình cho tệp chứng chỉ để ký bộ nạp mồi
set(VNExos_GRUB_CERT "open"                                    CACHE STRING "Tên của tệp chứng chỉ để ký chương trình")

# Kiểm tra các công cụ cho môi trường
find_program(VNExos_SBSIGN_TOOL sbsign)
if(NOT VNExos_SBSIGN_TOOL)
    message(FATAL_ERROR "Không tìm thấy công cụ sbsign!")
endif()

find_program(VNExos_BOREAS_TOOL boreas)
if(NOT VNExos_BOREAS_TOOL)
    message(FATAL_ERROR "Không tìm thấy công cụ VNExos Ám Băng (VNExos Boreas)!")
endif()
