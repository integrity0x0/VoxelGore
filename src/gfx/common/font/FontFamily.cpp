#include "FontFamily.h"

namespace gfx {

void FontFamily::AddFont(FontStyle style, FontFace&& face) {
  fonts_.emplace_back(std::make_unique<Font>(name_, style, std::move(face), atlas_));
}

std::optional<size_t> FontFamily::FindBestMatch(FontStyle requested) const {
  if (fonts_.empty()) {
    return std::nullopt;
  }

  for (size_t i = 0; i < fonts_.size(); ++i) {
    if (fonts_[i]->GetStyle() == requested) {
      return i;
    }
  }

  size_t bestIdx = 0;
  size_t bestScore = std::numeric_limits<size_t>::max();

  for (size_t i = 0; i < fonts_.size(); ++i) {
    const auto& s = fonts_[i]->GetStyle();

    size_t score = 0;

    if (s.italic != requested.italic) {
      score += 10000;
    }

    score += static_cast<size_t>(s.weight) - static_cast<size_t>(requested.weight);

    if (s.weight == FontWeight::Regular) {
      score--;
    }

    if (score < bestScore) {
      bestScore = score;
      bestIdx = i;
    }
  }

  return bestIdx;
}

gfx::Font* FontFamily::FindFont(FontStyle style) {
  if (auto index = FindBestMatch(style)) {
    return fonts_[*index].get();
  } else {
    return nullptr;
  }
}

}  // namespace gfx