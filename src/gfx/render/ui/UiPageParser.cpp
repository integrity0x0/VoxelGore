#include "UiPageParser.h"

#include <algorithm>
#include <unordered_set>

#include "../../../util/files.h"

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

void ParseCommonAttributes(UiElement& element, AttrReader& a) {
  element.SetPos(a.GetOr("pos", ParseLength2, UiLength2{}));
  element.SetSize(a.GetOr("size", ParseLength2, UiLength2{}));

  element.SetAnchor(a.GetOr("anchor", ParsePoint, UiPoint{}));
  element.SetPivot(a.GetOr("pivot", ParsePoint, UiPoint{}));

  element.SetVisible(a.GetOr("visible", ParseBool, true));
  element.SetRadius(a.GetOr("radius", ParseFloat, 0.0f));
  element.SetColor(a.GetOr("color", ParseColor, glm::vec4(1.0f)));

}

}  // namespace

const std::unordered_map<std::string, UiPageParser::ElementParserFn> UiPageParser::kElementParsers{
    {"container", &UiPageParser::ParseContainer},
    {"button", &UiPageParser::ParseButton},
    {"text", &UiPageParser::ParseText},
};

std::unique_ptr<UiElement> UiPageParser::ParseElement(const tinyxml2::XMLElement* element,
                                                      const UiElement* parent,
                                                      UiPageParseResult& result) {
  const auto it = kElementParsers.find(element->Name());

  if (it == kElementParsers.end()) {
    result.Error(std::string("Unknown UI element \"") + element->Name() + "\"",
                 element->GetLineNum());
    return nullptr;
  }

  auto uiElement = it->second(element, parent, result);

  for (const auto* child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    auto parsed = ParseElement(child, uiElement.get(), result);

    if (parsed) {
      uiElement->AddChild(std::move(parsed));
    }
  }
  return uiElement;
}

std::unique_ptr<UiElement> UiPageParser::ParseContainer(const tinyxml2::XMLElement* element,
                                                        const UiElement* parent,
                                                        UiPageParseResult& result) {
  AttrReader a(*element, result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto container = std::make_unique<UiContainer>(id, parent);

  ParseCommonAttributes(*container, a);

  a.ReportUnknown();
  return container;
}

std::unique_ptr<UiElement> UiPageParser::ParseButton(const tinyxml2::XMLElement* element,
                                                     const UiElement* parent,
                                                     UiPageParseResult& result) {
  AttrReader a(*element, result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto button = std::make_unique<UiButton>(id, parent);

  ParseCommonAttributes(*button, a);

  // TODO: image, button-specific attributes.

  a.ReportUnknown();
  return button;
}

std::unique_ptr<UiElement> UiPageParser::ParseText(const tinyxml2::XMLElement* element,
                                                   const UiElement* parent,
                                                   UiPageParseResult& result) {
  AttrReader a(*element, result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto text = std::make_unique<UiText>(id, parent);

  ParseCommonAttributes(*text, a);

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

  result.page = std::make_unique<UiPage>(props);
  std::vector<std::unique_ptr<UiElement>> elements;
  for (const auto* child = root->FirstChildElement(); child; child = child->NextSiblingElement()) {
    if (auto element = ParseElement(child, nullptr, result)) {
      elements.emplace_back(std::move(element));
    }
  }

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