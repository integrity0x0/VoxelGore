#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "TextureCache.h"

namespace gfx {

class TextureManager {
 public:
  using Resolver = std::function<std::optional<vkcore::SampledTexture>(std::string_view remainder)>;

  TextureManager(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                 vkcore::MemoryAllocator& memoryAllocator);

  void AddResolver(std::string prefix, Resolver resolver);

  [[nodiscard]] const vkcore::SampledTexture* Find(std::string_view key) {
    return textureCache_.Find(key);
  }

  [[nodiscard]] const vkcore::SampledTexture* Load(std::string_view path, std::string_view key) {
    return textureCache_.Load(path, key);
  }

  [[nodiscard]] const vkcore::SampledTexture* Load(std::string_view path) {
    return textureCache_.Load(path);
  }

  [[nodiscard]] const vkcore::SampledTexture* Require(std::string_view key);

  [[nodiscard]] const TextureCache& textureCache() const { return textureCache_; }

 private:
  struct PrefixResolver {
    std::string prefix;
    Resolver resolver;

    PrefixResolver(std::string_view prefix, Resolver resolver)
        : prefix(prefix), resolver(resolver) {}

    PrefixResolver(std::string&& prefix, Resolver resolver)
        : prefix(std::move(prefix)), resolver(resolver) {}
  };

  std::vector<PrefixResolver> resolvers_;
  TextureCache textureCache_;
};

}  // namespace gfx