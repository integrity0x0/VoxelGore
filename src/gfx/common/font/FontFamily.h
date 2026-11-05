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

  [[nodiscard]] const gfx::Font* GetFont(FontStyle style) {
    auto it = std::find(fonts_.begin(), fonts_.end(), style);

    return it != fonts_.end() ? it->get() : nullptr;
  }

 private:
  std::string name_;
  std::vector<std::unique_ptr<gfx::Font>> fonts_;
};
}  // namespace gfx
