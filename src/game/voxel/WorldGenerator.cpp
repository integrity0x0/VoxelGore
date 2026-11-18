#include "WorldGenerator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace gm {
namespace {

uint16_t RequireId(BlockManager& blocks, std::string_view name) {
  const auto id = blocks.FindId(name);
  if (!id)
    throw std::runtime_error(std::string("WorldGenerator: missing block \"") + name.data() + "\"");
  return static_cast<uint16_t>(*id);
}

// value noise 0..1
float ValueNoise2D(int x, int z) {
  uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(z) * 668265263u;
  h = (h ^ (h >> 13)) * 1274126177u;
  return static_cast<float>(h & 0xFFFFu) / 65535.0f;
}

float SmoothNoise2D(float x, float z) {
  const int x0 = static_cast<int>(std::floor(x));
  const int z0 = static_cast<int>(std::floor(z));
  const float fx = x - static_cast<float>(x0);
  const float fz = z - static_cast<float>(z0);
  const float sx = fx * fx * (3.0f - 2.0f * fx);
  const float sz = fz * fz * (3.0f - 2.0f * fz);

  const float n00 = ValueNoise2D(x0, z0);
  const float n10 = ValueNoise2D(x0 + 1, z0);
  const float n01 = ValueNoise2D(x0, z0 + 1);
  const float n11 = ValueNoise2D(x0 + 1, z0 + 1);

  const float nx0 = n00 + (n10 - n00) * sx;
  const float nx1 = n01 + (n11 - n01) * sx;
  return nx0 + (nx1 - nx0) * sz;
}

float Fbm2D(float x, float z, int octaves) {
  float value = 0.0f;
  float amp = 1.0f;
  float freq = 1.0f;
  float norm = 0.0f;
  for (int i = 0; i < octaves; ++i) {
    value += SmoothNoise2D(x * freq, z * freq) * amp;
    norm += amp;
    amp *= 0.5f;
    freq *= 2.0f;
  }
  return value / norm;
}

}  // namespace

WorldGenerator::WorldGenerator(BlockManager& blocks)
    : ids_{
          .air = RequireId(blocks, "air"),
          .grass = RequireId(blocks, "grass"),
          .dirt = RequireId(blocks, "dirt"),
          .stone = RequireId(blocks, "stone"),
          .sand = RequireId(blocks, "sand"),
          .ice = RequireId(blocks, "ice"),
          .glass = RequireId(blocks, "glass"),
          .tnt = RequireId(blocks, "tnt"),
          .luckyblock = RequireId(blocks, "luckyblock"),
          .seaLantern = RequireId(blocks, "sea_lantern"),
          .redLamp = RequireId(blocks, "red_lamp"),
      } {}

float WorldGenerator::HeightNoise(int worldX, int worldZ) {
  const float continental = Fbm2D(worldX * 0.01f, worldZ * 0.01f, 4);
  const float hills = Fbm2D(worldX * 0.04f + 20.0f, worldZ * 0.04f + 20.0f, 3);
  return continental * 0.7f + hills * 0.3f;
}

float WorldGenerator::DetailNoise(int worldX, int worldZ) {
  return Fbm2D(worldX * 0.12f + 50.0f, worldZ * 0.12f + 50.0f, 2);
}

uint32_t WorldGenerator::Hash(int x, int y, int z) {
  uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u +
               static_cast<uint32_t>(z) * 2147483647u;
  h = (h ^ (h >> 13)) * 1274126177u;
  return h;
}

void WorldGenerator::GenerateChunk(Chunk& chunk) const {
  const glm::ivec3 origin = chunk.pos() * static_cast<int>(Chunk::kLength);

  for (int lz = 0; lz < static_cast<int>(Chunk::kLength); ++lz) {
    for (int lx = 0; lx < static_cast<int>(Chunk::kLength); ++lx) {
      const int wx = origin.x + lx;
      const int wz = origin.z + lz;

      const float h01 = HeightNoise(wx, wz);
      const float detail = DetailNoise(wx, wz);

      int surfaceY = seaLevel_ + static_cast<int>(h01 * 18.0f + detail * 3.0f);
      surfaceY = std::clamp(surfaceY, 4, 40);

      const bool beach = surfaceY <= seaLevel_ + 1;
      const bool cold = HeightNoise(wx + 1000, wz - 500) > 0.72f;

      for (int ly = 0; ly < static_cast<int>(Chunk::kLength); ++ly) {
        const int wy = origin.y + ly;
        Voxel v{ids_.air};

        if (wy > surfaceY) {
          if (wy == surfaceY + 1 && (Hash(wx, wy, wz) % 2000u) == 0u) {
            v.id = ids_.luckyblock;
          }
        } else if (wy == surfaceY) {
          if (beach)
            v.id = ids_.sand;
          else if (cold)
            v.id = ids_.ice;
          else
            v.id = ids_.grass;
        } else if (wy >= surfaceY - 3) {
          v.id = beach ? ids_.sand : ids_.dirt;
        } else {
          v.id = ids_.stone;

          const uint32_t h = Hash(wx, wy, wz);
          if (h % 900u == 0u)
            v.id = ids_.seaLantern;
          else if (h % 1100u == 1u)
            v.id = ids_.redLamp;
          else if (h % 1500u == 2u)
            v.id = ids_.tnt;
          else if (h % 1800u == 3u)
            v.id = ids_.glass;
        }

        if (v.id == ids_.air && wy <= seaLevel_ && wy > surfaceY) {
        }

        chunk.SetVoxel({lx, ly, lz}, v);
      }
    }
  }
}

}  // namespace gm