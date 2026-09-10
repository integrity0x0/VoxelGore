#include "TextureManager.h"

namespace gfx {

TextureManager::TextureManager(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                               vkcore::MemoryAllocator& memoryAllocator)
    : textureCache_(device, transferCtxt, memoryAllocator) {}

void TextureManager::AddResolver(std::string prefix, Resolver resolver) {
  resolvers_.emplace_back(std::move(prefix), std::move(resolver));
}

const vkcore::SampledTexture* TextureManager::Require(std::string_view key) {
  if (const auto* cached = textureCache_.Find(key)) return cached;

  for (const auto& entry : resolvers_) {
    if (!key.starts_with(entry.prefix)) continue;

    if (auto texture = entry.resolver(key.substr(entry.prefix.size()))) {
      std::ignore = textureCache_.Emplace(std::string(key), std::move(*texture));
      return textureCache_.Find(key);
    }
    return nullptr;
  }

  return textureCache_.Require(key);
}

}  // namespace gfx