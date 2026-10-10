#include "UiPageParser.h"

#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <filesystem>

#include "../../../util/files.h"
#include "../../../util/pathUtils.h"

namespace gfx {

namespace {

class AttrReader {
 public:
  AttrReader(const tinyxml2::XMLElement& xml_, UiPageParseResult& result)
      : xml_(xml_), result_(result) {}

  template <class Parser>
  auto Get(const char* name, Parser parser) {
    using Result = std::invoke_result_t<Parser, std::string_view>;

    seen_.emplace(name);
    const char* text = xml_.Attribute(name);
    if (!text) return Result();

    Result value = parser(std::string_view(text));
    if (!value) {
      result_.Error(std::string("Invalid value \"") + text + "\" for attribute \"" + name + "\"",
                    xml_.GetLineNum());
    }
    return value;
  }

  template <class Parser, typename T>
  auto GetOr(const char* name, Parser parser, T defaultValue = T()) {
    return Get(name, parser).value_or(std::move(defaultValue));
  }

  void ReportUnknown() const {
    for (auto* a = xml_.FirstAttribute(); a; a = a->Next()) {
      if (seen_.count(std::string(a->Name())) == 0)
        result_.Error(std::string("Unknown attribute \"") + a->Name() + "\"", xml_.GetLineNum());
    }
  }
 private:
  const tinyxml2::XMLElement& xml_;
  UiPageParseResult& result_;
  std::unordered_set<std::string, util::StringHash, std::equal_to<>> seen_;
};

std::string_view Trim(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
    text.remove_prefix(1);
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
    text.remove_suffix(1);
  return text;
}

template <class Enum, size_t N>
std::optional<Enum> ParseEnum(std::string_view text,
                              const std::array<std::pair<std::string_view, Enum>, N>& values) {
  for (const auto& [name, value] : values) {
    if (name == text) return value;
  }

  return std::nullopt;
}

std::optional<float> ParseFloat(std::string_view text) {
  text = Trim(text);

  float value = 0.0f;
  auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

  if (ec != std::errc{} || ptr != text.data() + text.size()) return std::nullopt;

  return value;
}

std::optional<bool> ParseBool(std::string_view text) {
  text = Trim(text);

  if (text == "true" || text == "1") return true;
  if (text == "false" || text == "0") return false;

  return std::nullopt;
}

std::optional<std::string> ParseString(std::string_view text) {
  text = Trim(text);
  if (text.empty()) return std::nullopt;
  return std::string(text);
}

std::optional<UiLength> ParseLength(std::string_view text) {
  text = Trim(text);
  if (text.empty()) return std::nullopt;

  UiUnit unit = UiUnit::Px;

  if (text.ends_with('%')) {
    unit = UiUnit::Percent;
    text.remove_suffix(1);
  } else if (text.ends_with("px")) {
    text.remove_suffix(2);
  }

  auto value = ParseFloat(text);
  if (!value) return std::nullopt;

  return UiLength{*value, unit};
}

std::optional<UiInputMode> ParseInputMode(std::string_view text) {
  static constexpr std::array<std::pair<std::string_view, UiInputMode>, 2> values{{
      {"passthrough", UiInputMode::Passthrough},
      {"capture", UiInputMode::Capture},
  }};

  return ParseEnum(text, values);
}

std::optional<std::pair<std::string_view, std::string_view>> SplitTwo(std::string_view text) {
  const size_t comma = text.find(',');
  if (comma == std::string_view::npos) return std::pair{Trim(text), std::string_view{}};

  if (text.find(',', comma + 1) != std::string_view::npos) return std::nullopt;

  return std::pair{Trim(text.substr(0, comma)), Trim(text.substr(comma + 1))};
}

std::optional<UiLength2> ParseLength2(std::string_view text) {
  auto parts = SplitTwo(text);
  if (!parts || parts->first.empty()) return std::nullopt;

  auto x = ParseLength(parts->first);
  if (!x) return std::nullopt;

  if (parts->second.empty()) return UiLength2{*x, *x};

  auto y = ParseLength(parts->second);
  if (!y) return std::nullopt;

  return UiLength2{*x, *y};
}

std::optional<UiAlignX> ParseAlignX(std::string_view text) {
  static constexpr std::array values {
      std::pair{std::string_view("left"), UiAlignX::Left},
      std::pair{std::string_view("center"), UiAlignX::Center},
      std::pair{std::string_view("right"), UiAlignX::Right},
  };
  return ParseEnum(text, values);
}

std::optional<UiAlignY> ParseAlignY(std::string_view text) {
  static constexpr std::array values {
      std::pair{std::string_view("top"), UiAlignY::Top},
      std::pair{std::string_view("center"), UiAlignY::Center},
      std::pair{std::string_view("bottom"), UiAlignY::Bottom},
  };
  return ParseEnum(text, values);
}

std::optional<UiPoint> ParsePoint(std::string_view text) {
  auto parts = SplitTwo(text);
  if (!parts || parts->first.empty()) return std::nullopt;

  if (parts->second.empty()) {
    auto x = ParseAlignX(parts->first);
    auto y = ParseAlignY(parts->first);
    if (!x || !y) return std::nullopt;
    return UiPoint{*x, *y};
  }

  auto x = ParseAlignX(parts->first);
  auto y = ParseAlignY(parts->second);
  if (!x || !y) return std::nullopt;

  return UiPoint{*x, *y};
}

std::optional<glm::vec4> ParseColor(std::string_view text) {
  text = Trim(text);
  if (text.empty() || text.front() != '#') return std::nullopt;

  text.remove_prefix(1);
  if (text.size() != 6 && text.size() != 8) return std::nullopt;

  auto hex = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  };

