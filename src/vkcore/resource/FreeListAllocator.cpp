#include "FreeListAllocator.h"

#include <algorithm>

namespace vkcore {

FreeListAllocator::Region::Region(VkDeviceSize offset, VkDeviceSize size)
    : offset(offset), size(size) {}

FreeListAllocator::FreeListAllocator(VkDeviceSize size) : size_(size) {
  freeRegions_.emplace_back(0, size_);
}

std::optional<FreeListAllocator::Region> FreeListAllocator::reserve(VkDeviceSize size,
                                                                    VkDeviceSize alignment) {
  if (size == 0) {
    return std::nullopt;
  }

  for (size_t i = 0; i < freeRegions_.size(); ++i) {
    Region& freeRegion = freeRegions_[i];

    const VkDeviceSize alignedOffset = alignUp(freeRegion.offset, alignment);

    const VkDeviceSize padding = alignedOffset - freeRegion.offset;

    if (freeRegion.size < padding || freeRegion.size - padding < size) {
      continue;
    }

    const VkDeviceSize remainingSize = freeRegion.size - padding - size;

    const VkDeviceSize allocationOffset = alignedOffset;

    if (padding > 0 && remainingSize > 0) {
      freeRegion.size = padding;

      freeRegions_.insert(freeRegions_.begin() + i + 1,
                          Region{alignedOffset + size, remainingSize});
    } else if (padding > 0) {
      freeRegion.size = padding;
    } else if (remainingSize > 0) {
      freeRegion.offset = alignedOffset + size;
      freeRegion.size = remainingSize;
    } else {
      freeRegions_.erase(freeRegions_.begin() + i);
    }

    return Region{allocationOffset, size};
  }

  return std::nullopt;
}

std::optional<FreeListAllocator::Region> FreeListAllocator::reserveFull() { return reserve(size_); }

void FreeListAllocator::free(const Region& region) {
  if (region.size == 0) {
    return;
  }

  freeRegions_.push_back(region);
  merge();
}

void FreeListAllocator::merge() {
  if (freeRegions_.empty()) {
    return;
  }

  std::sort(freeRegions_.begin(), freeRegions_.end(),
            [](const Region& lhs, const Region& rhs) { return lhs.offset < rhs.offset; });

  std::vector<Region> merged;
  merged.reserve(freeRegions_.size());

  for (const Region& region : freeRegions_) {
    if (region.size == 0) {
      continue;
    }

    if (merged.empty()) {
      merged.push_back(region);
      continue;
    }

    Region& previous = merged.back();

    const VkDeviceSize previousEnd = previous.offset + previous.size;

    if (previousEnd < region.offset) {
      merged.push_back(region);
      continue;
    }

    const VkDeviceSize regionEnd = region.offset + region.size;

    if (regionEnd > previousEnd) {
      previous.size = regionEnd - previous.offset;
    }
  }

  freeRegions_ = std::move(merged);
}

}  // namespace vkcore