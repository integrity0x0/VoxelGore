#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../../util/hashers.h"
#include "LightChunk.h"
#include "ChannelRegistry.h"

namespace gm::lighting {

class LightChunkStorage {
 public:
  struct ChunkData {
    std::vector<std::unique_ptr<LightChunk>> channels;
  };

  using LightChunkMap = std::unordered_map<glm::ivec3, ChunkData, util::IVec3Hash>;

  [[nodiscard]] LightChunk* GetChunk(const glm::ivec3& chunkPos, ChannelId id) {
    auto it = chunks_.find(chunkPos);
    if (it == chunks_.end() || id >= it->second.channels.size()) {
      return nullptr;
    }

    return it->second.channels[id].get();
  }

  [[nodiscard]] const LightChunk* GetChunk(const glm::ivec3& chunkPos, ChannelId id) const {
    auto it = chunks_.find(chunkPos);
    if (it == chunks_.end() || id >= it->second.channels.size()) {
      return nullptr;
    }

    return it->second.channels[id].get();
  }

  [[nodiscard]] LightChunk& GetOrCreateChunk(const glm::ivec3& chunkPos, ChannelId id) {
    auto& chunk = chunks_[chunkPos];

    if (chunk.channels.size() <= static_cast<size_t>(id)) {
      chunk.channels.resize(static_cast<size_t>(id + 1));
    }

    if (!chunk.channels[id]) {
      chunk.channels[id] = std::make_unique<LightChunk>();
    }

    return *chunk.channels[id];
  }

  [[nodiscard]] uint8_t GetLight(const glm::ivec3& worldPos, ChannelId id) const;

  void SetLight(const glm::ivec3& worldPos, ChannelId id, uint8_t strength);

  [[nodiscard]] const LightChunkStorage::ChunkData* GetChunkData(
      const glm::ivec3& worldPos, glm::ivec3& outLocalPos) const {
    glm::ivec3 chunkPos;
    SplitWorldPos(worldPos, chunkPos, outLocalPos);

    const auto it = chunks_.find(chunkPos);
    if (it == chunks_.end()) {
      return nullptr;
    }

    return &it->second;
  }

  static void SplitWorldPos(const glm::ivec3& worldPos, glm::ivec3& outChunkPos,
                            glm::ivec3& outLocalPos);
 private:
  LightChunkMap chunks_;
};

}  // namespace gm::lighting