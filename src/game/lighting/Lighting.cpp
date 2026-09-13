#include "Lighting.h"

#include "../voxel/Chunk.h"

namespace gm::lighting {
LightChannel& Lighting::RequireChannel(ChannelId id) {
  if (channels_.size() <= id) {
    channels_.resize(static_cast<size_t>(id + 1));
  }

  if (!channels_[id]) {
    const auto* definition = registry_.Get(id);
    channels_[id] = std::make_unique<LightChannel>(*definition, *chunkManager_, *blockManager_);
  }

  return *channels_[id];
}

const BlockLightData* Lighting::RequireBlockLight(uint32_t id) {
  if (const auto* light = blockCache_.Get(id)) {
    return light;
  }

  const auto* block = blockManager_->block(id);
  if (!block || !block->light()) {
    blockCache_.Set(id, {
                            .id = kInvalidChannelId,
                            .strength = 0,
                        });

    return blockCache_.Get(id);
  }

  const auto channelId = registry_.Require(block->light()->channelId);

  blockCache_.Set(id, {
                          .id = channelId,
                          .strength = block->light()->strength,
                      });

  return blockCache_.Get(id);
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

        if (const auto* light = RequireBlockLight(voxel->id);
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
  //static constexpr uint32_t kSunLight = 15;

  //static const glm::ivec3 kOffsets[6] = {
  //    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
  //};

  //auto block = blockManager->block(voxel.id);
  //bool isPassingLight = block && block->isPassingLight();
  //glm::ivec3 emission = block ? block->getEmission() : glm::ivec3(0);

  //if (isPassingLight) {
  //  if (chunkManager->getLight(pos + glm::ivec3(0, 1, 0), LightChannel::S) == kSunLight) {
  //    for (int32_t i = pos.y; i >= 0; --i) {
  //      glm::ivec3 p = {pos.x, i, pos.z};

  //      auto v = chunkManager->getVoxel(p);
  //      if (!v.has_value()) break;

  //      auto b = blockManager->block(v->id);
  //      if (!b || !b->isPassingLight()) break;

  //      s.Spread(p, kSunLight);
  //    }
  //  }
  //} else {
  //  s.Remove(pos);
  //  for (int32_t i = pos.y - 1; i >= 0; --i) {
  //    glm::ivec3 p = {pos.x, i, pos.z};

  //    auto v = chunkManager->getVoxel(p);
  //    if (!v.has_value()) break;

  //    auto b = blockManager->block(v->id);
  //    if (!b || !b->isPassingLight()) break;

  //    s.Remove(p);
  //  }
  //  s.Update();
  //}

  //r.Remove(pos);
  //g.Remove(pos);
  //b.Remove(pos);

  //r.Update();
  //g.Update();
  //b.Update();

  //for (const auto& off : kOffsets) {
  //  glm::ivec3 neighborPos = pos + off;

  //  r.Spread(neighborPos);
  //  g.Spread(neighborPos);
  //  b.Spread(neighborPos);
  //  s.Spread(neighborPos);
  //}

  //if (emission.r) r.Spread(pos, emission.r);
  //if (emission.g) g.Spread(pos, emission.g);
  //if (emission.b) b.Spread(pos, emission.b);

  //r.Update();
  //g.Update();
  //b.Update();
  //s.Update();
}
}  // namespace gm