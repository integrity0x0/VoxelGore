#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../util/hashers.h"
#include "../../vkcore/resource/SampledTexture.h"
#include "../../vkcore/resource/TransferContext.h"
#include "../UvRegion.h"
#include "ImageRegionAllocator.h"
#include "glm/glm.hpp"

namespace gfx {

class Atlas;

class AtlasRegion {
 public:
  [[nodiscard]] const glm::ivec2& size() const { return region_.size; }
  [[nodiscard]] const glm::ivec2& pos() const { return region_.pos; }
  [[nodiscard]] uint32_t arrayLayer() const { return region_.arrayLayer; }

  [[nodiscard]] UvRegion toUv(glm::ivec2 offset, glm::ivec2 size) const {
    glm::ivec2 pixelMin = region_.pos + offset;
    glm::ivec2 pixelMax = pixelMin + size;
    return UvRegion{.min = glm::vec2(pixelMin) / glm::vec2(atlasSize_),
                    .max = glm::vec2(pixelMax) / glm::vec2(atlasSize_),
                    .arrayLayer = region_.arrayLayer};
  }

  [[nodiscard]] UvRegion toUv() const { return toUv({0, 0}, region_.size); }

 private:
  AtlasRegion(const ImageRegion& region, const glm::ivec2& atlasSize)
      : region_(region), atlasSize_(atlasSize) {}
  friend class Atlas;
  ImageRegion region_;
  glm::ivec2 atlasSize_;
};

class Atlas {
 public:
  Atlas(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
        vkcore::MemoryAllocator& allocator, glm::ivec2 size, uint32_t mipLevels,
        uint32_t arrayLayers = 1, uint32_t padding = 1);

  [[nodiscard]] const AtlasRegion* Load(std::string_view path, std::string_view key);
  [[nodiscard]] const AtlasRegion* Load(std::string_view path) { return Load(path, path); }

  [[nodiscard]] const AtlasRegion* Require(std::string_view key);

  [[nodiscard]] const AtlasRegion* Find(std::string_view key) const {
    const auto& it = regions_.find(key);
    return it != regions_.end() ? &it->second : nullptr;
  }

  [[nodiscard]] bool Contains(std::string_view key) const { 
    return regions_.contains(key);
  }

  const vkcore::SampledTexture& texture() const { return texture_; }

  const glm::ivec2& size() const { return size_; }

 private:
  [[nodiscard]] vkcore::SampledTexture CreateTexture(glm::ivec2 size, uint32_t mipLevels, uint32_t arrayLayers);
 private:
  const vkcore::Device* device_;
  vkcore::TransferContext* transferCtxt_;
  vkcore::MemoryAllocator* memoryAllocator_;
  glm::ivec2 size_;
  uint32_t mipLevels_;
  uint32_t padding_;
  vkcore::SampledTexture texture_;
  ImageRegionAllocator regionAllocator_;
  std::unordered_map<std::string, AtlasRegion, util::StringHash, std::equal_to<>> regions_;
};

}  // namespace gfx