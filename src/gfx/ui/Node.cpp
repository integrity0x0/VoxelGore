// Node.cpp
#include "Node.h"

namespace gfx::ui {

namespace {
const std::string kEmptyString = {};
}

// layout
void Node::layout(const glm::vec2& parentAbsPos, const glm::vec2& parentSize) {
  lastParentAbsPos_ = parentAbsPos;
  lastParentSize_ = parentSize;

  // compute own size
  computedSize_.x = (sizeUnit_[0] == Unit::Percent) ? parentSize.x * size_.x * 0.01f : size_.x;

  computedSize_.y = (sizeUnit_[1] == Unit::Percent) ? parentSize.y * size_.y * 0.01f : size_.y;

  // compute raw position (before anchor)
  const float rawX = (posUnit_[0] == Unit::Percent) ? parentSize.x * pos_.x * 0.01f : pos_.x;

  const float rawY = (posUnit_[1] == Unit::Percent) ? parentSize.y * pos_.y * 0.01f : pos_.y;

  // apply anchor factors
  const float ax = anchorFactorX(anchorX_);
  const float ay = anchorFactorY(anchorY_);

  computedPos_.x = rawX + parentSize.x * ax - computedSize_.x * ax;
  computedPos_.y = rawY + parentSize.y * ay - computedSize_.y * ay;

  // absolute position
  absPos_ = parentAbsPos + computedPos_;

  // layout children
  const Rect inner = contentRect();
  for (const auto& child : getChildren()) {
    child->layout(inner.pos, inner.size);
  }
}

void Node::relayout() {
  if (lastParentSize_.x <= 0.0f && lastParentSize_.y <= 0.0f) return;
  layout(lastParentAbsPos_, lastParentSize_);
}

// geometry setters
void Node::setPos(const glm::vec2& pos) {
  pos_ = pos;
  relayout();
}

void Node::setSize(const glm::vec2& size) {
  size_ = size;
  relayout();
}

void Node::setPosX(float v, Unit u) {
  pos_.x = v;
  posUnit_[0] = u;
  relayout();
}

void Node::setPosY(float v, Unit u) {
  pos_.y = v;
  posUnit_[1] = u;
  relayout();
}

void Node::setWidth(float v, Unit u) {
  size_.x = v;
  sizeUnit_[0] = u;
  relayout();
}

void Node::setHeight(float v, Unit u) {
  size_.y = v;
  sizeUnit_[1] = u;
  relayout();
}

void Node::setPosUnitX(Unit u) {
  posUnit_[0] = u;
  relayout();
}

void Node::setPosUnitY(Unit u) {
  posUnit_[1] = u;
  relayout();
}

void Node::setSizeUnitX(Unit u) {
  sizeUnit_[0] = u;
  relayout();
}

void Node::setSizeUnitY(Unit u) {
  sizeUnit_[1] = u;
  relayout();
}

void Node::setPadding(const std::optional<glm::vec4>& padding) {
  padding_ = padding;
  relayout();
}

// texture setters / getters
void Node::setBgImage(std::optional<TextureRegion> region) {
  if (region && region->src.empty()) region.reset();
  bgImage_ = std::move(region);
}

void Node::setImage(std::optional<TextureRegion> region) {
  if (region && region->src.empty()) region.reset();
  image_ = std::move(region);
}

const std::string& Node::getBgImageSrc() const { return bgImage_ ? bgImage_->src : kEmptyString; }

const std::string& Node::getImageSrc() const { return image_ ? image_->src : kEmptyString; }

// render
void Node::renderBg(Renderer& renderer) const {
  if (!padding_.has_value()) return;

  const Insets in = padding();
  const Rect bg =
      applyHeldScale({absPos_ - glm::vec2(in.left, in.top),
                      computedSize_ + glm::vec2(in.left + in.right, in.top + in.bottom)});

  renderer.addQuad(bg.pos, bg.size, bgImage_.value_or(TextureRegion{}), bgColor_);
}

void Node::render(Renderer& renderer) const {
  if (!visible_) return;

  renderBg(renderer);

  const Rect content = contentRect();
  if (content.size.x <= 0.0f || content.size.y <= 0.0f) return;

  const Rect scaled = applyHeldScale(content);

  renderer.addQuad(scaled.pos, scaled.size, image_.value_or(TextureRegion{}), getEffectiveColor());
}

}  // namespace gfx::ui