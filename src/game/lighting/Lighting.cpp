#include "Lighting.h"

#include "../voxel/Chunk.h"

namespace gm::lighting {
LightChannel& Lighting::RequireChannel(ChannelId id) {
  if (channels_.size() <= id) {
    channels_.resize(static_cast<size_t>(id + 1));
  }

  if (!channels_[id]) {
    const auto* definition = registry_.GetById(id);

    channels_[id] = std::make_unique<LightChannel>(*definition, storage_, id, blockCache_,
                                                   *chunkManager_, *blockManager_);
  }

  return *channels_[id];
}

void Lighting::LightUp() {
  constexpr int L = static_cast<int>(Chunk::kLength);
  constexpr uint8_t kSunLight = 15;

  const int worldWidth = chunkManager_->width() * L;
  const int worldHeight = chunkManager_->height() * L;
  const int worldDepth = chunkManager_->depth() * L;

  for (int x = 0; x < worldWidth; ++x) {
    for (int z = 0; z < worldDepth; ++z) {
      bool sunBlocked = false;

      for (int y = worldHeight - 1; y >= 0; --y) {
        const glm::ivec3 pos{x, y, z};

        const auto voxel = chunkManager_->getVoxel(pos);
        if (!voxel.has_value()) {
          break;
        }
        const auto* block = blockManager_->block(voxel->id);
        const bool isPassingLight = block && block->isPassingLight();

        if (const auto* light = blockCache_.Require(voxel->id);
            light && light->id != kInvalidChannelId && light->strength > 0) {
          RequireChannel(light->id).Spread(pos, light->strength);
        }

        if (!sunBlocked && isPassingLight) {
          sun_.Spread(pos, kSunLight);
        }

        if (!isPassingLight) {
          sunBlocked = true;
        }
      }
    }
  }

  sun_.Update();

  for (auto& channel : channels_) {
    if (channel) {
      channel->Update();
    }
  }
}

void Lighting::OnVoxelSetted(const glm::ivec3& pos, const Voxel& voxel) {
  const Block* block = blockManager_->block(voxel.id);
  const bool isPassingLight = block && block->isPassingLight();

  for (auto& channel : channels_) {
    if (!channel) continue;
    channel->Remove(pos);
  }

  for (auto& channel : channels_) {
    if (channel) channel->Update();
  }

  constexpr uint8_t kSunLight = 15;

  if (!isPassingLight) {
    sun_.Remove(pos);
    for (int32_t y = pos.y - 1; y >= 0; --y) {
      const glm::ivec3 currentPos{pos.x, y, pos.z};
      const auto currentVoxel = chunkManager_->getVoxel(currentPos);
      if (!currentVoxel.has_value()) break;

      const Block* currentBlock = blockManager_->block(currentVoxel->id);
      if (!currentBlock || !currentBlock->isPassingLight()) break;

      sun_.Remove(currentPos);
    }
  } else {
    const glm::ivec3 abovePos = pos + glm::ivec3(0, 1, 0);
    if (sun_.GetLight(abovePos) == kSunLight) {
      for (int32_t y = pos.y; y >= 0; --y) {
        const glm::ivec3 currentPos{pos.x, y, pos.z};
        const auto currentVoxel = chunkManager_->getVoxel(currentPos);
        if (!currentVoxel.has_value()) break;

        const Block* currentBlock = blockManager_->block(currentVoxel->id);
        if (!currentBlock || !currentBlock->isPassingLight()) break;

        sun_.Spread(currentPos, kSunLight);
      }
    }
  }

  sun_.Update();

  static const glm::ivec3 kOffsets[6] = {
      {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
  };

  for (const auto& off : kOffsets) {
    const glm::ivec3 neighborPos = pos + off;

    for (auto& channel : channels_) {
      if (channel) {
        channel->Spread(neighborPos);
      }
    }
    sun_.Spread(neighborPos);
  }

  if (const auto* light = blockCache_.Require(voxel.id);
      light && light->id != kInvalidChannelId && light->strength > 0) {
    auto& channel = RequireChannel(light->id);
    channel.Spread(pos, light->strength);
  }

  for (auto& channel : channels_) {
    if (channel) channel->Update();
  }
  sun_.Update();
}

glm::vec4 Lighting::GetColor(const glm::ivec3& pos) const {
  glm::ivec3 localPos;
  const auto* chunk = storage_.GetChunkData(pos, localPos);
  if (chunk == nullptr) {
    return glm::vec4(0.0f);
  }

  const auto& lights = chunk->channels;

  float sun = lights[kSunId] ? lights[kSunId]->Get(localPos) / 15.0f : 0.0f;
  glm::vec3 color(0.0f);

  for (ChannelId id = 0; id < channels_.size(); ++id) {
    if (id >= lights.size() || !lights[id]) {
      continue;
    }

    const float strength = static_cast<float>(lights[id]->Get(localPos)) / 15.0f;

    const auto& channel = channels_[id];
    if (!channel) {
      continue;
    }

    const glm::vec3 channelColor = channel->definition().color * strength;

    color.r = std::max(color.r, channelColor.r);
    color.g = std::max(color.g, channelColor.g);
    color.b = std::max(color.b, channelColor.b);
  }

  return glm::vec4(color, sun);
}

}  // namespace gm