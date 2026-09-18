#include "NodeParser.h"

#include <array>
#include <cctype>
#include <charconv>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gfx::ui {

namespace {

std::vector<std::string> splitTokens(const std::string& raw) {
  std::vector<std::string> tokens;
  std::stringstream ss(raw);
  std::string token;
  while (std::getline(ss, token, ',')) {
    size_t start = token.find_first_not_of(" \t");
    size_t end = token.find_last_not_of(" \t");
    if (start == std::string::npos) {
      throw std::runtime_error("Empty component in attribute value: " + raw);
    }
    tokens.push_back(token.substr(start, end - start + 1));
  }
  if (tokens.empty()) {
    throw std::runtime_error("Attribute value has no components: " + raw);
  }
  return tokens;
}

float parseFloatToken(const std::string& token) {
  try {
    return std::stof(token);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse float from token: " + token + " (" + e.what() + ")");
  }
}

bool parseBool(const std::string& token) {
  if (token == "true" || token == "1") return true;
  if (token == "false" || token == "0") return false;
  throw std::runtime_error("Invalid boolean value (expected true/false or 1/0): " + token);
}

struct ParsedDim {
  float value = 0.0f;
  Unit unit = Unit::Px;
};

ParsedDim parseDimToken(const std::string& token) {
  if (token.empty()) {
    throw std::runtime_error("Empty dimension token");
  }

  ParsedDim out;
  if (token.back() == '%') {
    out.unit = Unit::Percent;
    out.value = parseFloatToken(token.substr(0, token.size() - 1));
  } else {
    out.unit = Unit::Px;
    out.value = parseFloatToken(token);
  }
  return out;
}

struct ParsedVec2Dim {
  glm::vec2 value{0.0f};
  std::array<Unit, 2> units{Unit::Px, Unit::Px};
};

ParsedVec2Dim parseVec2Dim(const std::string& raw) {
  auto tokens = splitTokens(raw);
  ParsedVec2Dim out;

  if (tokens.size() == 1) {
    ParsedDim d = parseDimToken(tokens[0]);
    out.value = glm::vec2(d.value, d.value);
    out.units = {d.unit, d.unit};
    return out;
  }
  if (tokens.size() == 2) {
    ParsedDim dx = parseDimToken(tokens[0]);
    ParsedDim dy = parseDimToken(tokens[1]);
    out.value = glm::vec2(dx.value, dy.value);
    out.units = {dx.unit, dy.unit};
    return out;
  }
  throw std::runtime_error("vec2 attribute expects 1 or 2 components: " + raw);
}

glm::vec2 parseVec2(const std::string& raw) {
  auto tokens = splitTokens(raw);
  if (tokens.size() == 1) {
    float v = parseFloatToken(tokens[0]);
    return glm::vec2(v, v);
  }
  if (tokens.size() == 2) {
    return glm::vec2(parseFloatToken(tokens[0]), parseFloatToken(tokens[1]));
  }
  throw std::runtime_error("vec2 attribute expects 1 or 2 components: " + raw);
}

glm::vec4 parseVec4CSS(const std::string& raw) {
  auto tokens = splitTokens(raw);
  float top, right, bottom, left;
  switch (tokens.size()) {
    case 1:
      top = right = bottom = left = parseFloatToken(tokens[0]);
      break;
    case 2:
      top = bottom = parseFloatToken(tokens[0]);
      left = right = parseFloatToken(tokens[1]);
      break;
    case 3:
      top = parseFloatToken(tokens[0]);
      left = right = parseFloatToken(tokens[1]);
      bottom = parseFloatToken(tokens[2]);
      break;
    case 4:
      top = parseFloatToken(tokens[0]);
      right = parseFloatToken(tokens[1]);
      bottom = parseFloatToken(tokens[2]);
      left = parseFloatToken(tokens[3]);
      break;
    default:
      throw std::runtime_error("padding/margin attribute expects 1 to 4 components: " + raw);
  }
  return glm::vec4(left, top, right, bottom);
}

uint8_t hexNibble(char c) {
  if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
  if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
  if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
  throw std::runtime_error(std::string("Invalid hex digit: ") + c);
}

uint8_t hexByte(const std::string& s, size_t pos) {
  return static_cast<uint8_t>((hexNibble(s[pos]) << 4) | hexNibble(s[pos + 1]));
}

glm::vec4 parseHexColor(const std::string& raw) {
  std::string hex = raw.substr(1);
  for (char c : hex) {
    if (!std::isxdigit(static_cast<unsigned char>(c))) {
      throw std::runtime_error("Invalid hex color string: " + raw);
    }
  }
  uint8_t r, g, b, a = 255;
  switch (hex.size()) {
    case 3:
      r = static_cast<uint8_t>(hexNibble(hex[0]) * 17);
      g = static_cast<uint8_t>(hexNibble(hex[1]) * 17);
      b = static_cast<uint8_t>(hexNibble(hex[2]) * 17);
      break;
    case 4:
      r = static_cast<uint8_t>(hexNibble(hex[0]) * 17);
      g = static_cast<uint8_t>(hexNibble(hex[1]) * 17);
      b = static_cast<uint8_t>(hexNibble(hex[2]) * 17);
      a = static_cast<uint8_t>(hexNibble(hex[3]) * 17);
      break;
    case 6:
      r = hexByte(hex, 0);
      g = hexByte(hex, 2);
      b = hexByte(hex, 4);
      break;
    case 8:
      r = hexByte(hex, 0);
      g = hexByte(hex, 2);
      b = hexByte(hex, 4);
      a = hexByte(hex, 6);
      break;
    default:
      throw std::runtime_error("Hex color must be 3, 4, 6 or 8 digits: " + raw);
  }
  return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

glm::vec4 parseColor(const std::string& raw) {
  if (!raw.empty() && raw[0] == '#') {
    return parseHexColor(raw);
  }
  auto tokens = splitTokens(raw);
  std::array<float, 4> comps = {0.0f, 0.0f, 0.0f, 255.0f};
  if (tokens.size() == 1) {
    float v = parseFloatToken(tokens[0]);
    comps = {v, v, v, 255.0f};
  } else if (tokens.size() == 3 || tokens.size() == 4) {
    for (size_t i = 0; i < tokens.size(); ++i) {
      comps[i] = parseFloatToken(tokens[i]);
    }
  } else {
    throw std::runtime_error("color attribute expects 1, 3 or 4 components, or a hex string: " +
                             raw);
  }
  return glm::vec4(comps[0] / 255.0f, comps[1] / 255.0f, comps[2] / 255.0f, comps[3]);
}

AnchorX parseAnchorX(const std::string& token) {
  if (token == "left") return AnchorX::Left;
  if (token == "center") return AnchorX::Center;
  if (token == "right") return AnchorX::Right;
  throw std::runtime_error("Invalid anchor X value (expected left/center/right): " + token);
}

AnchorY parseAnchorY(const std::string& token) {
  if (token == "top") return AnchorY::Top;
  if (token == "center") return AnchorY::Center;
  if (token == "bottom") return AnchorY::Bottom;
  throw std::runtime_error("Invalid anchor Y value (expected top/center/bottom): " + token);
}

std::pair<AnchorX, AnchorY> parseAnchor(const std::string& raw) {
  auto tokens = splitTokens(raw);

  if (tokens.size() == 1) {
    if (tokens[0] == "center") {
      return {AnchorX::Center, AnchorY::Center};
    }
    throw std::runtime_error(
        "anchor: single directional keyword is ambiguous, "
        "specify both axes (e.g. 'left,top'): " +
        raw);
  }
  if (tokens.size() == 2) {
    return {parseAnchorX(tokens[0]), parseAnchorY(tokens[1])};
  }
  throw std::runtime_error("anchor attribute expects 1 or 2 components: " + raw);
}

std::string resolveAssetPath(const std::string& basePath, const std::string& userPath) {
  std::string resolved;

  if (!userPath.empty() && (userPath[0] == '@' || userPath[0] == '$')) {
    return util::NormalizePath(userPath.substr(1));
  }

  if (!userPath.empty() && userPath[0] == '/') {
    return util::NormalizePath(userPath.substr(1));
  }

  if (basePath.empty()) {
    return util::NormalizePath(userPath);
  }

  const size_t slash = basePath.find_last_of('/');
  std::string baseDir;
  if (slash != std::string::npos) {
    baseDir = basePath.substr(0, slash);
  }

  if (baseDir.empty()) {
    resolved = util::NormalizePath(userPath);
  } else {
    resolved = util::NormalizePath(baseDir + "/" + userPath);
  }

  return resolved;
}

void applyTextureRegion(Node& node, const tinyxml2::XMLElement& xmlElement,
                        const std::string& basePath, const char* srcAttr, const char* posAttr,
                        const char* sizeAttr, void (Node::*setter)(std::optional<TextureRegion>)) {
  const char* src = xmlElement.Attribute(srcAttr);
  if (!src) {
    return;
  }

  TextureRegion region;
  region.src = resolveAssetPath(basePath, src);

  if (const char* v = xmlElement.Attribute(posAttr)) {
    region.pos = parseVec2(v);
  }

  if (const char* v = xmlElement.Attribute(sizeAttr)) {
    region.size = parseVec2(v);
  }

  (node.*setter)(std::optional<TextureRegion>(std::move(region)));
}

void parseCommonAttributes(Node& node, const tinyxml2::XMLElement& xmlElement,
                           const std::string& basePath) {
  if (const char* v = xmlElement.Attribute("pos")) {
    ParsedVec2Dim d = parseVec2Dim(v);
    node.setPosX(d.value.x, d.units[0]);
    node.setPosY(d.value.y, d.units[1]);
  }
  if (const char* v = xmlElement.Attribute("size")) {
    ParsedVec2Dim d = parseVec2Dim(v);
    node.setWidth(d.value.x, d.units[0]);
    node.setHeight(d.value.y, d.units[1]);
  }
  if (const char* v = xmlElement.Attribute("color")) {
    node.setColor(parseColor(v));
  }
  if (const char* v = xmlElement.Attribute("padding")) {
    node.setPadding(parseVec4CSS(v));
  }
  if (const char* v = xmlElement.Attribute("margin")) {
    node.setMargin(parseVec4CSS(v));
  }
  if (const char* v = xmlElement.Attribute("held-color")) {
    node.setHeldColor(std::optional<glm::vec4>(parseColor(v)));
  }
  if (const char* v = xmlElement.Attribute("held-scale")) {
    node.setHeldScale(std::stof(v));
  }
  if (const char* v = xmlElement.Attribute("anchor")) {
    auto [ax, ay] = parseAnchor(v);
    node.setAnchor(ax, ay);
  }
  if (const char* v = xmlElement.Attribute("on-press")) {
    node.setOnPress(v);
  }
  if (const char* v = xmlElement.Attribute("on-held")) {
    node.setOnHeld(v);
  }
  if (const char* v = xmlElement.Attribute("on-release")) {
    node.setOnRelease(v);
  }
  if (const char* v = xmlElement.Attribute("bg-color")) {
    node.setBgColor(glm::vec4(parseColor(v)));
  }
  if (const char* v = xmlElement.Attribute("visible")) {
    node.setVisible(parseBool(v));
  }
  if (const char* v = xmlElement.Attribute("interactive")) {
    node.setInteractive(parseBool(v));
  }

  applyTextureRegion(node, xmlElement, basePath, "image", "image-pos", "image-size",
                     &Node::setImage);
  applyTextureRegion(node, xmlElement, basePath, "bg-image", "bg-image-pos", "bg-image-size",
                     &Node::setBgImage);
}

}  // namespace

const std::unordered_map<std::string, NodeParser::FactoryFn> NodeParser::factories_ = {
    {"container", &NodeParser::parseContainer},
    {"panel", &NodeParser::parsePanel},
};

std::shared_ptr<Node> NodeParser::parse(const tinyxml2::XMLElement& xmlElement,
                                        const Node* /*parent*/, const std::string& basePath) {
  const char* tag = xmlElement.Name();
  if (!tag) {
    throw std::runtime_error("UI node has no tag name");
  }

  auto it = factories_.find(tag);
  if (it == factories_.end()) {
    throw std::runtime_error(std::string("Unknown UI node tag: ") + tag);
  }

  auto node = it->second(xmlElement, nullptr, basePath);

  if (const tinyxml2::XMLElement* firstChild = xmlElement.FirstChildElement()) {
    auto* container = dynamic_cast<Container*>(node.get());
    if (!container) {
      throw std::runtime_error(std::string("Node <") + tag + "> does not support child elements");
    }
    for (const tinyxml2::XMLElement* child = firstChild; child;
         child = child->NextSiblingElement()) {
      auto childNode = parse(*child, node.get(), basePath);
      container->addChild(std::move(childNode));
    }
  }

  return node;
}

std::shared_ptr<Node> NodeParser::parseContainer(const tinyxml2::XMLElement& xmlElement,
                                                 const Node* /*parent*/,
                                                 const std::string& basePath) {
  auto node = std::make_shared<Container>();
  parseCommonAttributes(*node, xmlElement, basePath);
  return node;
}

std::shared_ptr<Node> NodeParser::parsePanel(const tinyxml2::XMLElement& xmlElement,
                                             const Node* /*parent*/, const std::string& basePath) {
  auto node = std::make_shared<Panel>();
  parseCommonAttributes(*node, xmlElement, basePath);
  return node;
}

}  // namespace gfx::ui