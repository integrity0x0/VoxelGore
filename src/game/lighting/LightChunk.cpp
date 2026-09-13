#include "LightChunk.h"

namespace gm::lighting {
size_t LightChunk::GetIndex(const glm::ivec3& localPos) {
  return static_cast<size_t>(localPos.x) * kLength * kLength +
         static_cast<size_t>(localPos.y) * kLength + static_cast<size_t>(localPos.z);
}

uint8_t LightChunk::Get(const glm::ivec3& localPos) const {
  size_t index = GetIndex(localPos);
  uint8_t packedByte = data_[index / 2];
  if (index & 1) {
    return packedByte & 0x0F;
  } else {
    return (packedByte >> 4) & 0x0F;
  }
}

void LightChunk::Set(const glm::ivec3& localPos, uint8_t strength) {
  size_t index = GetIndex(localPos);
  uint8_t& packedByte = data_[index / 2];
  if (index & 1) {
    packedByte = (packedByte & 0xF0) | (strength & 0x0F);
  } else {
    packedByte = (packedByte & 0x0F) | ((strength & 0x0F) << 4);
  }
}

}  // namespace gm::lighting