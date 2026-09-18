#include "BlockSurfaceRegistry.h"

#include "BlockAnimationParser.h"

namespace gfx {

namespace {
void UpdateAllBuffers(BlockSurfaceId id, const UvRegion& region, std::vector<BlockUvBuffer>& buffers) {
  for (size_t i = 0; i < buffers.size(); ++i) {
    UniformUv* ptr = buffers[i].mapped() + id;
    ptr->uvRect = glm::vec4(region.min, region.max);
    ptr->arrayLayer = region.arrayLayer;
  }
}

}  // namespace

BlockSurfaceId BlockSurfaceRegistry::RegisterSurface(const UvRegion& uvRegion, 
                                                     std::vector<BlockUvBuffer>& buffers) {
  const BlockSurfaceId id = static_cast<BlockSurfaceId>(surfaces_.size());
  surfaces_.emplace_back(uvRegion);
  UpdateAllBuffers(id, uvRegion, buffers);
  return id;
}

BlockSurfaceId BlockSurfaceRegistry::RegisterAnimatedSurface(const BlockAnimation& animation,
                                                             std::vector<BlockUvBuffer>& buffers) {
  const BlockSurfaceId id = static_cast<BlockSurfaceId>(surfaces_.size());
  animatedIds_.push_back(id);
  surfaces_.emplace_back(animation);
  UpdateAllBuffers(id, animation.getCurrentRegion(), buffers);
  return id;
}

const UvRegion& BlockSurfaceRegistry::ExtractRegion(BlockSurfaceId id) const {
  assert(id < surfaces_.size() && "SurfaceId out of range");

  const BlockSurface& surface = surfaces_[id];

  if (const UvRegion* region = std::get_if<UvRegion>(&surface)) {
    return *region;
  }
  return std::get<BlockAnimation>(surface).getCurrentRegion();
}

BlockSurfaceId BlockSurfaceRegistry::Resolve(const std::string& path, std::vector<BlockUvBuffer>& buffers) {
  if (path.empty()) return kInvalidSurface;

  if (auto it = cache_.find(path); it != cache_.end()) {
    return it->second;
  }

  BlockSurfaceId id = Load(path, buffers);

  cache_.emplace(path, id);

  return id;
}

BlockSurfaceId BlockSurfaceRegistry::Load(const std::string& path, std::vector<BlockUvBuffer>& buffers) {
  if (path.ends_with(".json")) {
    BlockAnimation animation = AnimationParser::Parse(path, *atlas_);

    return RegisterAnimatedSurface(std::move(animation), buffers);
  }

  if (atlas_->Load(path)) {
    auto* region = atlas_->Require(path);
    return RegisterSurface(region->toUv(), buffers);
  }

  return kInvalidSurface;
}

void BlockSurfaceRegistry::UpdateAnimations(float dt, BlockUvBuffer& uvBuffer) {
  UniformUv* mapped = uvBuffer.mapped();

  for (BlockSurfaceId id : animatedIds_) {
    if (BlockAnimation* anim = std::get_if<BlockAnimation>(&surfaces_[id])) {
      const auto& uvRegion = anim->Update(dt);
      mapped[id] = UniformUv{.uvRect = glm::vec4(uvRegion.min, uvRegion.max),
                             .arrayLayer = uvRegion.arrayLayer};
    }
  }
}

}  // namespace gfx