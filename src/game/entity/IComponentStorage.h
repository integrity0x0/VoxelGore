#pragma once

#include "Entity.h"

namespace gm {
class IComponentStorage {
 public:
  virtual ~IComponentStorage() = default;
  virtual void Remove(EntityId entityId) = 0;
  virtual bool Contains(EntityId entityId) const = 0;
};
}  // namespace gm