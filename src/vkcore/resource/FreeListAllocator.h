#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcore {

class FreeListAllocator {
 public:
  struct Region {
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;

    Region() = default;
    Region(VkDeviceSize offset, VkDeviceSize size);
  };

  explicit FreeListAllocator(VkDeviceSize size);

  [[nodiscard]] std::optional<Region> reserve(VkDeviceSize size, VkDeviceSize alignment = 0);

  [[nodiscard]] std::optional<Region> reserveFull();

  void free(const Region& region);

  [[nodiscard]] const std::vector<Region>& freeRegions() const { return freeRegions_; }

  [[nodiscard]] VkDeviceSize size() const { return size_; }

 private:
  void merge();

  VkDeviceSize size_;
  std::vector<Region> freeRegions_;
};

template <typename T>
constexpr T alignUp(T value, T alignment) {
  if (alignment == 0) {
    return value;
  }

  return (value + alignment - 1) & ~(alignment - 1);
}

}  // namespace vkcore