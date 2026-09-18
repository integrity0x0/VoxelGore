#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../../util/hashers.h"
#include "../../../vkcore/resource/SampledTexture.h"
#include "../../../vkcore/resource/TransferContext.h"

namespace gfx {
class TextureCache {
 public:
  TextureCache(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
               vkcore::MemoryAllocator& memoryAllocator);

  [[nodiscard]] const vkcore::SampledTexture* Require(std::string_view key);
  [[nodiscard]] const vkcore::SampledTexture* Find(std::string_view key) const;

  [[nodiscard]] const vkcore::SampledTexture* Load(std::string_view path, std::string_view key);
  [[nodiscard]] const vkcore::SampledTexture* Load(std::string_view path) {
    return Load(path, path);
  }

  [[nodiscard]] const vkcore::SampledTexture* Emplace(std::string key,
                                                      vkcore::SampledTexture&& texture);
  [[nodiscard]] const vkcore::SampledTexture* Emplace(std::string_view key,
                                                      vkcore::SampledTexture&& texture) {
    return Emplace(std::string(key), std::move(texture));
  }

  [[nodiscard]] bool Contains(std::string_view key) const { return textures_.contains(key); }

 private:
  const vkcore::Device* device_;
  vkcore::TransferContext* transferCtxt_;
  vkcore::MemoryAllocator* memoryAllocator_;
  std::unordered_map<std::string, std::unique_ptr<vkcore::SampledTexture>, util::StringHash,
                     std::equal_to<>>
      textures_;
};
}  // namespace gfx
