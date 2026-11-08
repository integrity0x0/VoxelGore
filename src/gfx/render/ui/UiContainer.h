#pragma once

#include <memory>

#include "UiElement.h"

namespace gfx {

class UiContainer : public UiElement {
 public:
  UiContainer(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}
  ~UiContainer() override {}

  [[nodiscard]] virtual UiElementType GetNodeType() const override {
    return UiElementType::Container;
  }

  [[nodiscard]] std::span<const std::unique_ptr<UiElement>> GetChildren() const override {
    return children_;
  }

  virtual UiElement& AddChild(std::unique_ptr<UiElement>&& child) {
    assert(child);
    children_.emplace_back(std::move(child));
    return *children_.back();
  }

  [[nodiscard]] UiSelectionMode GetSelectionMode() const { return selectionMode_; }

  void SetSelectionMode(UiSelectionMode selectionMode) {
    selectionMode_ = selectionMode;
  }

  virtual void Render(UiRenderer& renderer) override;

  virtual void Relayout(const UiRect& rect) override;

 protected:
  std::vector<std::unique_ptr<UiElement>> children_;
  UiSelectionMode selectionMode_ = UiSelectionMode::Multiple;
};
}  // namespace gfx
