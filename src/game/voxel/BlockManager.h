#pragma once

#include <stdint.h>

#include <deque>

#include "Block.h"
#include "BlockParser.h"

namespace gm {

class BlockManager {
 public:
  BlockManager() = default;

  void Load(std::string_view path) {
    const uint32_t id = static_cast<uint32_t>(blocks_.size());
    blocks_.push_back(BlockParser::Parse(id, path));
  }

  [[nodiscard]] const Block* block(uint32_t id) const {
    if (id >= blocks_.size()) {
      return nullptr;
    }
    return &blocks_[id];
  }

  [[nodiscard]] size_t blockCount() const { return blocks_.size(); }

 private:
  std::deque<Block> blocks_;
};

}  // namespace gm