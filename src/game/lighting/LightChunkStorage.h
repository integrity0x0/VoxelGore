#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../../util/hashers.h"
#include "LightChunk.h"

namespace gm::lighting {

class LightChunkStorage {
 public:
  using LightChunkMap = std::unordered_map<glm::ivec3, LightChunk, util::IVec3Hash>;

  [[nodiscard]] LightChunk* GetChunk(const glm::ivec3& chunkPos) {
    auto it = chunks_.find(chunkPos);
    return it != chunks_.end() ? &it->second : nullptr;
  }

  [[nodiscard]] const LightChunk* GetChunk(const glm::ivec3& chunkPos) const {
    auto it = chunks_.find(chunkPos);
    return it != chunks_.end() ? &it->second : nullptr;
  }

  [[nodiscard]] LightChunk& GetOrCreateChunk(const glm::ivec3& chunkPos) {
    return chunks_[chunkPos];
  }

  [[nodiscard]] uint8_t GetLight(const glm::ivec3& worldPos) const;

  void SetLight(const glm::ivec3& worldPos, uint8_t strength);

 private:
  static void SplitWorldPos(const glm::ivec3& worldPos, glm::ivec3& outChunkPos,
                            glm::ivec3& outLocalPos);
 private:
  LightChunkMap chunks_;
};

}  // namespace gm::lighting