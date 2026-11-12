#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "../../../script/LuaState.h"
#include "../../common/texture/MaterialManager.h"
#include "UiTypes.h"

struct lua_State;

namespace gfx {

class UiTextureRegionBinding {
 public:
  static constexpr std::string_view kMeta = "ui.TextureRegion";

  struct UserData {
    UiTextureRegion region;
    std::string key;
  };

  static void Register(const script::LuaState& state, MaterialManager& materialManager,
                       std::string_view assetRoot);

  static void Push(lua_State* L, const UiTextureRegion& region, std::string_view key = {});
  static std::optional<UiTextureRegion> Check(lua_State* L, int index);
  static UserData* CheckUd(lua_State* L, int index = 1);

 private:
  static int Create(lua_State* L);
  static int Gc(lua_State* L);
};

}  // namespace gfx