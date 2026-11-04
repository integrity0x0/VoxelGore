#pragma once

#include <unordered_map>
#include <string>

#include "FontStyle.h"
#include "Font.h"

namespace gfx {
class FontFamily {
 public:
  FontFamily(std::string&& name) : name_(std::move(name)) {}
  FontFamily(const std::string& name) : name_(name) {}

  [[nodiscard]] const std::string& GetName() const { return name_; }

  void AddFont(gfx::Font&& font);

 private:
  std::string name_;
  std::vector<gfx::Font> fonts_;
};
}  // namespace gfx
