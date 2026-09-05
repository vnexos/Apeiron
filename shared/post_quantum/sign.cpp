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

struct Crc32Table
{
  uint32_t data[256];

  constexpr const uint32_t& operator[](size_t index) const
  {
    return data[index];
  }
};

constexpr Crc32Table generateCRC32Table()
{
  Crc32Table table{};
  for (uint32_t i = 0; i < 256; ++i)
  {
    uint32_t crc = i;
    for (int j = 0; j < 8; ++j)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else
      {
        crc >>= 1;
      }
    }
    table.data[i] = crc;
  }
  return table;
}

inline constexpr auto CRC32_TABLE = generateCRC32Table();

static uint32_t calcCRC32(uint8_t* bytes, uint64_t n)
{
  uint32_t crc = 0xffffffff;
  for (uint64_t i = 0; i < n; ++i)
  {
    crc = CRC32_TABLE[(crc ^ bytes[i]) & 0xff] ^ (crc >> 8);
  }
  return (crc ^ 0xffffffff) & 0xffffffff;
}

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

USXSecurity* Sign::verifyUsxFileSignature(uint8_t* rawData, uint64_t dataSize, const uint8_t* rawPublicKey, uint64_t keySize)
{
  if (!rawData || dataSize < sizeof(USXHeader))
    return 0;

  USXHeader* header = (USXHeader*)rawData;

  // Kiểm tra mã nhận diện USX
  if (header->MagicBytes[0] != USX_MAGIC_0 ||
      header->MagicBytes[1] != USX_MAGIC_1 ||
      header->MagicBytes[2] != USX_MAGIC_2 ||
      header->MagicBytes[3] != USX_MAGIC_3)
    return 0;

  if (!(header->Flags & USX_HFLAG_SIGNED))
    return 0;

  // Xác minh tiêu đề của tệp USX
  uint32_t crc = calcCRC32(rawData, sizeof(USXHeader) - sizeof(uint32_t));
  if (crc != header->HeaderCRC32)
    return 0;

  // Kiểm tra cờ tệp đã ký
  if (!(header->Flags & USX_HFLAG_SIGNED))
    return 0;

  // Kiểm tra kích thước bảng bảo mật
  if (header->SecurityOffset + sizeof(USXSecurity) > dataSize)
    return 0;

  // Lấy dữ liệu bảng Bảo mật
  USXSecurity* secTable      = (USXSecurity*)(rawData + header->SecurityOffset);
  uint32_t     signatureSize = secTable->SignatureSize;

  if (signatureSize != DILITHIUM_BYTES + 64 ||
      secTable->SignatureOffset + signatureSize > dataSize)
    return 0;

  uint8_t*   signatureBuffer = rawData + secTable->SignatureOffset;
  Signature* signature       = (Signature*)signatureBuffer;

  KeyMetadata metadata;
  if (!getKeyData(&metadata, rawPublicKey, keySize))
    return 0;

  // Kiểm tra khóa ký
  if (memcmp(signature->parentKeyID, metadata.currentKey, sizeof(metadata.currentKey)) != 0)
    return 0;

  if (memcmp(signature->parentKeyHash, metadata.currentCertHash, sizeof(metadata.currentCertHash)) != 0)
    return 0;

  // Chỉ tách riêng 4627 byte chữ ký Dilithium ra bộ đệm tạm và gán 0 vùng chữ ký (giữ nguyên 64 byte metadata)
  uint8_t  savedDilithiumSign[DILITHIUM_BYTES];
  uint8_t* dilithiumBuffer = signatureBuffer + 64;

  memcpy(savedDilithiumSign, dilithiumBuffer, DILITHIUM_BYTES);
  memset(dilithiumBuffer, 0, DILITHIUM_BYTES);

  // Băm tệp sau khi đã zero-out phần chữ ký
  uint8_t fileHash[32];
  Crypto::VNExos::sha256(fileHash, rawData, dataSize);

  // Trả chữ ký về giá trị cũ
  memcpy(dilithiumBuffer, savedDilithiumSign, DILITHIUM_BYTES);

  if (!Dilithium::verify(savedDilithiumSign, DILITHIUM_BYTES, fileHash, sizeof(fileHash), rawPublicKey))
    return 0;

  return secTable;
}