  auto byte = [&](size_t pos) -> std::optional<int> {
    int hi = hex(text[pos]);
    int lo = hex(text[pos + 1]);
    if (hi < 0 || lo < 0) return std::nullopt;
    return hi * 16 + lo;
  };

  auto r = byte(0);
  auto g = byte(2);
  auto b = byte(4);
  if (!r || !g || !b) return std::nullopt;

  int a = 255;
  if (text.size() == 8) {
    auto alpha = byte(6);
    if (!alpha) return std::nullopt;
    a = *alpha;
  }

  return glm::vec4(*r, *g, *b, a) / 255.0f;
}

std::optional<UiPadding> ParsePadding(std::string_view text) {
  std::istringstream stream(text.data());
  std::array<UiLength, 4> values;
  size_t count = 0;

  std::string token;
  while (stream >> token) {
    if (count >= values.size()) return std::nullopt;

    auto value = ParseLength(token);
    if (!value) return std::nullopt;

    values[count++] = *value;
  }

  if (count == 0) return std::nullopt;

  switch (count) {
    case 1:
      return UiPadding{
          .left = values[0],
          .top = values[0],
          .right = values[0],
          .bottom = values[0],
      };

    case 2:
      return UiPadding{
          .left = values[1],
          .top = values[0],
          .right = values[1],
          .bottom = values[0],
      };

    case 3:
      return UiPadding{
          .left = values[1],
          .top = values[0],
          .right = values[1],
          .bottom = values[2],
      };

    case 4:
      return UiPadding{
          .left = values[3],
          .top = values[0],
          .right = values[1],
          .bottom = values[2],
      };
  }

  return std::nullopt;
}

struct UiImagePath {
  std::string path;
  std::optional<glm::vec4> rect;  // x, y, width, height
};

std::optional<UiImagePath> ParseImage(std::string_view text) {
  text = Trim(text);

  const size_t hash = text.find('#');

  if (hash == std::string_view::npos) {
    if (text.empty()) return std::nullopt;
    return UiImagePath{std::string(text), std::nullopt};
  }

  const std::string_view path = Trim(text.substr(0, hash));
  const std::string_view rectText = Trim(text.substr(hash + 1));

  if (path.empty() || rectText.empty()) return std::nullopt;

  std::istringstream stream{std::string(rectText)};

  float x, y, width, height;
  char comma;

  if (!(stream >> x >> comma) || comma != ',') return std::nullopt;
  if (!(stream >> y >> comma) || comma != ',') return std::nullopt;
  if (!(stream >> width >> comma) || comma != ',') return std::nullopt;
  if (!(stream >> height)) return std::nullopt;

  std::string extra;
  if (stream >> extra) return std::nullopt;

  if (width <= 0.0f || height <= 0.0f) return std::nullopt;

  return UiImagePath{std::string(path), glm::vec4{x, y, width, height}};
}

std::optional<UiTextureRegion> ResolveImage(const UiPageParser::Context& ctxt,
                                            const UiImagePath& path) {
  if (path.path.empty()) return std::nullopt;

  std::string pathStr = util::NormalizePath(ctxt.assetRoot + path.path);
  const Material* material = ctxt.materialManager.TryGet(ctxt.materialManager.Load(pathStr));
  if (!material) return std::nullopt;

  UiTextureRegion result{
      *material,
      {
          .min = glm::vec2(0.0f),
          .max = glm::vec2(1.0f),
          .arrayLayer = 0,
      },
  };

  if (!path.rect) return result;

  const glm::vec2 textureSize = glm::vec2(material->texture->GetWidth(), material->texture->GetHeight());

  if (textureSize.x <= 0.0f || textureSize.y <= 0.0f) return std::nullopt;

  const glm::vec4& rect = *path.rect;
  const float x = rect.x;
  const float y = rect.y;
  const float width = rect.z;
  const float height = rect.w;

  if (x < 0.0f || y < 0.0f || x + width > textureSize.x || y + height > textureSize.y) {
    return std::nullopt;
  }

  result.region.min = {x / textureSize.x, y / textureSize.y};
  result.region.max = {(x + width) / textureSize.x, (y + height) / textureSize.y};

  return result;
}

void ParseCommonAttributes(const UiPageParser::Context& ctxt, UiElement& element, AttrReader& a) {
  element.SetPos(a.GetOr("pos", ParseLength2, UiLength2{}));
  element.SetSize(a.GetOr("size", ParseLength2, UiLength2{}));

  const UiPoint anchor = a.GetOr("anchor", ParsePoint, UiPoint{});
  element.SetAnchor(anchor);
  element.SetPivot(a.GetOr("pivot", ParsePoint, anchor));
  element.SetPivot(a.GetOr("pivot", ParsePoint, UiPoint{}));

  element.SetVisible(a.GetOr("visible", ParseBool, true));
  element.SetRadius(a.GetOr("radius", ParseFloat, 0.0f));
  element.SetColor(a.GetOr("color", ParseColor, glm::vec4(1.0f)));
  if (auto image = a.Get("image", ParseImage)) {
    element.SetImage(ResolveImage(ctxt, *image));
  }
  element.SetPadding(a.Get("padding", ParsePadding));
  element.SetBgColor(a.GetOr("bg-color", ParseColor, glm::vec4(1.0f)));
  if (auto image = a.Get("bg-image", ParseImage)) {
    element.SetBgImage(ResolveImage(ctxt, *image));
  }
  element.SetVisible(a.GetOr("visible", ParseBool, true));
}

}  // namespace

