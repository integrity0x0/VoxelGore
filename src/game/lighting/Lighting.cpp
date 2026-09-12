#include "Lighting.h"

#include "../voxel/Chunk.h"

namespace gm {

void Lighting::LightUp() {
  constexpr int L = static_cast<int>(Chunk::kLength);

  const int worldWidth = chunkManager->width() * L;
  const int worldHeight = chunkManager->height() * L;
  const int worldDepth = chunkManager->depth() * L;

  for (int x = 0; x < worldWidth; ++x) {
    for (int z = 0; z < worldDepth; ++z) {
      bool sunBlocked = false;

      for (int y = worldHeight - 1; y >= 0; --y) {
        glm::ivec3 pos{x, y, z};

        auto voxel = chunkManager->getVoxel(pos);
        if (!voxel.has_value()) break;

        auto block = blockManager->block(voxel->id);
        bool isPassingLight = block && block->isPassingLight();

        glm::ivec3 emission = (block) ? block->getEmission() : glm::ivec3(0);
        r.Spread(pos, emission.r);
        g.Spread(pos, emission.g);
        b.Spread(pos, emission.b);

        if (!sunBlocked && isPassingLight) {
          chunkManager->setLight(pos, LightChannel::S, 15);
          s.Spread(pos, 15);
        }

        if (!isPassingLight) {
          sunBlocked = true;
        }
      }

      s.Update();
      r.Update();
      g.Update();
      b.Update();
    }
  }
}

void Lighting::OnVoxelSetted(const glm::ivec3& pos, const Voxel& voxel) {
  static constexpr uint32_t kSunLight = 15;

  static const glm::ivec3 kOffsets[6] = {
      {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
  };

  auto block = blockManager->block(voxel.id);
  bool isPassingLight = block && block->isPassingLight();
  glm::ivec3 emission = block ? block->getEmission() : glm::ivec3(0);

  if (isPassingLight) {
    if (chunkManager->getLight(pos + glm::ivec3(0, 1, 0), LightChannel::S) == kSunLight) {
      for (int32_t i = pos.y; i >= 0; --i) {
        glm::ivec3 p = {pos.x, i, pos.z};

        auto v = chunkManager->getVoxel(p);
        if (!v.has_value()) break;

        auto b = blockManager->block(v->id);
        if (!b || !b->isPassingLight()) break;

        s.Spread(p, kSunLight);
      }
    }
  } else {
    s.Remove(pos);
    for (int32_t i = pos.y - 1; i >= 0; --i) {
      glm::ivec3 p = {pos.x, i, pos.z};

      auto v = chunkManager->getVoxel(p);
      if (!v.has_value()) break;

      auto b = blockManager->block(v->id);
      if (!b || !b->isPassingLight()) break;

      s.Remove(p);
    }
    s.Update();
  }

  r.Remove(pos);
  g.Remove(pos);
  b.Remove(pos);

  r.Update();
  g.Update();
  b.Update();

  for (const auto& off : kOffsets) {
    glm::ivec3 neighborPos = pos + off;

    r.Spread(neighborPos);
    g.Spread(neighborPos);
    b.Spread(neighborPos);
    s.Spread(neighborPos);
  }

  if (emission.r) r.Spread(pos, emission.r);
  if (emission.g) g.Spread(pos, emission.g);
  if (emission.b) b.Spread(pos, emission.b);

  r.Update();
  g.Update();
  b.Update();
  s.Update();
}
}  // namespace gm