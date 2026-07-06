/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép GPLv3.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file sign.hpp
 * @brief Khai báo các hàm của thuật toán ký Dilithium.
 */
#ifndef __SIGN_HPP
#define __SIGN_HPP

#include <stdint.h>

namespace Sign {
typedef struct
{
  uint8_t currentKey[32];
  uint8_t currentCertHash[32];
} KeyMetadata;

/**
 * Xác thực tệp ngay trên bộ nhớ
 * @param rawData      Dữ liệu thô của tệp đọc được
 * @param dataSize     Kích thước của dữ liệu
 * @param rawPublicKey Dữ liệu thô của khóa công khai
 * @param keySize      Kích thước của khóa công khai
 */
bool verifyFileData(
    const uint8_t* rawData,
    uint64_t       dataSize,
    const uint8_t* rawPublicKey,
    uint64_t       keySize);
} // namespace Sign

#endif // __SIGN_HPP