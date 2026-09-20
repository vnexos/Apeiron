#ifndef __SHARED__BOOTINFO_HPP
#define __SHARED__BOOTINFO_HPP

#if !defined(__EFI_ALLOWED)
#define __EFI_ALLOWED
#else
#define __EFI_ALLOWED_DEFINED
#endif
#include <efi_memory.hpp>
#if !defined(__EFI_ALLOWED_DEFINED)
#undef __EFI_ALLOWED
#else
#undef __EFI_ALLOWED_DEFINED
#endif

typedef struct
{
  // Thông tin ánh xạ bộ nhớ
  EFI_MEMORY_DESCRIPTOR* map;
  uint64_t               mapSize;
  uint64_t               decsSize;
  // Thông tin địa chỉ
  uint64_t virtKeyAddress;
  uint64_t pagesRegion;
  uint64_t pagesRegionPages;
  uint64_t pagesRegionPtr;
  uint64_t maxRegion;
  uint64_t maxRegionPages;
  uint64_t maxRegionPtr;
} BootInfo;

#endif // __SHARED__BOOTINFO_HPP
