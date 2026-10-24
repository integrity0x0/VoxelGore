#include "UiPageParser.h"

#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <filesystem>

#include "../../../core/PathPrefixes.h"
#include "../../../util/files.h"
#include "../../../util/pathUtils.h"
#include "UiProp.h"

namespace gfx {

namespace {

class AttrReader {
 public:
  AttrReader(const tinyxml2::XMLElement& xml, UiPageParseResult& result)
      : xml_(xml), result_(result) {}

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

  bool Has(const char* name) const { return xml_.Attribute(name) != nullptr; }

  void Reject(const char* name, UiSeverity sev = UiSeverity::Error) {
    seen_.emplace(name);
    std::string msg =
        std::string("Attribute \"") + name + "\" is not supported in <" + xml_.Name() + ">";
    result_.diagnostics.emplace_back(sev, std::move(msg), static_cast<size_t>(xml_.GetLineNum()));
  }

  void Report(UiSeverity sev, std::string msg, size_t line) {
    result_.diagnostics.emplace_back(sev, std::move(msg), line);
  }

  void ReportUnknown(UiSeverity sev = UiSeverity::Error) const {
    for (auto* a = xml_.FirstAttribute(); a; a = a->Next()) {
      if (seen_.count(std::string(a->Name())) != 0) continue;
      const std::string msg = std::string("Unknown attribute \"") + a->Name() + "\"";
      if (sev == UiSeverity::Error)
        result_.Error(std::string(msg), xml_.GetLineNum());
      else
        result_.Warning(std::string(msg), xml_.GetLineNum());
    }
  }

  void AddSeen(std::string_view text) {
    seen_.emplace(text);
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
  static constexpr std::array values{
      std::pair{std::string_view("left"), UiAlignX::Left},
      std::pair{std::string_view("center"), UiAlignX::Center},
      std::pair{std::string_view("right"), UiAlignX::Right},
  };
  return ParseEnum(text, values);
}

std::optional<UiAlignY> ParseAlignY(std::string_view text) {
  static constexpr std::array values{
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
  std::istringstream stream{std::string(text)};
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
      return UiPadding{values[0], values[0], values[0], values[0]};
    case 2:
      return UiPadding{values[1], values[0], values[1], values[0]};
    case 3:
      return UiPadding{values[1], values[0], values[1], values[2]};
    case 4:
      return UiPadding{values[3], values[0], values[1], values[2]};
  }
  return std::nullopt;
}

struct UiImagePath {
  std::string path;
  std::optional<glm::vec4> rect;
};

std::optional<UiImagePath> ParseImageRegion(std::string_view text) {
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
      {.min = glm::vec2(0.0f), .max = glm::vec2(1.0f), .arrayLayer = 0},
  };
  if (!path.rect) return result;

  const glm::vec2 textureSize(material->texture->GetWidth(), material->texture->GetHeight());
  if (textureSize.x <= 0.0f || textureSize.y <= 0.0f) return std::nullopt;

  const glm::vec4& rect = *path.rect;
  const float x = rect.x, y = rect.y, width = rect.z, height = rect.w;
  if (x < 0.0f || y < 0.0f || x + width > textureSize.x || y + height > textureSize.y)
    return std::nullopt;

