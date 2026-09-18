#pragma once

#include <vector>

#include "../../util/containers.h"
#include "../voxel/BlockManager.h"
#include "ChannelRegistry.h"

namespace gm {

struct BlockLightData {
  ChannelId id = kInvalidChannelId;
  uint8_t strength = 0;
  bool passingLight = false;
};

class BlockLightCache {
 public:
  using Index = uint32_t;

  static constexpr Index kInvalidIndex = std::numeric_limits<Index>::max();

  BlockLightCache(const BlockManager& blockManager, ChannelRegistry& registry)
      : blockManager_(&blockManager), registry_(&registry) {}

  void Resize(uint32_t blockCount) { sparse_.resize(blockCount, kInvalidIndex); }

  void Set(uint32_t blockId, BlockLightData data) {
    EnsureSparse(blockId);

    const Index index = sparse_[blockId];

    if (index == kInvalidIndex) {
      sparse_[blockId] = static_cast<Index>(dense_.size());
      dense_.push_back(std::move(data));
      denseBlockIds_.push_back(blockId);
      return;
    }

    dense_[index] = std::move(data);
  }

  void Remove(uint32_t blockId) {
    if (!Has(blockId)) return;

    const Index index = sparse_[blockId];
    const Index lastIndex = static_cast<Index>(dense_.size() - 1);

    if (index != lastIndex) {
      dense_[index] = std::move(dense_[lastIndex]);

      const uint32_t movedBlockId = denseBlockIds_[lastIndex];
      denseBlockIds_[index] = movedBlockId;
      sparse_[movedBlockId] = index;
    }

    util::SwapAndPop(dense_, index);
    util::SwapAndPop(denseBlockIds_, index);

    sparse_[blockId] = kInvalidIndex;
  }

  [[nodiscard]] const BlockLightData* Require(uint32_t blockId);

  [[nodiscard]] BlockLightData* Get(uint32_t blockId) {
    if (!Has(blockId)) return nullptr;
    return &dense_[sparse_[blockId]];
  }

  [[nodiscard]] const BlockLightData* Get(uint32_t blockId) const {
    if (!Has(blockId)) return nullptr;
    return &dense_[sparse_[blockId]];
  }

  [[nodiscard]] bool Has(uint32_t blockId) const {
    return blockId < sparse_.size() && sparse_[blockId] != kInvalidIndex;
  }

  [[nodiscard]] size_t Size() const { return dense_.size(); }

  [[nodiscard]] bool Empty() const { return dense_.empty(); }

  [[nodiscard]] const std::vector<BlockLightData>& Dense() const { return dense_; }

  [[nodiscard]] const std::vector<uint32_t>& DenseBlockIds() const { return denseBlockIds_; }

 private:
  void EnsureSparse(uint32_t blockId) {
    if (blockId >= sparse_.size()) {
      sparse_.resize(static_cast<size_t>(blockId + 1), kInvalidIndex);
    }
  }

 private:
  const BlockManager* blockManager_;
  ChannelRegistry* registry_;

  std::vector<BlockLightData> dense_;
  std::vector<uint32_t> denseBlockIds_;
  std::vector<uint32_t> sparse_;
};

}  // namespace gm