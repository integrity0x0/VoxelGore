#pragma once

namespace gfx {

enum class FontWeight { Regular = 400, Medium = 500 }; 

struct FontStyle {
  FontWeight weight = {};
  bool italic = false;
  
  [[nodiscard]] bool operator==(const FontStyle& other) const {
    return weight == other.weight && italic == other.italic;
  }
};

}  // namespace gfx