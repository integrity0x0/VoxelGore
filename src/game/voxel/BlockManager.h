// BlockManager.h
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../../util/hashers.h"
#include "Block.h"
#include "BlockParser.h"

namespace gm {

class BlockManager {
 public:
  BlockManager() = default;

  void Load(std::string_view path);

  void LoadDirectory(std::string_view dir);

  [[nodiscard]] const Block* GetBlock(uint32_t id) const {
    if (id >= blocks_.size()) return nullptr;
    return blocks_[id].get();
  }

  [[nodiscard]] const Block* GetBlock(std::string_view name) const {
    const auto it = ids_.find(name);
    if (it == ids_.end()) return nullptr;
    return GetBlock(it->second);
  }

  [[nodiscard]] std::optional<uint32_t> FindId(std::string_view name) const {
    const auto it = ids_.find(name);
    if (it == ids_.end()) return std::nullopt;
    return it->second;
  }

  [[nodiscard]] size_t GetBlockCount() const { return blocks_.size(); }

 private:
  std::vector<std::unique_ptr<Block>> blocks_;
  std::unordered_map<std::string, uint32_t, util::StringHash, std::equal_to<>> ids_;
};

}  // namespace gm