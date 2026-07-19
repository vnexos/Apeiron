# =========================================================
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
#
# Hàm biên dịch cho Vi xử lý: RISC-V 64
# =========================================================

function(VNExosBuildEfi_riscv64 
    FILE_NAME DB_CERT DIL_CERT 
    SRC_FILES
    IS_LOWERCASE
    LINKER_FILE)
    if(NOT LINKER_FILE)
        message(FATAL_ERROR "[ VNExos ] Lỗi nghiêm trọng: Không tìm thấy tệp liên kết!")
    endif()

    # Tên đích xây dựng (độc nhất)
    set(TARGET_NAME "${FILE_NAME}_riscv64")

    # Thêm nguồn C, C++, ASM vào đích
    add_executable(${TARGET_NAME} ${SRC_FILES})

    # Xác định hậu tố cho tệp đầu ra
    set(EFI_SUFFIX "RISCV64.EFI")

    if(IS_LOWERCASE)
        string(TOLOWER "${EFI_SUFFIX}" EFI_SUFFIX)
    endif()

    # Ép các cờ áp dụng cho riscv64 đối với tệp C, C++, ASM
    target_compile_options(${TARGET_NAME} PRIVATE
        # Cho cả C, C++ và ASM
        --target=riscv64-unknown-none-elf
        -march=rv64imac
        -mabi=lp64
        -mcmodel=medany
        # Cho C, C++
        $<$<COMPILE_LANGUAGE:C,CXX>:
            -ffreestanding
            -O2
            -Wall 
            -Wextra
            -fno-asynchronous-unwind-tables
            -mno-implicit-float
            -mno-relax
            -msmall-data-limit=0
            -fno-jump-tables
            -fshort-wchar
        >
        # Cho riêng C++
        $<$<COMPILE_LANGUAGE:CXX>:
            -fno-exceptions
            -fno-rtti
            -fno-use-cxa-atexit
            -fno-threadsafe-statics
        >
        # Cho riêng ASM
        $<$<COMPILE_LANGUAGE:ASM>:
            -march=armv8-a
        >
    )

    # Dùng riêng ld.lld để liên kết
    target_link_options(${TARGET_NAME} PRIVATE
        --target=riscv64-unknown-none-elf
        -nostdlib
        -fuse-ld=ld.lld
        -Wl,-m,elf64lriscv
        -T ${LINKER_FILE}
    )

    # Đổi tên tệp đầu ra
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "${FILE_NAME}"
        SUFFIX "${EFI_SUFFIX}"
    )

    # Biến đổi tệp chương trình ELF sang EFI
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${ELF2EFI} $<TARGET_FILE:${TARGET_NAME}> $<TARGET_FILE:${TARGET_NAME}>
    )

    # Ký tệp bằng khóa gốc và khóa DB
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND boreas -sign -s
            ${VNExos_CERT_DIR}/${DIL_CERT}.key
            ${VNExos_CERT_DIR}/${DIL_CERT}.crt
            $<TARGET_FILE:${TARGET_NAME}>
            $<TARGET_FILE:${TARGET_NAME}>

        COMMAND sbsign
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