const std::unordered_map<std::string, UiPageParser::ElementParserFn> UiPageParser::kElementParsers{
    {"container", &UiPageParser::ParseContainer},
    {"button", &UiPageParser::ParseButton},
    {"text", &UiPageParser::ParseText},
};

std::unique_ptr<UiElement> UiPageParser::ParseElement(const Context& ctxt,
                                                      const tinyxml2::XMLElement* element,
                                                      const UiElement* parent,
                                                      UiPageParseResult& result) {
  const auto it = kElementParsers.find(element->Name());

  if (it == kElementParsers.end()) {
    result.Error(std::string("Unknown UI element \"") + element->Name() + "\"",
                 element->GetLineNum());
    return nullptr;
  }

  auto uiElement = it->second(ctxt, element, parent, result);

  
  return uiElement;
}

std::unique_ptr<UiElement> UiPageParser::ParseContainer(const Context& ctxt,
                                                        const tinyxml2::XMLElement* element,
                                                        const UiElement* parent,
                                                        UiPageParseResult& result) {
  AttrReader a(*element, result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto container = std::make_unique<UiContainer>(id, parent);

  ParseCommonAttributes(ctxt, *container, a);

  a.ReportUnknown();

  for (const auto* child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    auto parsed = ParseElement(ctxt, child, container.get(), result);

    if (parsed) {
      container->AddChild(std::move(parsed));
    }
  }

  return container;
}

std::unique_ptr<UiElement> UiPageParser::ParseButton(const Context& ctxt,
                                                     const tinyxml2::XMLElement* element,
                                                     const UiElement* parent,
                                                     UiPageParseResult& result) {
  AttrReader a(*element, result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto button = std::make_unique<UiButton>(id, parent);

  ParseCommonAttributes(ctxt, *button, a);

  // TODO: image, button-specific attributes.

  a.ReportUnknown();
  return button;
}

std::unique_ptr<UiElement> UiPageParser::ParseText(const Context& ctxt,
                                                   const tinyxml2::XMLElement* element,
                                                   const UiElement* parent,
                                                   UiPageParseResult& result) {
  AttrReader a(*element, result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto text = std::make_unique<UiText>(id, parent);

  ParseCommonAttributes(ctxt, *text, a);

  // TODO: text-specific attributes.

  a.ReportUnknown();
  return text;
}

std::optional<std::string> UiPageParser::ParsePageAttributes(const tinyxml2::XMLElement* element,
                                                             UiPageProps& props,
                                                             UiPageParseResult& result) {
  AttrReader a(*element, result);

  props.id = a.GetOr("id", ParseString, std::string{});
  props.input = a.GetOr("input-mode", ParseInputMode, UiInputMode::Passthrough);

  auto scriptPath = a.Get("script", ParseString);

  a.ReportUnknown();
  return scriptPath;
}

UiPageParseResult UiPageParser::Parse(std::string_view xmlPath, const Context& ctxt) {
  UiPageParseResult result;

  std::string file = util::ReadFile(xmlPath);
  if (file.empty()) {
    result.Error("Unable to open file: " + std::string(xmlPath));
    return result;
  }

  tinyxml2::XMLDocument document;
  if (const auto parseError = document.Parse(file.data(), file.size())) {
    result.Error(document.ErrorStr(), document.ErrorLineNum());
    return result;
  }

  const auto* root = document.RootElement();
  if (!root) {
    result.Error("UI document has no root element");
    return result;
  }

  if (std::string_view(root->Name()) != "page") {
    result.Error("Root element must be \"page\"", root->GetLineNum());
    return result;
  }

  UiPageProps props;
  auto scriptPath = ParsePageAttributes(root, props, result);

  std::vector<std::unique_ptr<UiElement>> elements;
  for (const auto* child = root->FirstChildElement(); child; child = child->NextSiblingElement()) {
    if (auto element = ParseElement(ctxt, child, nullptr, result)) {
      elements.emplace_back(std::move(element));
    }
  }

  result.page = std::make_unique<UiPage>(props, std::move(elements));

  if (result.HasErrors()) return result;

  if (scriptPath) {
    auto fileBytes = util::ReadFileBytes(*scriptPath);
    if (fileBytes.empty()) {
      result.Error("Unable to open lua script: " + *scriptPath);
      return result;
    }
    script::LuaScript script(ctxt.luaState, fileBytes, *scriptPath);

    result.page->AttachScript(std::move(script));
  }

  return result;
}
}  // namespace gfx