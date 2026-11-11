#pragma once

#include "UiContainer.h"

namespace gfx {
class UiGrid : public UiContainer {
 public:
  UiGrid(std::string_view id, const UiElement* parent) : UiContainer(id, parent) {}

  [[nodiscard]] UiElementType GetNodeType() const override {
    return UiElementType::Grid;
  }

  void ApplyStyle(const UiStyle& style) override;

  [[nodiscard]] const glm::vec2& GetCell() const { return cell_; }
  void SetCell(const glm::vec2& cell) {
    cell_ = cell;
    ArrangeChildren();
  }

  [[nodiscard]] const glm::vec2& GetGap() const { return gap_; }
  void SetGap(const glm::vec2& gap) {
    gap_ = gap;
    ArrangeChildren();
  }

  [[nodiscard]] const glm::vec2& GetInset() const { return inset_; }
  void SetInset(const glm::vec2& value) {
    inset_ = value;
    ArrangeChildren();
  }

  [[nodiscard]] size_t GetColumns() const { return columns_; }
  void SetColumns(size_t columns) {
    columns_ = columns;
    ArrangeChildren();
  }

  [[nodiscard]] size_t GetRows() const { return (children_.size() + columns_ - 1) / columns_; }

  UiElement& AddChild(std::unique_ptr<UiElement>&& element) {
    auto& child = UiContainer::AddChild(std::move(element));
    ArrangeChildren();
    return child;
  }

  [[nodiscard]] UiElement* GetCell(glm::ivec2 pos) const {
    if (pos.x < 0 || pos.y < 0 || static_cast<size_t>(pos.x) >= columns_) return nullptr;

    const size_t index = static_cast<size_t>(pos.y) * columns_ + static_cast<size_t>(pos.x);
    return index < children_.size() ? children_[index].get() : nullptr;
  }

 private:
  void ArrangeChildren();

  [[nodiscard]] glm::vec2 ResolveSize(const UiRect& parent) const override;

  glm::vec2 cell_ = glm::ivec2(0);
  glm::vec2 gap_ = glm::ivec2(0);
  glm::vec2 inset_ = glm::ivec2(0);
  size_t columns_ = 0;
};
}  // namespace gfx