/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file main.cpp
 * @brief Tệp khởi đầu của Bộ nạp mồi
 */
#include "efilib.hpp"
#include <cpu.hpp>

using namespace EFI;

extern "C" [[gnu::ms_abi]] EFI_STATUS
vnexos_grub_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  EFI_BOOT_SERVICES* bs = SystemTable->BootServices;

  init(ImageHandle, SystemTable);
  clear();

  printf("Nhan phim bat ky de doc tep...\n");
  waitForKey();

  void*      buffer;
  uint64_t   size;
  EFI_STATUS status = loadFile(EFI_TEXT("\\EFI\\BOOT\\vnexos.efi"), &buffer, &size);

  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\vnexos.efi");
    return 1;
  }

  printf("Hello world! Tep o vi tri: 0x%p", buffer);

  while (true)
  {
    cpu_halt();
  }
  return EFI_SUCCESS;
}
