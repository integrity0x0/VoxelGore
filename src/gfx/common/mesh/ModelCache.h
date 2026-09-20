#pragma once

#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../texture/MaterialManager.h"
#include "Model.h"

namespace gfx {

class ModelCache {
 public:
  ModelCache(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
             vkcore::BufferAllocator& bufferAllocator,
             TextureManager& textureManager,
             uint32_t framesCount);

  [[nodiscard]] ModelId Require(std::string_view path);

  [[nodiscard]] Model* Get(ModelId id);
  [[nodiscard]] const Model* Get(ModelId id) const;

  [[nodiscard]] Model* Get(std::string_view key) {
    auto it = pathToId_.find(key);
    if (it == pathToId_.end()) {
      return nullptr;
    }

    return Get(it->second);
  }

  [[nodiscard]] const Model* Get(std::string_view key) const {
    auto it = pathToId_.find(key);
    if (it == pathToId_.end()) {
      return nullptr;
    }

    return Get(it->second);
  }

  [[nodiscard]] const vkcore::DescriptorSetLayout& materialSetLayout() const {
    return materialCache_.descriptorSetLayout();
  }

 private:
  const vkcore::Device* device_;
  vkcore::TransferContext* transferCtxt_;
  vkcore::BufferAllocator* bufferAllocator_;
  uint32_t framesCount_;

  std::vector<Model> models_;
  std::vector<std::string> paths_;

  std::unordered_map<std::string, ModelId, util::StringHash, std::equal_to<>> pathToId_;

  MaterialManager materialCache_;
};

}  // namespace gfx