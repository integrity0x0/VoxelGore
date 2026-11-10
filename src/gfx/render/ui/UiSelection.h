#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <span>
#include <vector>

#include "UiElement.h"
#include "UiTypes.h"

namespace gfx {

struct UiSelectionLimits {
  size_t min = 0;
  size_t max = std::numeric_limits<size_t>::max();
};

class UiSelection {
 public:
  explicit UiSelection(UiSelectionLimits limits) : limits_(limits) {
    assert(limits_.min <= limits_.max);
  }

  UiSelection() = default;

  [[nodiscard]] std::span<UiElement* const> GetSelected() const { return selected_; }

  void Select(UiElement* item);
  void Deselect(UiElement* item);

  void Clear();

  void OnItemClicked(UiElement* item);

  void OnChildAdded(UiElement& child) {
    if (!child.IsCheckable()) return;
    if (child.IsChecked())
      OnItemClicked(&child);
    else if (selected_.size() < limits_.min)
      Select(&child);
  }

  void TrimToMax();

  void FillToMin(std::span<const std::unique_ptr<UiElement>> candidates);

  void ApplyLimits(std::span<const std::unique_ptr<UiElement>> candidates) {
    TrimToMax();
    FillToMin(candidates);
  }

  [[nodiscard]] size_t GetMin() const { return limits_.min; }
  void SetMin(size_t min) {
    assert(min <= limits_.max);
    limits_.min = min;
  }

  [[nodiscard]] size_t GetMax() const { return limits_.max; }
  void SetMax(size_t max) {
    assert(max >= limits_.min);
    limits_.max = max;
    TrimToMax();
  }

 private:
  [[nodiscard]] bool IsSelected(UiElement* item) const {
    return std::find(selected_.begin(), selected_.end(), item) != selected_.end();
  }

 private:
  UiSelectionLimits limits_;
  std::vector<UiElement*> selected_;
};

}  // namespace gfx