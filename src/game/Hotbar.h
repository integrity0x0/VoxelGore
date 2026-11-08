#pragma once

#include <array>
#include <stdint.h>

namespace gm {
class Hotbar {
 public:
  static constexpr size_t kSize = 8;

  [[nodiscard]] uint32_t GetBlock(size_t slot) const { return slot < kSize ? slots_[slot] : 0; }
  void SetBlock(size_t slot, uint32_t blockId) {
    if (slot < kSize) slots_[slot] = blockId;
  }

  [[nodiscard]] size_t GetSelected() const { return selected_; }
  void Select(size_t slot) {
    if (slot < kSize) selected_ = slot;
  }
  [[nodiscard]] uint32_t GetSelectedBlock() const { return slots_[selected_]; }

 private:
  std::array<uint32_t, kSize> slots_{1, 2, 3, 4, 5, 6, 7, 8};
  size_t selected_ = 0;
};
}  // namespace gm