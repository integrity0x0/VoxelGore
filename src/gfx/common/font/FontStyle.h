#pragma once

namespace gfx {

enum class FontWeight : uint16_t {
  Thin = 100,
  ExtraLight = 200,
  Light = 300,
  Regular = 400,
  Medium = 500,
  SemiBold = 600,
  Bold = 700,
  ExtraBold = 800,
  Black = 900,
};

struct FontStyle {
  FontWeight weight = {};
  bool italic = false;
  
  [[nodiscard]] bool operator==(const FontStyle& other) const {
    return weight == other.weight && italic == other.italic;
  }
};

}  // namespace gfx