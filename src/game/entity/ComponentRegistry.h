#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

#include "ComponentStorage.h"

namespace gm {
class ComponentRegistry {
 public:
  template <typename T>
  ComponentStorage<T>& Storage() {
    auto& storage = storages_[std::type_index(typeid(T))];

    if (!storage) storage = std::make_unique<ComponentStorage<T>>();

    return reinterpret_cast<ComponentStorage<T>&>(*storage);
  }

  template <typename T>
  const ComponentStorage<T>& Storage() const {
    const auto it = storages_.find(std::type_index(typeid(T)));

    if (it == storages_.end()) {
      throw std::runtime_error("Component storage does not exist");
    }

    return reinterpret_cast<const ComponentStorage<T>&>(*it->second);
  }

  void DestroyEntity(EntityId id) {
    for (auto& [type, storage] : storages_) storage->Remove(id);
  }

 private:
  std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages_;
};
}  // namespace gm