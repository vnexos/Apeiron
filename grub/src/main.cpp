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
#include <efilib.hpp>
#include <post_quantum/sign.hpp>

using namespace EFI;

extern "C" [[gnu::ms_abi]] EFI_STATUS
vnexos_grub_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  EFI_BOOT_SERVICES* bs = SystemTable->BootServices;

  init(ImageHandle, SystemTable);
  clear();

  // printf("Nhan phim bat ky de doc tep...\n");
  // waitForKey();

  uint8_t*   buffer;
  uint64_t   size;
  EFI_STATUS status = loadFile(EFI_TEXT("\\EFI\\BOOT\\vnexos.efi"), &buffer, &size);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\vnexos.efi");
    return 1;
  }

  uint8_t* key;
  uint64_t keySize;
  status = loadFile(EFI_TEXT("\\EFI\\BOOT\\root.crt"), &key, &keySize);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the doc tep: %s\n", "\\EFI\\BOOT\\root.crt");
    return 1;
  }

  if (Sign::verifyFileData(buffer, size, key, keySize))
    printf("OK\n");
  else
    printf("NG\n");

  bs->FreePool(key);
  bs->FreePool(buffer);

  while (true)
  {
    cpu_halt();
  }
  return EFI_SUCCESS;
}
