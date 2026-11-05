#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../texture/MaterialManager.h"
#include "FontFamily.h"
#include "FontParser.h"
#include "FreeTypeLibrary.h"
#include "GlyphAtlas.h"
#include "../../../util/hashers.h"

namespace gfx {

class FontManager {
 public:
  FontManager(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
              vkcore::TransferContext& transferCtxt, MaterialManager& materialManager,
              const FreeTypeLibrary& ftLib, std::filesystem::path fontsRoot = "assets/fonts");

  [[nodiscard]] Font* RequireFont(std::string_view familyName, FontStyle style = {FontWeight::Regular, false});

  [[nodiscard]] FontFamily* RequireFamily(std::string_view familyName);


  [[nodiscard]] GlyphAtlas& GetAtlas() { return atlas_; }
  [[nodiscard]] const GlyphAtlas& GetAtlas() const { return atlas_; }

  [[nodiscard]] bool LoadFamily(std::string_view jsonPath);

 private:
  MaterialManager& materialManager_;
  const FreeTypeLibrary& ftLib_;
  std::filesystem::path fontsRoot_;

  GlyphAtlas atlas_;
  std::unordered_map<std::string, FontFamily, util::StringHash, std::equal_to<>> families_;
  mutable std::unordered_set<std::string, util::StringHash, std::equal_to<>> failedFamilies_;
};

}  // namespace gfx