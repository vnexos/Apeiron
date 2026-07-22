/**
 * Copyright (c) 2026 VNExos
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

bool Sign::verifyFileSignature(const uint8_t* rawData, uint64_t dataSize, const uint8_t* rawPublicKey, uint64_t keySize)
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

bool Sign::verifyEfiFileSignature(uint8_t* rawData, uint64_t dataSize, const uint8_t* rawPublicKey, uint64_t keySize)
{
  // Kiểm tra định danh của tệp EFI
  if (rawData[0] != 'M' || rawData[1] != 'Z')
    return false;

  // Nếu tệp nhỏ hơn chữ ký + siêu dữ liệu
  if (dataSize < DILITHIUM_BYTES + 64)
    return false;

  // Lấy các khóa để xác thực
  KeyMetadata metadata;
  if (!getKeyData(&metadata, rawPublicKey, keySize))
    return false;

  // Quay về giá trị nguyên mẫu lúc ký
  uint32_t peOffset       = *(uint32_t*)(rawData + 0x3c);
  uint32_t checksumOffset = peOffset + 24 + 64;
  uint32_t secDirOffset   = peOffset + 24 + 144;

  // Lưu giá trị gốc
  uint32_t savedChecksum = *(uint32_t*)(rawData + checksumOffset);
  uint32_t savedSBOffset = *(uint32_t*)(rawData + secDirOffset);
  uint32_t savedSBSize   = *(uint32_t*)(rawData + secDirOffset + 4);

  // Đặt về 0 trước khi băm
  *(uint32_t*)(rawData + checksumOffset) = 0;
  *(uint64_t*)(rawData + secDirOffset)   = 0;

  // Chữ ký ở cuối tệp mặc định
  uint64_t sigOffset = savedSBOffset == 0 ? dataSize : savedSBOffset - (DILITHIUM_BYTES + 64);

  sigOffset = sigOffset & ~0xf; // Căn lề 16 byte

  Signature* signature = (Signature*)(rawData + sigOffset);

  // 2 bước để xác thực chữ ký với khóa công khai
  if (memcmp(signature->parentKeyID, metadata.currentKey, sizeof(metadata.currentKey)) != 0)
  {
    *(uint32_t*)(rawData + checksumOffset)   = savedChecksum;
    *(uint32_t*)(rawData + secDirOffset)     = savedSBOffset;
    *(uint32_t*)(rawData + secDirOffset + 4) = savedSBSize;
    return false;
  }

  if (memcmp(signature->parentKeyHash, metadata.currentCertHash, sizeof(metadata.currentCertHash)) != 0)
  {
    *(uint32_t*)(rawData + checksumOffset)   = savedChecksum;
    *(uint32_t*)(rawData + secDirOffset)     = savedSBOffset;
    *(uint32_t*)(rawData + secDirOffset + 4) = savedSBSize;
    return false;
  }

  // Xác minh chữ ký
  uint8_t fileHash[32];
  Crypto::VNExos::sha256(fileHash, rawData, sigOffset + 64);

  // Khôi phục lại giá trị chữ ký EFI
  *(uint32_t*)(rawData + checksumOffset)   = savedChecksum;
  *(uint32_t*)(rawData + secDirOffset)     = savedSBOffset;
  *(uint32_t*)(rawData + secDirOffset + 4) = savedSBSize;

  if (!Dilithium::verify(signature->sign, DILITHIUM_BYTES, fileHash, sizeof(fileHash), rawPublicKey))
    return false;

  return true;
}
