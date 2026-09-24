#include "ChannelProcessor.h"

#include <array>

namespace gm {

namespace {

constexpr auto kNeighbourOffsets = std::to_array<glm::ivec3>({
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
});

bool PassesLight(const BlockManager* blockManager, uint32_t blockId) {
  const Block* block = blockManager->block(blockId);
  return block != nullptr && block->isPassingLight();
}

}  // namespace

void ChannelProcessor::Spread(const glm::ivec3& pos, uint32_t strength) {
  if (strength == 0) {
    return;
  }

  glm::ivec3 chunkPos;
  glm::ivec3 localPos;

  LightChunkStorage::SplitWorldPos(pos, chunkPos, localPos);

  LightChunk& chunk = storage_->GetOrCreateChunk(chunkPos, id_);

  chunk.Set(localPos, static_cast<uint8_t>(strength));

  spreadQueue_.emplace(LightNode{
      .pos = pos,
      .strength = strength,
      .chunk = &chunk,
  });

  chunkManager_->MarkDirty(pos);
}

void ChannelProcessor::Spread(const glm::ivec3& pos) {
  glm::ivec3 localPos;
  glm::ivec3 chunkPos;

  LightChunkStorage::SplitWorldPos(pos, chunkPos, localPos);

  LightChunk* chunk = storage_->GetChunk(chunkPos, id_);

  if (chunk == nullptr) {
    return;
  }

  Spread(pos, chunk->Get(localPos), chunk);
}

void ChannelProcessor::Remove(const glm::ivec3& pos) {
  glm::ivec3 localPos;
  glm::ivec3 chunkPos;

  LightChunkStorage::SplitWorldPos(pos, chunkPos, localPos);

  LightChunk* chunk = storage_->GetChunk(chunkPos, id_);

  if (chunk == nullptr) {
    return;
  }

  Remove(pos, chunk);
}

void ChannelProcessor::Spread(const glm::ivec3& pos, uint32_t strength, LightChunk* chunk) {
  if (strength == 0) {
    return;
  }

  glm::ivec3 localPos;
  glm::ivec3 chunkPos;

  LightChunkStorage::SplitWorldPos(pos, chunkPos, localPos);

  chunk->Set(localPos, static_cast<uint8_t>(strength));

  spreadQueue_.emplace(LightNode{
      .pos = pos,
      .strength = strength,
      .chunk = chunk,
  });

  chunkManager_->MarkDirty(pos);
}

void ChannelProcessor::Remove(const glm::ivec3& pos, LightChunk* chunk) {
  glm::ivec3 localPos;
  glm::ivec3 chunkPos;

  LightChunkStorage::SplitWorldPos(pos, chunkPos, localPos);

  const uint8_t light = chunk->Get(localPos);

  if (light == 0) {
    return;
  }

  chunk->Set(localPos, 0);

  removeQueue_.emplace(LightNode{
      .pos = pos,
      .strength = light,
      .chunk = chunk,
  });

  chunkManager_->MarkDirty(pos);
}

LightChunk* ChannelProcessor::GetNeighborChunk(LightChunk* chunk, const glm::ivec3& neighborPos,
                                               glm::ivec3& outLocalPos) {
  glm::ivec3 chunkPos;

  LightChunkStorage::SplitWorldPos(neighborPos, chunkPos, outLocalPos);

  if (chunk->pos() == chunkPos) {
    return chunk;
  }

  return storage_->GetChunk(chunkPos, id_);
}


void ChannelProcessor::Update() {
  ProcessRemoveQueue();
  ProcessSpreadQueue();
  spreadQueue_ = {};
  removeQueue_ = {};
}

void ChannelProcessor::ProcessRemoveQueue() {
  while (!removeQueue_.empty()) {
    const LightNode front = removeQueue_.front();
    removeQueue_.pop();

    for (const glm::ivec3& offset : kNeighbourOffsets) {
      const glm::ivec3 neighborPos = front.pos + offset;

      const std::optional<Voxel> voxel = chunkManager_->getVoxel(neighborPos);

      if (!voxel.has_value()) {
        continue;
      }

      glm::ivec3 localPos;

      LightChunk* neighborChunk = GetNeighborChunk(front.chunk, neighborPos, localPos);

      uint8_t neighborLight = 0;

      if (neighborChunk != nullptr) {
        neighborLight = neighborChunk->Get(localPos);
      }

      if (neighborLight != 0 && neighborLight == front.strength - 1) {
        const auto* light = blockCache_->Require(voxel->id);

        if (light && light->id == id_ && light->strength != 0) {
          if (neighborChunk == nullptr) {
            glm::ivec3 chunkPos;

            LightChunkStorage::SplitWorldPos(neighborPos, chunkPos, localPos);

            neighborChunk = &storage_->GetOrCreateChunk(chunkPos, id_);
          }

          Spread(neighborPos, light->strength, neighborChunk);
        } else {
          Remove(neighborPos, neighborChunk);
        }
      } else if (neighborLight >= front.strength) {
        Spread(neighborPos, neighborLight, neighborChunk);
      }
    }
  }
}

void ChannelProcessor::ProcessSpreadQueue() {
  while (!spreadQueue_.empty()) {
    const LightNode front = spreadQueue_.front();
    spreadQueue_.pop();

    if (front.strength <= 1) {
      continue;
    }

    for (const glm::ivec3& offset : kNeighbourOffsets) {
      const glm::ivec3 neighborPos = front.pos + offset;

      const std::optional<Voxel> voxel = chunkManager_->getVoxel(neighborPos);

      if (!voxel.has_value() || !PassesLight(blockManager_, voxel->id)) {
        continue;
      }

      glm::ivec3 localPos;

      LightChunk* neighborChunk = GetNeighborChunk(front.chunk, neighborPos, localPos);

      const uint32_t newStrength = front.strength - 1;

      uint8_t neighborLight = 0;

      if (neighborChunk != nullptr) {
        neighborLight = neighborChunk->Get(localPos);
      }

      if (neighborLight < newStrength) {
        if (neighborChunk == nullptr) {
          glm::ivec3 chunkPos;

          LightChunkStorage::SplitWorldPos(neighborPos, chunkPos, localPos);

          neighborChunk = &storage_->GetOrCreateChunk(chunkPos, id_);
        }

        Spread(neighborPos, newStrength, neighborChunk);
      }
    }
  }
}

}  // namespace gm