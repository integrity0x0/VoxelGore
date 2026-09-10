#pragma once

#include <stdint.h>

#include <limits>

namespace gm {

using EntityId = uint32_t;

static constexpr EntityId kInvalidEntityId = std::numeric_limits<EntityId>::max();
static constexpr uint32_t kInvalidEntityGeneration = std::numeric_limits<uint32_t>::max();

struct Entity {
  EntityId id = kInvalidEntityId;
  uint32_t generation = kInvalidEntityGeneration;

  Entity() = default;

  Entity(EntityId id, uint32_t generation) : id(id), generation(generation) {}
};
}  // namespace gm