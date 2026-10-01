#include "UiElementParser.h"

namespace gfx {
const std::unordered_map<std::string, std::function<UiElement(const tinyxml2::XMLElement*)>>
    UiElementParser::kParsers = {
    {"container", ParseContainer}
};
std::vector<std::unique_ptr<UiElement>> UiElementParser::Parse(
    const tinyxml2::XMLElement* element) {
  for (const auto* el = element; el; el = element->NextSiblingElement()) {
    
  }
}
}  // namespace gfx