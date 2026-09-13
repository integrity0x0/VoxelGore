#pragma once

#include <stdint.h>

#include <cassert>
#include <limits>
#include <vector>

#include "Entity.h"
#include "IComponentStorage.h"
#include "../../util/containers.h"

namespace gm {
template <typename T>
class ComponentStorage final : public IComponentStorage {
 public:
  void Add(EntityId entityId, T value) {
    if (Contains(entityId)) return;

    ExtendSparse(entityId);

    const size_t index = denseComponents_.size();

    denseEntities_.emplace_back(entityId);
    denseComponents_.emplace_back(std::move(value));
    sparse_[entityId] = index;
  }

  void Remove(EntityId entityId) override {
    if (!Contains(entityId)) return;

    const size_t index = sparse_[entityId];
    const size_t lastIndex = denseComponents_.size() - 1;

    if (index != lastIndex) {
      const EntityId movedEntity = denseEntities_.back();

      util::SwapAndPop(denseEntities_, index);
      util::SwapAndPop(denseComponents_, index);

      sparse_[movedEntity] = index;
    } else {
      util::SwapAndPop(denseEntities_, index);
      util::SwapAndPop(denseComponents_, index);
    }

    sparse_[entityId] = kInvalidIndex;
  }

  bool Contains(EntityId entityId) const override {
    return static_cast<size_t>(entityId) < sparse_.size() && sparse_[entityId] != kInvalidIndex;
  }

  bool IsEmpty() const { return denseComponents_.empty(); }

  T* Get(EntityId entityId) {
    if (!Contains(entityId)) return nullptr;

    return &denseComponents_[sparse_[entityId]];
  }

  const T* Get(EntityId entityId) const {
    if (!Contains(entityId)) return nullptr;

    return &denseComponents_[sparse_[entityId]];
  }

  size_t size() const { return denseComponents_.size(); }

  std::vector<T>& denseComponents() { return denseComponents_; }
  const std::vector<EntityId> denseEntities() const { return denseEntities_; };
  ~ComponentStorage() override = default;

 private:

  void ExtendSparse(EntityId entityId) {
    sparse_.resize(static_cast<size_t>(entityId) + 1, kInvalidIndex);
  }

 private:
  static constexpr size_t kInvalidIndex = std::numeric_limits<size_t>::max();

  std::vector<T> denseComponents_;
  std::vector<EntityId> denseEntities_;
  std::vector<size_t> sparse_;
};
}  // namespace gm