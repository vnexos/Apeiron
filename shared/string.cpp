/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file string.cpp
 * @brief Hiện thực các hàm thao tác bộ nhớ chuẩn C cho
 *        môi trường độc lập (UEFI / bare-metal).
 */
#include "string.hpp"

extern "C" {

void* memcpy(void* dest, const void* src, uint64_t n)
{
  uint8_t*       d = static_cast<uint8_t*>(dest);
  const uint8_t* s = static_cast<const uint8_t*>(src);

  /* Khắc phục lỗi lệch lề: Sao chép từng byte cho tới khi `d` chia hết cho 8 */
  while (n > 0 && (reinterpret_cast<uintptr_t>(d) & 7) != 0)
  {
    *d++ = *s++;
    n--;
  }

  /* Duỗi vòng lặp cấp độ cao */
  uint64_t*       d64 = reinterpret_cast<uint64_t*>(d);
  const uint64_t* s64 = reinterpret_cast<const uint64_t*>(s);

  while (n >= 64)
  {
    d64[0]  = s64[0];
    d64[1]  = s64[1];
    d64[2]  = s64[2];
    d64[3]  = s64[3];
    d64[4]  = s64[4];
    d64[5]  = s64[5];
    d64[6]  = s64[6];
    d64[7]  = s64[7];
    d64    += 8;
    s64    += 8;
    n      -= 64;
  }

  /* Xử lý phần lẻ */
  while (n >= 8)
  {
    *d64++  = *s64++;
    n      -= 8;
  }

  /* Xử lý phần lẻ dưới 8 byte */
  d = reinterpret_cast<uint8_t*>(d64);
  s = reinterpret_cast<const uint8_t*>(s64);
  while (n > 0)
  {
    *d++ = *s++;
    n--;
  }

  return dest;
}

void* memset(void* dest, int c, uint64_t n)
{
  uint8_t* d   = static_cast<uint8_t*>(dest);
  uint8_t  val = static_cast<uint8_t>(c);

  /* Sao chép từng byte cho tới khi `d` chia hết cho 8 */
  while (n > 0 && (reinterpret_cast<uintptr_t>(d) & 7) != 0)
  {
    *d++ = val;
    n--;
  }

  /* Tô màu theo khối 8 byte */
  uint64_t val64  = val;
  val64          |= val64 << 8;
  val64          |= val64 << 16;
  val64          |= val64 << 32;

  uint64_t* d64 = reinterpret_cast<uint64_t*>(d);
  while (n >= 8)
  {
    *d64++  = val64;
    n      -= 8;
  }

  /* Xử lý phần lẻ */
  d = reinterpret_cast<uint8_t*>(d64);
  while (n > 0)
  {
    *d++ = val;
    n--;
  }

  return dest;
}

void* memmove(void* dest, const void* src, uint64_t n)
{
  uint8_t*       d = static_cast<uint8_t*>(dest);
  const uint8_t* s = static_cast<const uint8_t*>(src);

  if (d < s || d >= s + n)
  {
    /* Không trùng lấp hoặc đích nằm trước nguồn — sao chép tiến */
    return memcpy(dest, src, n);
  }

  /* Đích nằm sau nguồn và có trùng lấp — sao chép lùi */
  d += n;
  s += n;
  while (n > 0)
  {
    *(--d) = *(--s);
    n--;
  }

  return dest;
}

int memcmp(const void* s1, const void* s2, uint64_t n)
{
  const uint8_t* a = static_cast<const uint8_t*>(s1);
  const uint8_t* b = static_cast<const uint8_t*>(s2);

  while (n > 0)
  {
    if (*a != *b)
      return (*a < *b) ? -1 : 1;
    a++;
    b++;
    n--;
  }

  return 0;
}

} // extern "C"
