/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file main.cpp
 * @brief Tệp khởi đầu của Bộ nạp mồi
 */
#include <cpu.hpp>
#include <efi.hpp>

extern "C" [[gnu::ms_abi]] EFI_STATUS
vnexos_grub_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  (void)ImageHandle;
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, EFI_TEXT("Hello world!"));
  while (true)
  {
    cpu_halt();
  }
  return EFI_SUCCESS;
}
