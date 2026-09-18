#include "NodeBinding.h"

#include <new>
#include <optional>

namespace gfx::ui {

namespace {

const char* kNodeMetatableName = "gui.Node";

int node_gc(lua_State* L) {
  auto* ud = static_cast<std::shared_ptr<Node>*>(lua_touserdata(L, 1));
  ud->~shared_ptr<Node>();
  return 0;
}

glm::vec4 parseCSSLike(lua_State* L, int firstIdx) {
  int n = lua_gettop(L) - firstIdx + 1;
  if (n < 1 || n > 4) {
    luaL_error(L, "expected 1..4 numbers, got %d", n);
  }

  float vals[4];
  for (int i = 0; i < n; ++i) {
    vals[i] = static_cast<float>(luaL_checknumber(L, firstIdx + i));
  }

  float top, right, bottom, left;
  switch (n) {
    case 1:
      top = right = bottom = left = vals[0];
      break;
    case 2:
      top = bottom = vals[0];
      left = right = vals[1];
      break;
    case 3:
      top = vals[0];
      left = right = vals[1];
      bottom = vals[2];
      break;
    case 4:
      top = vals[0];
      right = vals[1];
      bottom = vals[2];
      left = vals[3];
      break;
    default:
      break;
  }
  return glm::vec4(top, right, bottom, left);
}

AnchorX parseAnchorX(lua_State* L, const char* str) {
  if (strcmp(str, "left") == 0) return AnchorX::Left;
  if (strcmp(str, "center") == 0) return AnchorX::Center;
  if (strcmp(str, "right") == 0) return AnchorX::Right;
  luaL_argerror(L, 1, "invalid anchorX (expected 'left', 'center' or 'right')");
  return AnchorX::Left;
}

AnchorY parseAnchorY(lua_State* L, const char* str) {
  if (strcmp(str, "top") == 0) return AnchorY::Top;
  if (strcmp(str, "Center") == 0) return AnchorY::Center;
  if (strcmp(str, "bottom") == 0) return AnchorY::Bottom;
  luaL_argerror(L, 2, "invalid anchorY (expected 'top', 'Center' or 'bottom')");
  return AnchorY::Top;
}

const char* anchorXToString(AnchorX ax) {
  switch (ax) {
    case AnchorX::Left:
      return "left";
    case AnchorX::Center:
      return "center";
    case AnchorX::Right:
      return "right";
  }
  return "unknown";
}

const char* anchorYToString(AnchorY ay) {
  switch (ay) {
    case AnchorY::Top:
      return "top";
    case AnchorY::Center:
      return "center";
    case AnchorY::Bottom:
      return "bottom";
  }
  return "unknown";
}

int node_getPos(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec2 pos = node->getPos();
  lua_pushnumber(L, pos.x);
  lua_pushnumber(L, pos.y);
  return 2;
}

int node_setPos(lua_State* L) {
  auto& node = checkNode(L, 1);
  float x = static_cast<float>(luaL_checknumber(L, 2));
  float y = static_cast<float>(luaL_checknumber(L, 3));
  node->setPos(glm::vec2(x, y));
  return 0;
}

int node_getAbsolutePos(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec2 pos = node->getAbsolutePos();
  lua_pushnumber(L, pos.x);
  lua_pushnumber(L, pos.y);
  return 2;
}

int node_getSize(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec2 size = node->size();
  lua_pushnumber(L, size.x);
  lua_pushnumber(L, size.y);
  return 2;
}

int node_setSize(lua_State* L) {
  auto& node = checkNode(L, 1);
  float w = static_cast<float>(luaL_checknumber(L, 2));
  float h = static_cast<float>(luaL_checknumber(L, 3));
  node->setSize(glm::vec2(w, h));
  return 0;
}

int node_getColor(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec4 c = node->getColor();
  lua_pushnumber(L, c.r);
  lua_pushnumber(L, c.g);
  lua_pushnumber(L, c.b);
  lua_pushnumber(L, c.a);
  return 4;
}

int node_setColor(lua_State* L) {
  auto& node = checkNode(L, 1);
  float r = static_cast<float>(luaL_checknumber(L, 2));
  float g = static_cast<float>(luaL_checknumber(L, 3));
  float b = static_cast<float>(luaL_checknumber(L, 4));
  float a = static_cast<float>(luaL_optnumber(L, 5, 1.0));
  node->setColor(glm::vec4(r, g, b, a));
  return 0;
}

int node_isPressed(lua_State* L) {
  auto& node = checkNode(L, 1);
  lua_pushboolean(L, node->isPressed());
  return 1;
}

int node_getPadding(lua_State* L) {
  auto& node = checkNode(L, 1);
  const auto& p = node->getPadding();
  if (p.has_value()) {
    lua_pushnumber(L, p->x);
    lua_pushnumber(L, p->y);
    lua_pushnumber(L, p->z);
    lua_pushnumber(L, p->w);
  } else {
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
  }
  return 4;
}

int node_setPadding(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec4 p = parseCSSLike(L, 2);
  node->setPadding(p);
  return 0;
}

int node_getMargin(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec4 m = node->getMargin();
  lua_pushnumber(L, m.x);
  lua_pushnumber(L, m.y);
  lua_pushnumber(L, m.z);
  lua_pushnumber(L, m.w);
  return 4;
}

int node_setMargin(lua_State* L) {
  auto& node = checkNode(L, 1);
  glm::vec4 m = parseCSSLike(L, 2);
  node->setMargin(m);
  return 0;
}

int node_getAnchorX(lua_State* L) {
  auto& node = checkNode(L, 1);
  lua_pushstring(L, anchorXToString(node->getAnchorX()));
  return 1;
}

int node_getAnchorY(lua_State* L) {
  auto& node = checkNode(L, 1);
  lua_pushstring(L, anchorYToString(node->getAnchorY()));
  return 1;
}

int node_setAnchorX(lua_State* L) {
  auto& node = checkNode(L, 1);
  const char* str = luaL_checkstring(L, 2);
  node->setAnchorX(parseAnchorX(L, str));
  return 0;
}

int node_setAnchorY(lua_State* L) {
  auto& node = checkNode(L, 1);
  const char* str = luaL_checkstring(L, 2);
  node->setAnchorY(parseAnchorY(L, str));
  return 0;
}

int node_setAnchor(lua_State* L) {
  auto& node = checkNode(L, 1);
  const char* sx = luaL_checkstring(L, 2);
  const char* sy = luaL_checkstring(L, 3);
  node->setAnchor(parseAnchorX(L, sx), parseAnchorY(L, sy));
  return 0;
}

int node_getHeldColor(lua_State* L) {
  auto& node = checkNode(L, 1);
  const auto& opt = node->getHeldColor();
  if (opt) {
    glm::vec4 c = *opt;
    lua_pushnumber(L, c.r);
    lua_pushnumber(L, c.g);
    lua_pushnumber(L, c.b);
    lua_pushnumber(L, c.a);
    return 4;
  } else {
    lua_pushnil(L);
    return 1;
  }
}

int node_setHeldColor(lua_State* L) {
  auto& node = checkNode(L, 1);
  int n = lua_gettop(L) - 1;
  if (n == 0) {
    node->setHeldColor(std::nullopt);
    return 0;
  }
  if (n < 3 || n > 4) {
    luaL_error(L, "expected 3 or 4 numbers for held color, got %d", n);
  }
  float r = static_cast<float>(luaL_checknumber(L, 2));
  float g = static_cast<float>(luaL_checknumber(L, 3));
  float b = static_cast<float>(luaL_checknumber(L, 4));
  float a = static_cast<float>(luaL_optnumber(L, 5, 1.0));
  node->setHeldColor(glm::vec4(r, g, b, a));
  return 0;
}

int node_getHeldScale(lua_State* L) {
  auto& node = checkNode(L, 1);
  lua_pushnumber(L, node->getHeldScale());
  return 1;
}

int node_setHeldScale(lua_State* L) {
  auto& node = checkNode(L, 1);
  float s = static_cast<float>(luaL_checknumber(L, 2));
  node->setHeldScale(s);
  return 0;
}

int node_setOnPress(lua_State* L) {
  auto& node = checkNode(L, 1);
  const char* fn = luaL_checkstring(L, 2);
  node->setOnPress(fn);
  return 0;
}

int node_setOnHeld(lua_State* L) {
  auto& node = checkNode(L, 1);
  const char* fn = luaL_checkstring(L, 2);
  node->setOnHeld(fn);
  return 0;
}

int node_setOnRelease(lua_State* L) {
  auto& node = checkNode(L, 1);
  const char* fn = luaL_checkstring(L, 2);
  node->setOnRelease(fn);
  return 0;
}

int node_consumeJustPressed(lua_State* L) {
  auto& node = checkNode(L, 1);
  lua_pushboolean(L, node->consumeJustPressed());
  return 1;
}

int node_consumeJustReleased(lua_State* L) {
  auto& node = checkNode(L, 1);
  lua_pushboolean(L, node->consumeJustReleased());
  return 1;
}

struct NodeMethod {
  const char* name;
  lua_CFunction fn;
};

constexpr NodeMethod kNodeMethods[] = {
    {"getPos", node_getPos},
    {"setPos", node_setPos},
    {"getAbsolutePos", node_getAbsolutePos},
    {"size", node_getSize},
    {"setSize", node_setSize},
    {"getColor", node_getColor},
    {"setColor", node_setColor},
    {"isPressed", node_isPressed},
    {"getPadding", node_getPadding},
    {"setPadding", node_setPadding},
    {"getMargin", node_getMargin},
    {"setMargin", node_setMargin},
    {"getAnchorX", node_getAnchorX},
    {"getAnchorY", node_getAnchorY},
    {"setAnchorX", node_setAnchorX},
    {"setAnchorY", node_setAnchorY},
    {"setAnchor", node_setAnchor},
    {"getHeldColor", node_getHeldColor},
    {"setHeldColor", node_setHeldColor},
    {"getHeldScale", node_getHeldScale},
    {"setHeldScale", node_setHeldScale},
    {"setOnPress", node_setOnPress},
    {"setOnHeld", node_setOnHeld},
    {"setOnRelease", node_setOnRelease},
    {"consumeJustPressed", node_consumeJustPressed},
    {"consumeJustReleased", node_consumeJustReleased},
};

}  // namespace

void registerNodeMetatable(lua_State* L) {
  if (!luaL_newmetatable(L, kNodeMetatableName)) {
    lua_pop(L, 1);
    return;
  }

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, node_gc);
  lua_setfield(L, -2, "__gc");

  for (const auto& method : kNodeMethods) {
    lua_pushcfunction(L, method.fn);
    lua_setfield(L, -2, method.name);
  }

  lua_pop(L, 1);
}

void pushNode(lua_State* L, std::shared_ptr<Node> node) {
  void* mem = lua_newuserdata(L, sizeof(std::shared_ptr<Node>));
  new (mem) std::shared_ptr<Node>(std::move(node));

  luaL_getmetatable(L, kNodeMetatableName);
  lua_setmetatable(L, -2);
}

std::shared_ptr<Node>& checkNode(lua_State* L, int idx) {
  void* ud = luaL_checkudata(L, idx, kNodeMetatableName);
  return *static_cast<std::shared_ptr<Node>*>(ud);
}

}  // namespace gfx::ui