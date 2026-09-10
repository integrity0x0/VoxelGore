#include "Lighting.h"

#include "../voxel/Chunk.h"

namespace gm {

void Lighting::lightUp() {
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
        r.spread(pos, emission.r);
        g.spread(pos, emission.g);
        b.spread(pos, emission.b);

        if (!sunBlocked && isPassingLight) {
          chunkManager->setLight(pos, LightChannel::S, 15);
          s.spread(pos, 15);
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

void Lighting::onVoxelSetted(const glm::ivec3& pos, const Voxel& voxel) {
  constexpr uint32_t SUN_LIGHT = 15;

  static const glm::ivec3 kOffsets[6] = {
      {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
  };

  auto block = blockManager->block(voxel.id);
  bool isPassingLight = block && block->isPassingLight();
  glm::ivec3 emission = (block) ? block->getEmission() : glm::ivec3(0);

  if (isPassingLight) {
    r.remove(pos);
    g.remove(pos);
    b.remove(pos);
    s.remove(pos);

    r.spread(pos, emission.r);
    g.spread(pos, emission.g);
    b.spread(pos, emission.b);

    if (chunkManager->getLight(pos + glm::ivec3(0, 1, 0), LightChannel::S) == SUN_LIGHT) {
      glm::ivec3 p = pos;
      while (true) {
        auto v = chunkManager->getVoxel(p);
        if (!v.has_value()) break;

        auto b = blockManager->block(v->id);
        if (!b || !b->isPassingLight()) break;

        s.spread(p, SUN_LIGHT);
        --p.y;
      }
    }

    for (const auto& off : kOffsets) {
      glm::ivec3 n = pos + off;
      auto v = chunkManager->getVoxel(n);
      if (v.has_value()) {
        auto neighborBlock = blockManager->block(v->id);
        if (neighborBlock) {
          glm::ivec3 neighborEmission = neighborBlock->getEmission();
          if (neighborEmission.r > 0) r.spread(n, neighborEmission.r);
          if (neighborEmission.g > 0) g.spread(n, neighborEmission.g);
          if (neighborEmission.b > 0) b.spread(n, neighborEmission.b);
        }
      }
    }

    s.Update();
    r.Update();
    g.Update();
    b.Update();

  } else {
    r.remove(pos);
    g.remove(pos);
    b.remove(pos);
    s.remove(pos);

    r.spread(pos, emission.r);
    g.spread(pos, emission.g);
    b.spread(pos, emission.b);

    glm::ivec3 p = pos;
    --p.y;
    while (true) {
      auto v = chunkManager->getVoxel(p);
      if (!v.has_value()) break;

      auto b = blockManager->block(v->id);
      if (!b || !b->isPassingLight()) break;

      s.remove(p);
      --p.y;
    }

    s.Update();
    r.Update();
    g.Update();
    b.Update();
  }
}

}  // namespace gm