/**
 * Copyright (c) 2026 VNExos
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file string.hpp
 * @brief Khai báo các hàm thao tác với bộ nhớ và chuỗi, thay thế
 *        cho thư viện chuẩn C trong môi trường độc lập.
 */
#ifndef __SHARED__STRING_HPP
#define __SHARED__STRING_HPP

#include <stdint.h>

extern "C" {
void* memcpy(void* dest, const void* src, uint64_t n);
void* memset(void* dest, int c, uint64_t n);
void* memmove(void* dest, const void* src, uint64_t n);
int   memcmp(const void* s1, const void* s2, uint64_t n);
}

#endif // __SHARED__STRING_HPP
