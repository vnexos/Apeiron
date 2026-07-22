# =========================================================
# Copyright (c) 2026 VNExos
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Hàm biên dịch cho Vi xử lý: x86_64
# =========================================================

include(${VNExos_CONFIG_DIR}/source_filter.cmake)


add_library(vnexos_flags_x86_64 INTERFACE)
target_compile_options(vnexos_flags_x86_64 INTERFACE
    # Cho cả C, C++ và ASM
    --target=x86_64-unknown-windows
    # Cho C, C++
    $<$<COMPILE_LANGUAGE:C,CXX>:
        -ffreestanding
        -O2
        -Wall 
        -Wextra
        -fno-asynchronous-unwind-tables
        -mno-stack-arg-probe
        -fshort-wchar
        -D__EFI_ALLOWED
    >
    # Cho riêng C++
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-exceptions
        -fno-rtti
        -fno-use-cxa-atexit
        -fno-threadsafe-statics
    >
)
target_link_options(vnexos_flags_x86_64 INTERFACE
    --target=x86_64-unknown-windows
    -nostdlib
    -fuse-ld=lld-link
    -Wl,-subsystem:efi_application
    -Wl,-nodefaultlib
    -Wl,-dll
    -Wl,-dynamicbase
    -Wl,-noimplib
)

function(VNExosBuildEfi_x86_64 
    FILE_NAME DB_CERT DIL_CERT 
    ENTRYPOINT SRC_FILES
    IS_LOWERCASE)
    # Tên đích xây dựng (độc nhất)
    set(TARGET_NAME "${FILE_NAME}_x86_64")
    set(TARGET_NAME ${TARGET_NAME} PARENT_SCOPE)
    set_property(GLOBAL APPEND PROPERTY VNExos_ALL_TARGET ${TARGET_NAME})
    
    # Lọc để lấy các mã Assembly đúng với dòng Vi xử lý
    VNExosFilterAssemblySource("x86_64" SRC_FILES)

    # Thêm nguồn C, C++, ASM vào đích
    add_executable(${TARGET_NAME} ${SRC_FILES})
    target_link_libraries(${TARGET_NAME} PRIVATE vnexos_flags_x86_64 vnexos_shared_x86_64)

    # Xác định hậu tố cho tệp đầu ra
    set(EFI_SUFFIX "X64.EFI")

    if(IS_LOWERCASE)
        string(TOLOWER "${EFI_SUFFIX}" EFI_SUFFIX)
    endif()

    # Ép LLD liên kết điểm mấu chốt
    target_link_options(${TARGET_NAME} PRIVATE
        -Wl,-entry:${ENTRYPOINT}
    )

    # Đổi tên tệp đầu ra
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "${FILE_NAME}"
        SUFFIX "${EFI_SUFFIX}"
    )

    # Ký tệp bằng thuật toán bằng khóa gốc và khóa DB
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${VNExos_BOREAS_TOOL} -sign -s
            ${VNExos_CERT_DIR}/${DIL_CERT}.key
            ${VNExos_CERT_DIR}/${DIL_CERT}.crt
            $<TARGET_FILE:${TARGET_NAME}>
            $<TARGET_FILE:${TARGET_NAME}>

        COMMAND ${VNExos_SBSIGN_TOOL}
            --key ${VNExos_CERT_DIR}/${DB_CERT}.key
            --cert ${VNExos_CERT_DIR}/${DB_CERT}.crt
            --output $<TARGET_FILE:${TARGET_NAME}>
            $<TARGET_FILE:${TARGET_NAME}>

        COMMENT "[ VNExos ] Đã ký tệp chương trình thành công!"
    )

    # Đẩy tệp chương trình vào mục `sysroot` sau khi dựng xong
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${VNExos_SYSROOT_BOOT_DIR}"

        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${VNExos_SYSROOT_BOOT_DIR}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
        
        COMMENT "[ VNExos ] Đã xây dựng xong chương trình: ${VNExos_SYSROOT_BOOT_DIR}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
    )
endfunction()