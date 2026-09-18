#include "LightChunkStorage.h"

namespace gm {
namespace {
constexpr int32_t kShift = 4;
static_assert(LightChunk::kLength == 16);
constexpr int32_t kMask = static_cast<int32_t>(LightChunk::kLength) - 1;
}  // namespace

void LightChunkStorage::SplitWorldPos(const glm::ivec3& worldPos, glm::ivec3& outChunkPos,
                                      glm::ivec3& outLocalPos) {
  outChunkPos = glm::ivec3(worldPos.x >> kShift, worldPos.y >> kShift, worldPos.z >> kShift);

  outLocalPos = glm::ivec3(worldPos.x & kMask, worldPos.y & kMask, worldPos.z & kMask);
}

uint8_t LightChunkStorage::GetLight(const glm::ivec3& worldPos, ChannelId id) const {
  glm::ivec3 chunkPos, localPos;
  SplitWorldPos(worldPos, chunkPos, localPos);

  const LightChunk* chunk = GetChunk(chunkPos, id);
  if (chunk == nullptr) {
    return 0;
  }

  return chunk->Get(localPos);
}

void LightChunkStorage::SetLight(const glm::ivec3& worldPos, ChannelId id, uint8_t strength) {
  glm::ivec3 chunkPos, localPos;
  SplitWorldPos(worldPos, chunkPos, localPos);

  LightChunk& chunk = GetOrCreateChunk(chunkPos, id);
  chunk.Set(localPos, strength);
}

}  // namespace gm