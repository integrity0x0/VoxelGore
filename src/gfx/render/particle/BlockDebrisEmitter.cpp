#include "BlockDebrisEmitter.h"

#include <algorithm>
#include <random>

namespace gfx {

namespace {

std::mt19937 gRandomEngine{std::random_device{}()};
std::uniform_real_distribution<float> gDist01(0.0f, 1.0f);

float RandFloat01() { return gDist01(gRandomEngine); }

float RandRange(float lo, float hi) { return lo + RandFloat01() * (hi - lo); }

glm::vec3 RandRange(const glm::vec3& lo, const glm::vec3& hi) {
  return {RandRange(lo.x, hi.x), RandRange(lo.y, hi.y), RandRange(lo.z, hi.z)};
}

gm::Block::Face PickFace(gm::BlockDebrisConfig::Face mode) {
  using Face = gm::Block::Face;

  switch (mode) {
    case gm::BlockDebrisConfig::Face::Top:
      return Face::Top;
    case gm::BlockDebrisConfig::Face::Bottom:
      return Face::Bottom;
    case gm::BlockDebrisConfig::Face::Side: {
      static constexpr Face kSides[4] = {Face::North, Face::South, Face::West, Face::East};
      return kSides[static_cast<int>(RandFloat01() * 4)];
    }
    case gm::BlockDebrisConfig::Face::Random:
    default: {
      static constexpr Face kAll[6] = {Face::North, Face::South, Face::West,
                                       Face::East,  Face::Top,   Face::Bottom};
      return kAll[static_cast<int>(RandFloat01() * 6)];
    }
  }
}

bool IsObstacle(const glm::ivec3& pos, const gm::ChunkManager& chunkManager) {
  return chunkManager.hasVoxel(pos) && chunkManager.getVoxel(pos)->id != 0;
}
void ResolveParticleCollision(Particle& p, float dt, const gm::ChunkManager& chunkManager) {
  static constexpr float kSettleVelocity = 0.05f;
  if (p.settled_) return;

  glm::vec3 delta = p.velocity * dt;
  bool wasFalling = p.velocity.y < 0.0f;
  bool hitGround = false;

  for (int axis = 0; axis < 3; ++axis) {
    glm::vec3 next = p.pos;
    next[axis] += delta[axis];

    if (IsObstacle(glm::ivec3(glm::floor(next)), chunkManager)) {
      p.velocity[axis] *= -p.bounceFactor;

      if (axis == 1 && wasFalling) hitGround = true;
      continue;
    }
    p.pos[axis] = next[axis];
  }

  if (hitGround && std::abs(p.velocity.y) < kSettleVelocity) {
    p.velocity = glm::vec3(0.0f);
    p.settled_ = true;
  }
}

}  // namespace

BlockDebrisEmitter::BlockDebrisEmitter(BlockRenderData& renderData,
                                       const gm::BlockManager& blockManager,
                                       const gm::ChunkManager& chunkManager)
    : renderData_(&renderData), blockManager_(&blockManager), chunkManager_(&chunkManager) {}

void BlockDebrisEmitter::Spawn(uint32_t blockId, const glm::vec3& position) {
  const gm::Block* block = blockManager_->block(blockId);
  if (!block) return;

  const auto& configOpt = block->debrisConfig();
  if (!configOpt.has_value()) return;

  const auto& config = configOpt.value();
  if (config.count == 0) return;

  for (uint32_t i = 0; i < config.count; ++i) {
    gm::Block::Face face = PickFace(config.face);
    const UvRegion& region = renderData_->ExtractRegion(blockId, face);

    float uRange = region.max.x - region.min.x;
    float vRange = region.max.y - region.min.y;

    float sizeX = RandRange(config.size.x, config.size.y);
    float sizeY = RandRange(config.size.x, config.size.y);

    float uSize = std::min(uRange, uRange * (sizeX / kBlockSize));
    float vSize = std::min(vRange, vRange * (sizeY / kBlockSize));

    float uMin = region.min.x + RandFloat01() * (uRange - uSize);
    float vMin = region.min.y + RandFloat01() * (vRange - vSize);
    float uMax = uMin + uSize;
    float vMax = vMin + vSize;

    glm::vec3 velocity = RandRange(config.velocityMin, config.velocityMax);
    float life = RandRange(config.lifetime.x, config.lifetime.y);
    float rotation = RandRange(config.rotationSpeed.x, config.rotationSpeed.y);

    glm::vec3 pos = position + glm::vec3(RandRange(-0.1f, 0.1f), RandRange(-0.1f, 0.1f),
                                         RandRange(-0.1f, 0.1f));

    Particle particle;
    particle.atlasType = Particle::AtlasType::Terrain;
    particle.renderLayer = ToRenderLayer(block->renderLayer());
    particle.pos = pos;
    particle.velocity = velocity;
    particle.acceleration = config.acceleration;
    particle.rotation = rotation;
    particle.angularVelocity = 0.0f;
    particle.size = glm::vec2(sizeX, sizeY);
    particle.uvMinMax = glm::vec4(uMin, vMin, uMax, vMax);
    particle.layer = static_cast<float>(region.arrayLayer);
    particle.color = glm::vec4(1.0f);
    particle.life = life;
    particle.maxLife = life;
    particle.ignoreLighting = false;
    particle.collision = true;
    particles_.push_back(particle);
  }
}

void BlockDebrisEmitter::updateParticles(float dt) {
  for (auto& p : particles_) {
    p.velocity += p.acceleration * dt;

    ResolveParticleCollision(p, dt, *chunkManager_);

    p.life -= dt;
    p.rotation += p.angularVelocity * dt;
  }

  particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                  [](const Particle& p) { return p.life <= 0.0f; }),
                   particles_.end());
}

void BlockDebrisEmitter::Update(float dt) { updateParticles(dt); }

}  // namespace gfx