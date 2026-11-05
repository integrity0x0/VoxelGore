#pragma once

#include <unordered_map>
#include <string>

#include "FontStyle.h"
#include "Font.h"

namespace gfx {
class FontFamily {
 public:
  explicit FontFamily(const std::string& name, GlyphAtlas& atlas) : name_(name), atlas_(atlas) {}

  [[nodiscard]] const std::string& GetName() const { return name_; }

  void AddFont(FontStyle style, FontFace&& face);

  [[nodiscard]] gfx::Font* FindFont(FontStyle style);

 private:
  [[nodiscard]] std::optional<size_t> FindBestMatch(FontStyle style) const;
 private:
  std::string name_;
  std::reference_wrapper<GlyphAtlas> atlas_;
  std::vector<std::unique_ptr<gfx::Font>> fonts_;
};
}  // namespace gfx
