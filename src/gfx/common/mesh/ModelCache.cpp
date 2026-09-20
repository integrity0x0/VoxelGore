#include "ModelCache.h"

#include "ModelLoader.h"

namespace gfx {

ModelCache::ModelCache(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                       vkcore::BufferAllocator& bufferAllocator,
                       TextureManager& textureManager, uint32_t framesCount)
    : device_(&device),
      transferCtxt_(&transferCtxt),
      bufferAllocator_(&bufferAllocator),
      framesCount_(framesCount),
      materialCache_(device, textureManager) {}

ModelId ModelCache::Require(std::string_view path) {
  auto it = pathToId_.find(path);

  if (it != pathToId_.end()) return it->second;

  auto model = ModelLoader::Load(static_cast<ModelId>(models_.size()), *device_, *transferCtxt_,
                                 *bufferAllocator_, materialCache_, path);

  if (!model) return kInvalidModelId;

  const ModelId id = static_cast<ModelId>(models_.size());

  models_.emplace_back(std::move(*model));
  paths_.emplace_back(path);
  pathToId_.emplace(paths_.back(), id);

  return id;
}

Model* ModelCache::Get(ModelId id) {
  if (id >= models_.size()) return nullptr;
  return &models_[id];
}

const Model* ModelCache::Get(ModelId id) const {
  if (id >= models_.size()) return nullptr;

  return &models_[id];
}

}  // namespace gfx