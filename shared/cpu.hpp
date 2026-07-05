/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file cpu.hpp
 * @brief Những câu lệnh đơn giản để tương tác với CPU
 */
#ifndef __SHARED__CPU_HPP
#define __SHARED__CPU_HPP

inline void cpu_halt()
{
#if defined(__x86_64__)
  __asm__ __volatile__("hlt");
#elif defined(__aarch64__)
  // Trạng thái nghỉ tiết kiệm điện của ARM, chờ ngắt từ GIC (Generic Interrupt Controller)
  __asm__ __volatile__("wfi");
#elif defined(__riscv)
  // Trong môi trường UEFI (S-mode), lệnh wfi có thể bị OpenSBI từ chối (bắt lỗi Illegal Instruction)
  // để ngăn hệ điều hành làm treo CPU. Do đó, ta chỉ dùng nop trong vòng lặp chờ.
  __asm__ __volatile__("nop");
#else
#error "Kiến trúc chip này chưa được VNExos hỗ trợ!"
#endif
}

#endif // __SHARED__CPU_HPP
