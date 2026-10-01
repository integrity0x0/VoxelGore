#pragma once

#include <unordered_map>
#include <tinyxml2.h>

#include "../../../util/hashers.h"
#include "UiElement.h"
#include "UiContainer.h"
#include "UiButton.h"
#include "UiText.h"
#include "UiStateOverrides.h"
#include "UiStateTable.h"
#include "UiPage.h"

namespace gfx {
class UiElementParser {
 public:
  UiElementParser() = delete;
  [[nodiscard]] std::vector<std::unique_ptr<UiElement>> Parse(const tinyxml2::XMLElement* element);
 private:
  void ParseCommonAttributes(UiElement& uiElement, const tinyxml2::XMLElement* xmlElement); 
  [[nodiscard]] std::unique_ptr<UiElement> ParseContainer(const tinyxml2::XMLElement* element);
  [[nodiscard]] std::unique_ptr<UiButton> ParseButton(const tinyxml2::XMLElement* element);
  [[nodiscard]] std::unique_ptr<UiText> ParseText(const tinyxml2::XMLElement* element);
  //[[nodiscard]] std::unique_ptr<UiCheckbox> ParseContainer(const tinyxml2::XMLElement* element);
 private:
  static const std::unordered_map<std::string, std::function<UiElement(tinyxml2::XMLElement*)>>
      kParsers;
};
}  // namespace gfx