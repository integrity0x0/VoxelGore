#pragma once

#include <cassert>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "UiElement.h"
#include "UiSelection.h"

namespace gfx {

class UiContainer : public UiElement {
 public:
  UiContainer(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}
  ~UiContainer() override {}

  [[nodiscard]] UiElementType GetNodeType() const override { return UiElementType::Container; }

  [[nodiscard]] std::span<const std::unique_ptr<UiElement>> GetChildren() const override {
    return children_;
  }

  virtual UiElement& AddChild(std::unique_ptr<UiElement>&& child) {
    assert(child);
    children_.emplace_back(std::move(child));
    selection_.OnChildAdded(*children_.back());
    return *children_.back();
  }

  [[nodiscard]] const UiSelection& GetSelection() const { return selection_; }

  void SetSelection(UiSelection selection) {
    selection_ = std::move(selection);
    selection_.ApplyLimits(children_);
  }

  [[nodiscard]] size_t GetSelectionMin() const { return selection_.GetMin(); }
  [[nodiscard]] size_t GetSelectionMax() const { return selection_.GetMax(); }

  void SetSelectionMin(size_t min) {
    selection_.SetMin(min);
    selection_.FillToMin(children_);
  }

  void SetSelectionMax(size_t max) {
    selection_.SetMax(max);
  }

  void Render(UiRenderer& renderer) override;
  void Relayout(const UiRect& rect) override;

  void OnChildCheckedChanged(UiElement& child) override { selection_.OnItemClicked(&child); }

 protected:
  std::vector<std::unique_ptr<UiElement>> children_;
  UiSelection selection_;
};

}  // namespace gfx