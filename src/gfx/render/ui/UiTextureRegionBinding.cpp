#include "UiTextureRegionBinding.h"

#include <lua.hpp>
#include <new>

#include "UiImageResolver.h"

namespace gfx {
namespace {

MaterialManager& UpMm(lua_State* L) {
  return *static_cast<MaterialManager*>(lua_touserdata(L, lua_upvalueindex(1)));
}

std::string_view UpRoot(lua_State* L) {
  size_t len = 0;
  const char* s = lua_tolstring(L, lua_upvalueindex(2), &len);
  return {s, len};
}

}  // namespace

UiTextureRegionBinding::UserData* UiTextureRegionBinding::CheckUd(lua_State* L, int index) {
  return static_cast<UserData*>(luaL_checkudata(L, index, kMeta.data()));
}

void UiTextureRegionBinding::Push(lua_State* L, const UiTextureRegion& region,
                                  std::string_view key) {
  auto* mem = lua_newuserdata(L, sizeof(UserData));
  new (mem) UserData{region, std::string(key)};

  luaL_getmetatable(L, kMeta.data());
  lua_setmetatable(L, -2);
}

std::optional<UiTextureRegion> UiTextureRegionBinding::Check(lua_State* L, int index) {
  if (lua_isnoneornil(L, index)) return std::nullopt;

  UserData* ud = CheckUd(L, index);
  if (ud->region.materialId == kInvalidMaterialId) return std::nullopt;
  return ud->region;
}

int UiTextureRegionBinding::Gc(lua_State* L) {
  CheckUd(L, 1)->~UserData();
  return 0;
}

int UiTextureRegionBinding::Create(lua_State* L) {
  MaterialManager& mm = UpMm(L);
  const std::string_view assetRoot = UpRoot(L);

  if (lua_gettop(L) == 0 || lua_isnoneornil(L, 1)) {
    Push(L, UiTextureRegion{kInvalidMaterialId}, {});
    return 1;
  }

  size_t len = 0;
  const char* s = luaL_checklstring(L, 1, &len);
  auto path = ParseImageRegion(std::string_view(s, len));
  if (!path) return luaL_argerror(L, 1, "invalid texture region string");

  auto resolved = ResolveImage(mm, assetRoot, *path);
  if (!resolved) return luaL_argerror(L, 1, "failed to resolve texture");

  Push(L, *resolved, path->path);
  return 1;
}

void UiTextureRegionBinding::Register(const script::LuaState& state,
                                      MaterialManager& materialManager,
                                      std::string_view assetRoot) {
  lua_State* L = state.Get();

  if (luaL_newmetatable(L, kMeta.data())) {
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_pop(L, 1);

  lua_getglobal(L, "ui");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "ui");
  }

  lua_pushlightuserdata(L, &materialManager);
  lua_pushlstring(L, assetRoot.data(), assetRoot.size());
  lua_pushcclosure(L, Create, 2);
  lua_setfield(L, -2, "TextureRegion");

  lua_pop(L, 1);
}

}  // namespace gfx