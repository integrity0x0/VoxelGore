#pragma once

#include <tinyxml2.h>

#include <unordered_map>

#include "../../../util/hashers.h"
#include "UiButton.h"
#include "UiContainer.h"
#include "UiElement.h"
#include "UiPage.h"
#include "UiStateOverrides.h"
#include "UiStateTable.h"
#include "UiText.h"

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
  };
  UiPageParser() = delete;

  [[nodiscard]] UiPageParseResult Parse(std::string_view xmlPath, const Context& ctxt);

 private:
  [[nodiscard]] std::optional<std::string> ParsePageAttributes(const tinyxml2::XMLElement* element,
                                                               UiPageProps& props,
                                                               UiPageParseResult& result);

  [[nodiscard]] std::unique_ptr<UiElement> ParseElement(const tinyxml2::XMLElement* element,
                                                        const UiElement* parent,
                                                        UiPageParseResult& result);

  [[nodiscard]] std::unique_ptr<UiElement> ParseContainer(const tinyxml2::XMLElement* element,
                                                          const UiElement* parent,
                                                          UiPageParseResult& result);

  [[nodiscard]] std::unique_ptr<UiElement> ParseButton(const tinyxml2::XMLElement* element,
                                                       const UiElement* parent,
                                                       UiPageParseResult& result);

  [[nodiscard]] std::unique_ptr<UiElement> ParseText(const tinyxml2::XMLElement* element,
                                                     const UiElement* parent,
                                                     UiPageParseResult& result);

 private:
  using ElementParserFn = std::unique_ptr<UiElement> (*)(const tinyxml2::XMLElement*,
                                        const UiElement*, UiPageParseResult&);
  static const std::unordered_map<std::string, ElementParserFn> kElementParsers;
};
}  // namespace gfx