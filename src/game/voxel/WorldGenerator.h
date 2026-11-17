#pragma once

#include <cstdint>

#include "BlockManager.h"
#include "Chunk.h"

namespace gm {

class WorldGenerator {
 public:
  explicit WorldGenerator(BlockManager& blocks);

  void GenerateChunk(Chunk& chunk) const;

 private:
  struct Ids {
    uint16_t air = 0;
    uint16_t grass = 0;
    uint16_t dirt = 0;
    uint16_t stone = 0;
    uint16_t sand = 0;
    uint16_t ice = 0;
    uint16_t glass = 0;
    uint16_t tnt = 0;
    uint16_t luckyblock = 0;
    uint16_t seaLantern = 0;
    uint16_t redLamp = 0;
  };

  [[nodiscard]] static float HeightNoise(int worldX, int worldZ);
  [[nodiscard]] static float DetailNoise(int worldX, int worldZ);
  [[nodiscard]] static uint32_t Hash(int x, int y, int z);

  Ids ids_;
  int seaLevel_ = 12;
};

}  // namespace gm