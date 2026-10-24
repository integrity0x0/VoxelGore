#pragma once

#include <tinyxml2.h>
#include <unordered_map>

#include "../../../util/hashers.h"
#include "UiButton.h"
#include "UiContainer.h"
#include "UiCheckbox.h"
#include "UiElement.h"
#include "UiPage.h"
#include "UiText.h"
#include "UiImage.h"
#include "UiBar.h"

namespace gfx {
enum class UiSeverity { Error, Warning };
struct UiParseDiagnostic {
  UiSeverity severity;
  std::string msg;
  size_t line;
  UiParseDiagnostic(UiSeverity severity, std::string&& msg, size_t line = 0)
      : severity(severity), msg(std::move(msg)), line(line) {}
};

struct UiPageParseResult {
  std::unique_ptr<UiPage> page;
  std::vector<UiParseDiagnostic> diagnostics;

  [[nodiscard]] bool HasErrors() const {
    return std::any_of(diagnostics.begin(), diagnostics.end(),
                       [](const UiParseDiagnostic& d) { return d.severity == UiSeverity::Error; });
  }

  void Error(std::string&& msg, size_t line = 0) {
    diagnostics.emplace_back(UiSeverity::Error, std::move(msg), line);
  }

  void Warning(std::string&& msg, size_t line = 0) {
    diagnostics.emplace_back(UiSeverity::Warning, std::move(msg), line);
  }
};

class UiPageParser {
 public:
  struct Context {
    script::LuaState& luaState;
    MaterialManager& materialManager;
    std::string assetRoot;

    Context(script::LuaState& luaState, MaterialManager& materialManager,
            const std::string& assetRoot)
        : luaState(luaState), materialManager(materialManager), assetRoot(assetRoot) {}
  };

  struct ParsingState {
    Context ctxt;
    UiPageParseResult result;
    std::unordered_map<std::string, UiStyle> styles;

    ParsingState(Context ctxt) : ctxt(ctxt) {}
  };

  UiPageParser() = delete;

  [[nodiscard]] static UiPageParseResult Parse(std::string_view xmlPath, const Context& ctxt);

 private:

  [[nodiscard]] static std::optional<std::string> ParsePageAttributes(
      const tinyxml2::XMLElement* element, UiPageProps& props,
      ParsingState& state);

  [[nodiscard]] static std::unique_ptr<UiElement> ParseElement(UiPageParser::ParsingState& st,
                                                               const tinyxml2::XMLElement* element,
                                                               const UiElement* parent);

  static void ParseStyles(const tinyxml2::XMLElement& xml, ParsingState& state);

  [[nodiscard]] static std::unique_ptr<UiElement> ParseContainer(
      UiPageParser::ParsingState& st, const tinyxml2::XMLElement* element, const UiElement* parent);

   [[nodiscard]] static std::unique_ptr<UiElement> ParseImage(UiPageParser::ParsingState& st,
                                                             const tinyxml2::XMLElement* element,
                                                             const UiElement* parent);

  [[nodiscard]] static std::unique_ptr<UiElement> ParseButton(UiPageParser::ParsingState& st,
                                                              const tinyxml2::XMLElement* element,
                                                              const UiElement* parent);

  [[nodiscard]] static std::unique_ptr<UiElement> ParseCheckbox(
      UiPageParser::ParsingState& st, const tinyxml2::XMLElement* element, const UiElement* parent);

  [[nodiscard]] static std::unique_ptr<UiElement> ParseBar(
      UiPageParser::ParsingState& st, const tinyxml2::XMLElement* element, const UiElement* parent);


  [[nodiscard]] static std::unique_ptr<UiElement> ParseText(UiPageParser::ParsingState& st,
                                                            const tinyxml2::XMLElement* element,
                                                            const UiElement* parent);
 private:
  using ElementParserFn = std::unique_ptr<UiElement> (*)(ParsingState& st, const tinyxml2::XMLElement*,
                                                         const UiElement*);
  static const std::unordered_map<std::string, ElementParserFn> kElementParsers;
  
};
}  // namespace gfx