  result.region.min = {x / textureSize.x, y / textureSize.y};
  result.region.max = {(x + width) / textureSize.x, (y + height) / textureSize.y};
  return result;
}

// ---------------------------------------------------------------------------
// Styles and states
// ---------------------------------------------------------------------------

std::optional<std::vector<std::string>> ParseStyleNames(std::string_view text) {
  std::istringstream ss(text.data());
  std::vector<std::string> names; 
  std::string token;
  while (std::getline(ss, token, ',')) {
    ss >> token;
    names.emplace_back(std::move(token));
  }

  return names;
}

enum class AttrScope { Element, State, Style };

constexpr bool IsVisualProp(UiProp p) { return p >= UiProp::Color && p <= UiProp::BorderImage; }

std::optional<UiState> ParseUiStateName(std::string_view text) {
  static constexpr std::array values{
      std::pair{std::string_view("disabled"), UiState::Disabled},
      std::pair{std::string_view("hover"), UiState::Hovered},
      std::pair{std::string_view("held"), UiState::Held},
      std::pair{std::string_view("checked"), UiState::Checked},
  };
  return ParseEnum(text, values);
}


std::optional<UiFillFrom> ParseFillFrom(std::string_view text) {
  static constexpr std::array values{
      std::pair{std::string_view("left"), UiFillFrom::Left},
      std::pair{std::string_view("right"), UiFillFrom::Right},
      std::pair{std::string_view("top"), UiFillFrom::Top},
      std::pair{std::string_view("bottom"), UiFillFrom::Bottom},
      std::pair{std::string_view("center-x"), UiFillFrom::CenterX},
      std::pair{std::string_view("center-y"), UiFillFrom::CenterY},
  };

  return ParseEnum(text, values);
}

void ParseAttrs(UiPageParser::ParsingState& st, AttrReader& a, AttrScope scope,
                std::optional<UiElementType> type, UiStyle& out) {
  const UiSeverity sev = (scope == AttrScope::Element) ? UiSeverity::Error : UiSeverity::Warning;

  for (size_t i = 0; i < kPropCount; ++i) {
    const UiProp p = static_cast<UiProp>(i);
    const char* name = kPropNames[i].data();
    if (!a.Has(name)) continue;

    if (scope == AttrScope::State && !IsVisualProp(p)) {
      a.Reject(name, sev);
      continue;
    }
    if (type && !IsPropSupported(p, *type)) {
      a.Reject(name, sev);
      continue;
    }

    if (IsEventProp(p)) continue;

    switch (p) {
      case UiProp::Pos:
        if (auto v = a.Get(name, ParseLength2)) out.layoutProps.pos = *v;
        break;
      case UiProp::Size:
        if (auto v = a.Get(name, ParseLength2)) out.layoutProps.size = *v;
        break;
      case UiProp::Anchor:
        if (auto v = a.Get(name, ParsePoint)) out.layoutProps.anchor = *v;
        break;
      case UiProp::Pivot:
        if (auto v = a.Get(name, ParsePoint)) out.layoutProps.pivot = *v;
        break;
      case UiProp::Padding:
        if (auto v = a.Get(name, ParsePadding)) out.layoutProps.padding = *v;
        break;

      case UiProp::Color:
        if (auto v = a.Get(name, ParseColor)) out.visualProps.color = *v;
        break;
      case UiProp::BgColor:
        if (auto v = a.Get(name, ParseColor)) out.visualProps.bgColor = *v;
        break;
      case UiProp::BorderColor:
        if (auto v = a.Get(name, ParseColor)) out.visualProps.borderColor = *v;
        break;
      case UiProp::Radius:
        if (auto v = a.Get(name, ParseFloat)) out.visualProps.radius = *v;
        break;
      case UiProp::BgRadius:
        if (auto v = a.Get(name, ParseFloat)) out.visualProps.bgRadius = *v;
        break;
      case UiProp::BorderWidth:
        if (auto v = a.Get(name, ParseFloat)) out.visualProps.borderWidth = *v;
        break;
      case UiProp::Scale:
        if (scope == AttrScope::State)
          if (auto v = a.Get(name, ParseFloat)) out.visualProps.scale = *v;
        break;
      case UiProp::Src:
        if (auto path = a.Get(name, ParseImageRegion))
          if (auto reg = ResolveImage(st.ctxt, *path)) out.visualProps.src = std::move(*reg);
        break;
      case UiProp::BgImage:
        if (auto path = a.Get(name, ParseImageRegion))
          if (auto reg = ResolveImage(st.ctxt, *path)) out.visualProps.bgImage = std::move(*reg);
        break;
      case UiProp::BorderImage:
        if (auto path = a.Get(name, ParseImageRegion))
          if (auto reg = ResolveImage(st.ctxt, *path)) out.visualProps.borderImage = std::move(*reg);
        break;
      case UiProp::FillFrom:
        if (auto v = a.Get(name, ParseFillFrom)) out.visualProps.fillFrom = *v;
        break;
      case UiProp::FillImage:
        if (auto path = a.Get(name, ParseImageRegion))
          if (auto reg = ResolveImage(st.ctxt, *path)) out.visualProps.fillImage = std::move(*reg);
        break;
      case UiProp::FillColor:
        if (auto v = a.Get(name, ParseColor)) out.visualProps.fillColor = *v;
        break;
      case UiProp::FillRadius:
        if (auto v = a.Get(name, ParseFloat)) out.visualProps.fillRadius = *v;
        break;

      default:
        break;
    }
  }
}

void ParseStateBlock(UiPageParser::ParsingState& st, const tinyxml2::XMLElement& xml,
                     std::optional<UiElementType> type, UiStyle& out) {
  AttrReader a(xml, st.result);

  auto state = a.Get("name", ParseUiStateName);
  if (!state) {
    st.result.Error("<state> requires valid \"name\"", xml.GetLineNum());
    return;
  }

  UiStyle tmp;
  ParseAttrs(st, a, AttrScope::State, type, tmp);
  a.ReportUnknown(UiSeverity::Warning);

  out.stateOverrides[static_cast<size_t>(*state)].MergeFrom(tmp.visualProps);
}

void ParseStyleDef(UiPageParser::ParsingState& st, const tinyxml2::XMLElement& xml) {
  AttrReader a(xml, st.result);
  const auto name = a.Get("name", ParseString);
  if (!name) {
    if (!a.Has("name")) a.Report(UiSeverity::Error, "<style> requires \"name\"", xml.GetLineNum());
    return;
  }
  if (st.styles.contains(*name)) {
    a.Report(UiSeverity::Error, "Duplicate style \"" + *name + "\"", xml.GetLineNum());
    return;
  }

  UiStyle style;
  if (const auto base = a.Get("extends", ParseString)) {
    const auto it = st.styles.find(*base);
    if (it == st.styles.end())
      a.Report(UiSeverity::Error, "Unknown base style \"" + *base + "\" (declare it above)", xml.GetLineNum());
    else
      style = it->second;
  }

  ParseAttrs(st, a, AttrScope::Style, std::nullopt, style);
  for (const auto* c = xml.FirstChildElement(); c; c = c->NextSiblingElement()) {
    if (std::string_view(c->Name()) == "state")
      ParseStateBlock(st, *c, std::nullopt, style);
    else
      st.result.Warning(std::string("Unexpected <") + c->Name() + "> inside <style>",
                        c->GetLineNum());
  }
  a.ReportUnknown(UiSeverity::Warning);
  st.styles.emplace(*name, std::move(style));
}

void ParseStylesBlock(UiPageParser::ParsingState& st, const tinyxml2::XMLElement& xml) {
  for (const auto* c = xml.FirstChildElement(); c; c = c->NextSiblingElement()) {
    if (std::string_view(c->Name()) == "style")
      ParseStyleDef(st, *c);
    else
      st.result.Warning(std::string("Unexpected <") + c->Name() + "> inside <styles>",
                        c->GetLineNum());
  }
}
std::optional<std::vector<std::string>> ParseNameList(std::string_view text) {
  std::vector<std::string> names;
  const auto isSep = [](char c) { return c == ',' || std::isspace(static_cast<unsigned char>(c)); };
  size_t i = 0;
  while (i < text.size()) {
    while (i < text.size() && isSep(text[i])) ++i;
    const size_t start = i;
    while (i < text.size() && !isSep(text[i])) ++i;
    if (i > start) names.emplace_back(text.substr(start, i - start));
  }
  if (names.empty()) return std::nullopt;
  return names;
}

std::optional<UiStateFlags> ParseVisibleOn(std::string_view text) {
  const auto names = ParseNameList(text);
  if (!names) return std::nullopt;
  UiStateFlags flags = {};
  for (const auto& name : *names) {
    const auto state = ParseUiStateName(name);
    if (!state) return std::nullopt;
    flags |= static_cast<UiStateFlags>(1u << static_cast<uint32_t>(*state));
  }
  return flags;
}

void ParseCommon(UiPageParser::ParsingState& st, UiElement& el, const tinyxml2::XMLElement& xml,
                 AttrReader& a) {
  const UiElementType type = el.GetNodeType();

  UiStyle style;

  if (auto names = a.Get("style", ParseNameList)) {
    for (const auto& name : *names) {
      auto it = st.styles.find(name);

      if (it != st.styles.end()) {
        style.MergeFrom(it->second);
      } else {
        st.result.Error("Invalid style name: " + name, xml.GetLineNum());
      }
    }
  }


  ParseAttrs(st, a, AttrScope::Element, type, style);

  for (auto* c = xml.FirstChildElement(); c; c = c->NextSiblingElement()) {
    if (std::string_view(c->Name()) == "state")
      ParseStateBlock(st, *c, type, style);
  }

  if (!style.layoutProps.pivot && style.layoutProps.anchor)
    style.layoutProps.pivot = style.layoutProps.anchor;

  el.ApplyStyle(style);

  for (size_t i = static_cast<size_t>(UiProp::OnClick); i < kPropCount; ++i) {
    const UiProp p = static_cast<UiProp>(i);
    const char* name = kPropNames[i].data();
    if (!a.Has(name)) continue;
    if (!IsPropSupported(p, type)) {
      a.Reject(name);
      continue;
    }
    el.SetCallback(*EventOf(p), a.GetOr(name, ParseString, std::string{}));
  }

  el.SetVisible(a.GetOr("visible", ParseBool, true));
  el.SetIntercative(a.GetOr("interactive", ParseBool, true));

  el.SetVisibleOn(a.GetOr("visible-on", ParseVisibleOn, UiStateFlags::None));


}

}  // namespace

const std::unordered_map<std::string, UiPageParser::ElementParserFn> UiPageParser::kElementParsers{
    {"container", &UiPageParser::ParseContainer},
    {"button", &UiPageParser::ParseButton},
    {"text", &UiPageParser::ParseText},
    {"image", &UiPageParser::ParseImage},
    {"bar", &UiPageParser::ParseBar},
};

[[nodiscard]] std::unique_ptr<UiElement> UiPageParser::ParseImage(ParsingState& st,
                                                                  const tinyxml2::XMLElement* element,
                                                                  const UiElement* parent) {
  AttrReader a(*element, st.result);
  const std::string id = a.GetOr("id", ParseString, std::string{});
  auto el = std::make_unique<UiImage>(id, parent);

  ParseCommon(st, *el, *element, a);

  a.ReportUnknown();
  return el;
}

std::unique_ptr<UiElement> UiPageParser::ParseElement(ParsingState& st,
                                                      const tinyxml2::XMLElement* element,
                                                      const UiElement* parent) {
  const auto it = kElementParsers.find(element->Name());

  if (it == kElementParsers.end()) {
    st.result.Error(std::string("Unknown UI element \"") + element->Name() + "\"",
                    element->GetLineNum());
    return nullptr;
  }

  auto uiElement = it->second(st, element, parent);

  
  return uiElement;
}

std::unique_ptr<UiElement> UiPageParser::ParseContainer(ParsingState& st,
                                                        const tinyxml2::XMLElement* element,
                                                        const UiElement* parent) {
  AttrReader a(*element, st.result);
  const std::string id = a.GetOr("id", ParseString, std::string{});
  auto el = std::make_unique<UiContainer>(id, parent);

  ParseCommon(st, *el, *element, a);

  for (const auto* child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    if (std::string_view(child->Name()) == "state") continue;
    auto parsed = ParseElement(st, child, el.get());
    if (parsed) el->AddChild(std::move(parsed));
  }

  a.ReportUnknown();
  return el;
}

std::unique_ptr<UiElement> UiPageParser::ParseButton(ParsingState& st,
                                                     const tinyxml2::XMLElement* element,
                                                     const UiElement* parent) {
  AttrReader a(*element, st.result);
  const std::string id = a.GetOr("id", ParseString, std::string{});
  auto el = std::make_unique<UiButton>(id, parent);

  ParseCommon(st, *el, *element, a); 

  for (const auto* child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    if (std::string_view(child->Name()) == "state") continue;
    auto parsed = ParseElement(st, child, el.get());
    if (parsed) el->AddChild(std::move(parsed));
  }

  a.ReportUnknown();
  return el;
}

std::unique_ptr<UiElement> UiPageParser::ParseCheckbox(UiPageParser::ParsingState& st,
                                                        const tinyxml2::XMLElement* element,
                                                        const UiElement* parent) {
  AttrReader a(*element, st.result);
  const std::string id = a.GetOr("id", ParseString, std::string{});
  auto el = std::make_unique<UiCheckbox>(id, parent);

  ParseCommon(st, *el, *element, a);

  for (const auto* child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    if (std::string_view(child->Name()) == "state") continue;
    auto parsed = ParseElement(st, child, el.get());
    if (parsed) el->AddChild(std::move(parsed));
  }

  a.ReportUnknown();
  return el;
}

[[nodiscard]] std::unique_ptr<UiElement> UiPageParser::ParseBar(UiPageParser::ParsingState& st,
                                                                const tinyxml2::XMLElement* element,
                                                                const UiElement* parent) {
  AttrReader a(*element, st.result);
  const std::string id = a.GetOr("id", ParseString, std::string{});
  auto el = std::make_unique<UiBar>(id, parent);

  ParseCommon(st, *el, *element, a);

  for (const auto* child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    if (std::string_view(child->Name()) == "state") continue;
    auto parsed = ParseElement(st, child, el.get());
    if (parsed) el->AddChild(std::move(parsed));
  }

  a.ReportUnknown();
  return el;
}

std::unique_ptr<UiElement> UiPageParser::ParseText(UiPageParser::ParsingState& st,
                                                   const tinyxml2::XMLElement* element,
                                                   const UiElement* parent) {
  AttrReader a(*element, st.result);

  const std::string id = a.GetOr("id", ParseString, std::string{});

  auto text = std::make_unique<UiText>(id, parent);


  // TODO: text-specific attributes.

  a.ReportUnknown();
  return text;
}

std::optional<std::string> UiPageParser::ParsePageAttributes(const tinyxml2::XMLElement* element,
                                                             UiPageProps& props,
                                                             ParsingState& st) {
  AttrReader a(*element, st.result);

  props.id = a.GetOr("id", ParseString, std::string{});
  props.input = a.GetOr("input-mode", ParseInputMode, UiInputMode::Passthrough);

  auto scriptPath = a.Get("script", ParseString);

  if (scriptPath) scriptPath = core::kAssetsPrefix + scriptPath.value();

  a.ReportUnknown();
  return scriptPath;
}

void UiPageParser::ParseStyles(const tinyxml2::XMLElement& xml, ParsingState& st) {
  if (std::string_view(xml.Name()) == "styles") {
    ParseStylesBlock(st, xml);
  }
}

UiPageParseResult UiPageParser::Parse(std::string_view xmlPath, const Context& ctxt) {
  
  ParsingState st(ctxt);

  std::string file = util::ReadFile(xmlPath);
  if (file.empty()) {
    st.result.Error("Unable to open file: " + std::string(xmlPath));
    return std::move(st.result);
  }

  tinyxml2::XMLDocument document;
  if (const auto parseError = document.Parse(file.data(), file.size())) {
    st.result.Error(document.ErrorStr(), document.ErrorLineNum());
    return std::move(st.result);
  }

  const auto* root = document.RootElement();
  if (!root) {
    st.result.Error("UI document has no root element");
    return std::move(st.result);
  }

  if (std::string_view(root->Name()) != "page") {
    st.result.Error("Root element must be \"page\"", root->GetLineNum());
    return std::move(st.result);
  }

  UiPageProps props;
  auto scriptPath = ParsePageAttributes(root, props, st);
   
  std::vector<std::unique_ptr<UiElement>> elements;
  for (const auto* child = root->FirstChildElement(); child; child = child->NextSiblingElement()) {
    if (auto element = ParseElement(st, child, nullptr)) {
      elements.emplace_back(std::move(element));
    } else {
      ParseStyles(*child, st);
    }
  }

  st.result.page = std::make_unique<UiPage>(props, std::move(elements));

  if (st.result.HasErrors()) return std::move(st.result);

  if (scriptPath) {
    auto fileBytes = util::ReadFileBytes(*scriptPath);
    if (fileBytes.empty()) {
      st.result.Error("Unable to open lua script: " + *scriptPath);
      return std::move(st.result);
    }

    try {
      script::LuaScript script(ctxt.luaState, fileBytes, *scriptPath);
      st.result.page->AttachScript(std::move(script));
    } catch (const std::exception& e) {
      st.result.Error(e.what());
    }
  }

  return std::move(st.result);
}
}  // namespace gfx