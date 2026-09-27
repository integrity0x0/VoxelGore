#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "../../UvRegion.h"
#include "../../common/texture/MaterialManager.h"
#include "UiStateOverrides.h"

namespace gfx {

enum class UiNodeType : uint8_t { Undefined, Container, Text, Image, Button, Checkbox, Slider };

class UiElement {
 public:
  using States = std::array<bool, static_cast<size_t>(UiState::Count)>;
  
  UiElement(std::string_view id, const UiElement* parent);

  virtual ~UiElement();

  [[nodiscard]] UiNodeType NodeType() const { return nodeType_; }

  [[nodiscard]] const UiSize& Pos() const { return pos_; }
  [[nodiscard]] const UiSize& Size() const { return size_; }

  void SetPos(const UiSize& position) { pos_ = position; }
  void SetSize(const UiSize& size) { size_ = size; }

  [[nodiscard]] const UiPoint& Anchor() const { return anchor_; }
  [[nodiscard]] const UiPoint& Pivot() const { return pivot_; }

  void SetAnchor(UiPoint anchor) { anchor_ = anchor; }
  void SetPivot(UiPoint pivot) { pivot_ = pivot; }

  [[nodiscard]] bool IsVisible() const { return visible_; }
  void SetVisible(bool visible) { visible_ = visible; }

  [[nodiscard]] bool IsEnabled() const { return enabled_; }
  void SetEnabled(bool enabled) { enabled_ = enabled; }

  [[nodiscard]] bool IsChecked() const { return checked_; }
  void SetChecked(bool checked) { checked_ = checked; }

  [[nodiscard]] UiElement* Parent() const { return parent_; }

  [[nodiscard]] const std::vector<std::unique_ptr<UiElement>>& Children() const {
    return children_;
  }

  [[nodiscard]] UiElement& AddChild(std::unique_ptr<UiElement>&& child) {
    assert(child);
    children_.emplace_back(std::move(child));
  }

  void UpdateState(UiState state, bool value) {
    assert(state < UiState::Count);
    states_[static_cast<size_t>(state)] = value;
  }

 protected:
  void SetNodeType(UiNodeType nodeType) { nodeType_ = nodeType; }
  void SetParent(UiElement* parent) { parent_ = parent; }

 private:
  UiNodeType nodeType_ = UiNodeType::Undefined;
  States states_{};

  std::string id_;

  UiElement* parent_ = nullptr;
  std::vector<std::unique_ptr<UiElement>> children_;

  UiSize pos_;
  UiSize size_;

  std::optional<UiTextureRegion> image_;

  UiPoint anchor_;
  UiPoint pivot_;

  bool visible_ = true;
  bool enabled_ = true;
  bool checked_ = false;

  UiPadding padding_;
  std::optional<UiTextureRegion> bgImage_;
};

}  // namespace gfx