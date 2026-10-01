#include "UiElement.h"

namespace gfx {
UiElement::UiElement(std::string_view id, const UiElement* parent) 
	: id_(id), parent_(parent) {}

void UiElement::Render(UiRenderer& renderer) {
  renderer.Submit({pos_.x.value, pos_.y.value}, {size_.x.value, size_.y.value}, image_, color_);
}


}  // namespace gfx