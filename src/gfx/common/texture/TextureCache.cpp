#include "TextureCache.h"

#include "TextureLoader.h"

namespace gfx {

TextureCache::TextureCache(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                           vkcore::MemoryAllocator& memoryAllocator)
    : device_(&device), transferCtxt_(&transferCtxt), memoryAllocator_(&memoryAllocator) {}

const vkcore::SampledTexture* TextureCache::Require(std::string_view key) {
  if (const auto* cached = Find(key)) return cached;
  return Load(key, key);
}

const vkcore::SampledTexture* TextureCache::Find(std::string_view key) const {
  auto it = textures_.find(key);
  return it != textures_.end() ? it->second.get() : nullptr;
}

const vkcore::SampledTexture* TextureCache::Load(std::string_view path, std::string_view key) {
  if (const auto* cached = Find(key)) return cached;

  auto texture = TextureLoader::Load(*device_, *transferCtxt_, *memoryAllocator_, path, 1u);
  if (!texture) return nullptr;

  return Emplace(std::string(key), std::move(*texture));
}

const vkcore::SampledTexture* TextureCache::Emplace(std::string key,
                                                    vkcore::SampledTexture&& texture) {
  auto [it, inserted] = textures_.insert_or_assign(
      std::move(key), std::make_unique<vkcore::SampledTexture>(std::move(texture)));
  return it->second.get();
}

}  // namespace gfx