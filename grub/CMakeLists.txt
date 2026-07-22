# =========================================================
# Copyright (c) 2026 VNExos
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
            cmake_language(CALL ${FUNC_NAME}
                "BOOT"                # Tên của tệp đầu ra + ARCH. Ví dụ như BOOTX64.EFI
                # Khai báo khóa để ký tệp chương trình
                # Khóa riêng tư sẽ có dạng : <tên>.key
                # Khóa công khai sẽ có dạng: <tên>.crt
                "vnexos_db"           # Tên của khóa DB để ký tệp EFI
                "${VNExos_GRUB_CERT}" # Tên của khóa Dilithium để ký tệp EFI
                "${GRUB_SOURCES}"     # Danh sách các tệp mã nguồn
                FALSE                 # Có dùng ký tự in thường không? FALSE - Dùng ký tự in hoa
                "${LINKER_SCRIPT}"    # Đối với riêng dòng RISC-V thì phải biên dịch thông qua 
                                      # ELF sau đó chuyển đổi sang định dạng EFI. Nên thay vì
                                      # khai báo hàm đầu vào, ta sẽ khai báo tệp liên kết.
            )
        else()
            cmake_language(CALL ${FUNC_NAME}
                "BOOT"                # Tên của tệp đầu ra + ARCH. Ví dụ như BOOTX64.EFI
                # Khai báo khóa để ký tệp chương trình
                # Khóa riêng tư sẽ có dạng : <tên>.key
                # Khóa công khai sẽ có dạng: <tên>.crt
                "vnexos_db"           # Tên của khóa DB để ký tệp EFI
                "${VNExos_GRUB_CERT}" # Tên của khóa Dilithium để ký tệp EFI
                "vnexos_grub_main"    # Tên của hàm đầu vào (hàm chính)
                "${GRUB_SOURCES}"     # Danh sách các tệp mã nguồn
                FALSE                 # Có dùng ký tự in thường không? FALSE - Dùng ký tự in hoa
            )
        endif()
    endif()
endforeach()