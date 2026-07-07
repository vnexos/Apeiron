/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép GPLv3.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * @file sign.cpp
 * @brief Triển khai các hàm của thuật toán ký Dilithium.
 */
#include "sign.hpp"
#include <efilib.hpp>
#include <post_quantum/crypto/sha3.hpp>
#include <post_quantum/sig/dilithium.hpp>
#include <string.hpp>

// Chữ ký của tệp mặc định
struct Signature
{
  uint8_t parentKeyID[32];
  uint8_t parentKeyHash[32];
  uint8_t sign[];
} __attribute__((packed));

static bool getKeyData(Sign::KeyMetadata* metadata, const uint8_t* rawPublicKey, uint64_t keySize)
{
  if (keySize < DILITHIUM_PUBLICKEYBYTES)
    return false;

  Crypto::VNExos::sha256(metadata->currentKey, rawPublicKey, DILITHIUM_PUBLICKEYBYTES);
  Crypto::VNExos::sha256(metadata->currentCertHash, rawPublicKey, keySize);

  return true;
}

bool Sign::verifyFileData(const uint8_t* rawData, uint64_t dataSize, const uint8_t* rawPublicKey, uint64_t keySize)
{
  // Nếu tệp nhỏ hơn chữ ký + siêu dữ liệu
  if (dataSize < DILITHIUM_BYTES + 64)
    return false;

  // Lấy các khóa để xác thực
  KeyMetadata metadata;
  if (!getKeyData(&metadata, rawPublicKey, keySize))
    return false;

  // Chữ ký ở cuối tệp mặc định
  uint64_t   sigPos    = dataSize - (DILITHIUM_BYTES + 64);
  Signature* signature = (Signature*)(rawData + sigPos);

  // 2 bước để xác thực chữ ký với khóa công khai
  if (memcmp(signature->parentKeyID, metadata.currentKey, sizeof(metadata.currentKey)) != 0)
    return false;

  if (memcmp(signature->parentKeyHash, metadata.currentCertHash, sizeof(metadata.currentCertHash)) != 0)
    return false;

  // Xác minh chữ ký
  uint8_t fileHash[32];
  Crypto::VNExos::sha256(fileHash, rawData, dataSize - DILITHIUM_BYTES);

  if (!Dilithium::verify(signature->sign, DILITHIUM_BYTES, fileHash, sizeof(fileHash), rawPublicKey))
    return false;

  return true;
}
