#include "SurfaceRegistry.h"

#include "AnimationParser.h"

namespace gfx::block {

namespace {
void updateAllBuffers(SurfaceId id, const UvRegion& region, std::vector<UvBuffer>& buffers) {
  for (size_t i = 0; i < buffers.size(); ++i) {
    UniformUv* ptr = buffers[i].mapped() + id;
    ptr->uvRect = glm::vec4(region.min, region.max);
    ptr->arrayLayer = region.arrayLayer;
  }
}

}  // namespace

SurfaceId SurfaceRegistry::registerSurface(const UvRegion& uvRegion,
                                           std::vector<UvBuffer>& buffers) {
  const SurfaceId id = static_cast<SurfaceId>(surfaces_.size());
  surfaces_.emplace_back(uvRegion);
  updateAllBuffers(id, uvRegion, buffers);
  return id;
}

SurfaceId SurfaceRegistry::registerAnimatedSurface(const Animation& animation,
                                                   std::vector<UvBuffer>& buffers) {
  const SurfaceId id = static_cast<SurfaceId>(surfaces_.size());
  animatedIds_.push_back(id);
  surfaces_.emplace_back(animation);
  updateAllBuffers(id, animation.getCurrentRegion(), buffers);
  return id;
}

const UvRegion& SurfaceRegistry::extractRegion(SurfaceId id) const {
  assert(id < surfaces_.size() && "SurfaceId out of range");

  const Surface& surface = surfaces_[id];

  if (const UvRegion* region = std::get_if<UvRegion>(&surface)) {
    return *region;
  }
  return std::get<Animation>(surface).getCurrentRegion();
}

SurfaceId SurfaceRegistry::resolve(const std::string& path, std::vector<UvBuffer>& buffers) {
  if (path.empty()) return kInvalidSurface;

  if (auto it = cache_.find(path); it != cache_.end()) {
    return it->second;
  }

  SurfaceId id = load(path, buffers);

  cache_.emplace(path, id);

  return id;
}

SurfaceId SurfaceRegistry::load(const std::string& path, std::vector<UvBuffer>& buffers) {
  if (path.ends_with(".json")) {
    Animation animation = AnimationParser::parse(path, *atlas_);

    return registerAnimatedSurface(std::move(animation), buffers);
  }

  if (atlas_->load(path)) {
    auto* region = atlas_->get(path);
    return registerSurface(region->toUv(), buffers);
  }

  return kInvalidSurface;
}

void SurfaceRegistry::updateAnimations(float dt, UvBuffer& uvBuffer) {
  UniformUv* mapped = uvBuffer.mapped();

  for (SurfaceId id : animatedIds_) {
    if (Animation* anim = std::get_if<Animation>(&surfaces_[id])) {
      const auto& uvRegion = anim->Update(dt);
      mapped[id] = UniformUv{.uvRect = glm::vec4(uvRegion.min, uvRegion.max),
                             .arrayLayer = uvRegion.arrayLayer};
    }
  }
}

}  // namespace gfx::block