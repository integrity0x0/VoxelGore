#include "FontManager.h"

#include "FontParser.h"
#include "../../../util/files.h"

namespace gfx {

FontManager::FontManager(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
                         vkcore::TransferContext& transferCtxt, MaterialManager& materialManager,
                         const FreeTypeLibrary& ftLib,
                         std::filesystem::path fontsRoot)
    : materialManager_(materialManager),
      ftLib_(ftLib),
      fontsRoot_(std::move(fontsRoot)),
      atlas_(device, memoryAllocator, transferCtxt, materialManager) {}

Font* FontManager::RequireFont(std::string_view familyName, FontStyle style) {
  if (FontFamily* family = RequireFamily(familyName)) {
    return family->FindFont(style);
  } else {
    return nullptr;
  }
}

FontFamily* FontManager::RequireFamily(std::string_view familyName) {
  const std::string name(familyName);

  if (auto it = families_.find(name); it != families_.end()) {
    return &it->second;
  }

  if (failedFamilies_.contains(name)) return nullptr;

  if (!LoadFamily(name)) return nullptr;
  
  auto it = families_.find(name);

  return &it->second;
}


bool FontManager::LoadFamily(std::string_view name) {
  const std::filesystem::path jsonPath = fontsRoot_ / (std::string(name) + ".json");

  auto def = FontParser::Parse(jsonPath.string());
  if (!def) return false;

  const std::filesystem::path familyDir = jsonPath.parent_path();

  FontFamily family(def->name, atlas_);

  for (auto& face : def->faces) {
    const std::filesystem::path fontPath = familyDir / face.file;

    auto bytes = util::ReadFileBytes(fontPath.string());
    if (bytes.empty()) {
      return false;
    }

    family.AddFont(face.style, FontFace(ftLib_, std::move(bytes)));
  }

  families_.insert_or_assign(def->name, std::move(family));
  return true;
}


}  // namespace gfx