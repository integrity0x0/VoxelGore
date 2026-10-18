#include "UiElementBinding.h"

#include "UiCheckbox.h"
#include "UiElement.h"

namespace gfx {
namespace {
void PushElement(lua_State* L, UiElement* element) {
  if (!element) {
    lua_pushnil(L);
    return;
  }

  auto** userdata = static_cast<UiElement**>(lua_newuserdata(L, sizeof(UiElement*)));

  *userdata = element;

  luaL_getmetatable(L, UiElementBinding::kElementMeta.data());
  lua_setmetatable(L, -2);
}

int AbsIndex(lua_State* L, int index) {
  return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

void SetFunction(lua_State* L, int table, const char* name, lua_CFunction function) {
  table = AbsIndex(L, table);
  lua_pushcfunction(L, function);
  lua_setfield(L, table, name);
}

UiElement* CheckElement(lua_State* L, int index = 1) {
  auto** userdata =
      static_cast<UiElement**>(luaL_checkudata(L, index, UiElementBinding::kElementMeta.data()));

  luaL_argcheck(L, *userdata != nullptr, index, "UiElement is null");
  return *userdata;
}

template <typename T>
T* CheckDerivedElement(lua_State* L, int index = 1) {
  auto* element = CheckElement(L, index);
  auto* derived = dynamic_cast<T*>(element);

  luaL_argcheck(L, derived != nullptr, index, "UiElement has an incompatible type");

  return derived;
}

// -----------------------------------------------------------------------------
// Lua table readers
// -----------------------------------------------------------------------------

float CheckFloat(lua_State* L, int index) { return static_cast<float>(luaL_checknumber(L, index)); }

float CheckFieldFloat(lua_State* L, int table, const char* field) {
  table = AbsIndex(L, table);
  lua_getfield(L, table, field);
  const float value = CheckFloat(L, -1);
  lua_pop(L, 1);
  return value;
}

float CheckArrayFloat(lua_State* L, int table, int index) {
  table = AbsIndex(L, table);
  lua_rawgeti(L, table, index);
  const float value = CheckFloat(L, -1);
  lua_pop(L, 1);
  return value;
}

UiUnit CheckUnit(lua_State* L, int index) {
  const auto unit = static_cast<UiUnit>(luaL_checkinteger(L, index));

  luaL_argcheck(L, unit == UiUnit::Px || unit == UiUnit::Percent, index, "invalid UI length unit");

  return unit;
}

UiLength CheckLength(lua_State* L, int index) {
  if (lua_type(L, index) == LUA_TNUMBER) {
    return {.value = static_cast<float>(lua_tonumber(L, index)), .unit = UiUnit::Px};
  }

  luaL_checktype(L, index, LUA_TTABLE);
  index = AbsIndex(L, index);

  UiLength length{};

  length.value = CheckFieldFloat(L, index, "value");

  lua_getfield(L, index, "unit");
  length.unit = CheckUnit(L, -1);
  lua_pop(L, 1);

  return length;
}

UiLength2 CheckLength2(lua_State* L, int index) {
  luaL_checktype(L, index, LUA_TTABLE);
  index = AbsIndex(L, index);

  UiLength2 result{};

  lua_getfield(L, index, "x");
  result.x = CheckLength(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, index, "y");
  result.y = CheckLength(L, -1);
  lua_pop(L, 1);

  return result;
}

UiAlignX CheckAlignX(lua_State* L, int index) {
  const auto value = static_cast<UiAlignX>(luaL_checkinteger(L, index));

  luaL_argcheck(L, value == UiAlignX::Left || value == UiAlignX::Center || value == UiAlignX::Right,
                index, "invalid UiAlignX");

  return value;
}

UiAlignY CheckAlignY(lua_State* L, int index) {
  const auto value = static_cast<UiAlignY>(luaL_checkinteger(L, index));

  luaL_argcheck(L, value == UiAlignY::Top || value == UiAlignY::Center || value == UiAlignY::Bottom,
                index, "invalid UiAlignY");

  return value;
}


UiPoint CheckPoint(lua_State* L, int index) {
  luaL_checktype(L, index, LUA_TTABLE);
  index = AbsIndex(L, index);

  UiPoint point{};

  lua_getfield(L, index, "x");
  point.x = CheckAlignX(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, index, "y");
  point.y = CheckAlignY(L, -1);
  lua_pop(L, 1);

  return point;
}

glm::vec2 CheckVec2(lua_State* L, int index) {
  luaL_checktype(L, index, LUA_TTABLE);
  index = AbsIndex(L, index);

  return {
      CheckFieldFloat(L, index, "x"),
      CheckFieldFloat(L, index, "y"),
  };
}

int HexDigit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// "#RGB", "#RGBA", "#RRGGBB", "#RRGGBBAA" ('#' is optional)
glm::vec4 ParseHexColor(lua_State* L, int index, std::string_view text) {
  if (!text.empty() && text.front() == '#') {
    text.remove_prefix(1);
  }

  const size_t digits = text.size();
  const bool shortForm = digits == 3 || digits == 4;
  const bool longForm = digits == 6 || digits == 8;

  luaL_argcheck(L, shortForm || longForm, index, "expected #RGB, #RGBA, #RRGGBB or #RRGGBBAA");

  const size_t channels = shortForm ? digits : digits / 2;
  const size_t step = shortForm ? 1 : 2;

  int c[4] = {255, 255, 255, 255};

  for (size_t i = 0; i < channels; ++i) {
    int value = 0;

    for (size_t j = 0; j < step; ++j) {
      const int digit = HexDigit(text[i * step + j]);
      luaL_argcheck(L, digit >= 0, index, "invalid hex digit in color");
      value = value * 16 + digit;
    }

    c[i] = shortForm ? value * 17 : value;  // 0xF -> 0xFF
  }

  return {c[0] / 255.0f, c[1] / 255.0f, c[2] / 255.0f, c[3] / 255.0f};
}

glm::vec4 CheckColor(lua_State* L, int index) {
  if (lua_type(L, index) == LUA_TSTRING) {
    size_t length = 0;
    const char* text = lua_tolstring(L, index, &length);
    return ParseHexColor(L, index, std::string_view(text, length));
  }

  luaL_checktype(L, index, LUA_TTABLE);
  index = AbsIndex(L, index);

  glm::vec4 color{
      CheckArrayFloat(L, index, 1),
      CheckArrayFloat(L, index, 2),
      CheckArrayFloat(L, index, 3),
      1.0f,
  };

  // альфа optional
  lua_rawgeti(L, index, 4);
  if (!lua_isnil(L, -1)) {
    color.a = CheckFloat(L, -1);
  }
  lua_pop(L, 1);

  return color;
}

UiPadding CheckPadding(lua_State* L, int index) {
  luaL_checktype(L, index, LUA_TTABLE);
  index = AbsIndex(L, index);

  UiPadding padding{};

  lua_getfield(L, index, "left");
  padding.left = CheckLength(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, index, "top");
  padding.top = CheckLength(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, index, "right");
  padding.right = CheckLength(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, index, "bottom");
  padding.bottom = CheckLength(L, -1);
  lua_pop(L, 1);

  return padding;
}

// -----------------------------------------------------------------------------
// Lua table writers
// -----------------------------------------------------------------------------

void SetNumber(lua_State* L, int table, const char* field, lua_Number value) {
  table = AbsIndex(L, table);
  lua_pushnumber(L, value);
  lua_setfield(L, table, field);
}

void PushLength(lua_State* L, const UiLength& length) {
  lua_createtable(L, 0, 2);

  SetNumber(L, -1, "value", length.value);

  lua_pushinteger(L, static_cast<lua_Integer>(length.unit));
  lua_setfield(L, -2, "unit");
}

void PushLength2(lua_State* L, const UiLength2& length) {
  lua_createtable(L, 0, 2);

  PushLength(L, length.x);
  lua_setfield(L, -2, "x");

  PushLength(L, length.y);
  lua_setfield(L, -2, "y");
}

void PushPoint(lua_State* L, const UiPoint& point) {
  lua_createtable(L, 0, 2);

  lua_pushinteger(L, static_cast<lua_Integer>(point.x));
  lua_setfield(L, -2, "x");

  lua_pushinteger(L, static_cast<lua_Integer>(point.y));
  lua_setfield(L, -2, "y");
}

void PushColor(lua_State* L, const glm::vec4& color) {
  lua_createtable(L, 4, 0);

  for (int i = 0; i < 4; ++i) {
    lua_pushnumber(L, color[i]);
    lua_rawseti(L, -2, i + 1);
  }
}

void PushPadding(lua_State* L, const UiPadding& padding) {
  lua_createtable(L, 0, 4);

  PushLength(L, padding.left);
  lua_setfield(L, -2, "left");

  PushLength(L, padding.top);
  lua_setfield(L, -2, "top");

  PushLength(L, padding.right);
  lua_setfield(L, -2, "right");

  PushLength(L, padding.bottom);
  lua_setfield(L, -2, "bottom");
}

// -----------------------------------------------------------------------------
// Identity and hierarchy
// -----------------------------------------------------------------------------

int GetId(lua_State* L) {
  const auto id = CheckElement(L)->GetId();
  lua_pushlstring(L, id.data(), id.size());
  return 1;
}

int GetParent(lua_State* L) {
  const auto* parent = CheckElement(L)->GetParent();
  PushElement(L, const_cast<UiElement*>(parent));
  return 1;
}

int GetRoot(lua_State* L) {
  UiElement* root = CheckElement(L);

  while (const UiElement* parent = root->GetParent()) {
    root = const_cast<UiElement*>(parent);
  }

  PushElement(L, root);
  return 1;
}

int GetNodeType(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(CheckElement(L)->GetNodeType()));
  return 1;
}

// -----------------------------------------------------------------------------
// Transform getters
// -----------------------------------------------------------------------------

int GetPos(lua_State* L) {
  PushLength2(L, CheckElement(L)->GetPos());
  return 1;
}

int GetSize(lua_State* L) {
  PushLength2(L, CheckElement(L)->GetSize());
  return 1;
}

int GetAnchor(lua_State* L) {
  PushPoint(L, CheckElement(L)->GetAnchor());
  return 1;
}

int GetPivot(lua_State* L) {
  PushPoint(L, CheckElement(L)->GetPivot());
  return 1;
}

// -----------------------------------------------------------------------------
// Transform setters
// -----------------------------------------------------------------------------

int SetPos(lua_State* L) {
  CheckElement(L)->SetPos(CheckLength2(L, 2));
  return 0;
}

int SetSize(lua_State* L) {
  CheckElement(L)->SetSize(CheckLength2(L, 2));
  return 0;
}

int SetAnchor(lua_State* L) {
  CheckElement(L)->SetAnchor(CheckPoint(L, 2));
  return 0;
}

int SetPivot(lua_State* L) {
  CheckElement(L)->SetPivot(CheckPoint(L, 2));
  return 0;
}

// -----------------------------------------------------------------------------
// Appearance getters
// -----------------------------------------------------------------------------

int GetColor(lua_State* L) {
  PushColor(L, CheckElement(L)->GetColor());
  return 1;
}

int GetBgColor(lua_State* L) {
  PushColor(L, CheckElement(L)->GetBgColor());
  return 1;
}

int GetRadius(lua_State* L) {
  lua_pushnumber(L, CheckElement(L)->GetRadius());
  return 1;
}

int GetBgRadius(lua_State* L) {
  const auto& radius = CheckElement(L)->GetBgRadius();

  if (!radius) {
    lua_pushnil(L);
  } else {
    lua_pushnumber(L, *radius);
  }

  return 1;
}

int GetPadding(lua_State* L) {
  const auto& padding = CheckElement(L)->GetPadding();

  if (!padding) {
    lua_pushnil(L);
  } else {
    PushPadding(L, *padding);
  }

  return 1;
}

// -----------------------------------------------------------------------------
// Appearance setters
// -----------------------------------------------------------------------------

int SetColor(lua_State* L) {
  CheckElement(L)->SetColor(CheckColor(L, 2));
  return 0;
}

int SetBgColor(lua_State* L) {
  CheckElement(L)->SetBgColor(CheckColor(L, 2));
  return 0;
}

int SetRadius(lua_State* L) {
  CheckElement(L)->SetRadius(CheckFloat(L, 2));
  return 0;
}

int SetBgRadius(lua_State* L) {
  auto* element = CheckElement(L);

  if (lua_isnoneornil(L, 2)) {
    element->SetBgRadius(std::nullopt);
  } else {
    element->SetBgRadius(CheckFloat(L, 2));
  }

  return 0;
}

int SetPadding(lua_State* L) {
  auto* element = CheckElement(L);

  if (lua_isnoneornil(L, 2)) {
    element->SetPadding(std::nullopt);
  } else {
    element->SetPadding(CheckPadding(L, 2));
  }

  return 0;
}

int SetBorderWidth(lua_State* L) {
  UiElement* element = CheckElement(L, 1);

  if (lua_isnoneornil(L, 2)) {
    element->SetBorderWidth(std::nullopt);
    return 0;
  }

  const lua_Number width = luaL_checknumber(L, 2);
  if (width < 0.0) return luaL_argerror(L, 2, "border width must be >= 0");

  element->SetBorderWidth(static_cast<float>(width));
  return 0;
}

int GetBorderWidth(lua_State* L) {
  UiElement* element = CheckElement(L, 1);

  if (auto width = element->GetBorderWidth()) {
    PushLength(L, {*width, UiUnit::Px});
  } else {
    lua_pushnil(L);
  }
  return 1;
}

int SetBorderColor(lua_State* L) {
  CheckElement(L)->SetBorderColor(CheckColor(L, 2));
  return 0;
}

int GetBorderColor(lua_State* L) {
  PushColor(L, CheckElement(L)->GetBorderColor());
  return 1;
}

// -----------------------------------------------------------------------------
// Properties
// -----------------------------------------------------------------------------

int IsVisible(lua_State* L) {
  lua_pushboolean(L, CheckElement(L)->IsVisible());
  return 1;
}

int SetVisible(lua_State* L) {
  const int visible = lua_toboolean(L, 2);
  CheckElement(L)->SetVisible(visible != 0);
  return 0;
}

int GetStateFlags(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(CheckElement(L)->GetStateFlags()));
  return 1;
}

int SetStateFlags(lua_State* L) {
  const auto flags = static_cast<UiStateFlags>(luaL_checkint(L, 2));
  CheckElement(L)->SetStateFlags(flags);
  return 0;
}

int IsState(lua_State* L) {
  const auto flags = static_cast<UiStateFlags>(luaL_checkint(L, 2));
  lua_pushboolean(L, CheckElement(L)->IsState(flags));
  return 1;
}

// -----------------------------------------------------------------------------
// Layout and hit testing
// -----------------------------------------------------------------------------

int Contains(lua_State* L) {
  const glm::vec2 pos = CheckVec2(L, 2);
  lua_pushboolean(L, CheckElement(L)->Contains(pos));
  return 1;
}

int GetChildren(lua_State* L) {
  const auto children = CheckElement(L)->GetChildren();

  lua_createtable(L, static_cast<int>(children.size()), 0);

  int index = 1;

  for (const auto& child : children) {
    PushElement(L, child.get());
    lua_rawseti(L, -2, index++);
  }

  return 1;
}

int ElementEq(lua_State* L) {
  auto** a = static_cast<UiElement**>(lua_touserdata(L, 1));
  auto** b = static_cast<UiElement**>(lua_touserdata(L, 2));
  lua_pushboolean(L, a && b && *a == *b);
  return 1;
}

// в Register: SetFunction(L, -1, "__eq", ElementEq);

// -----------------------------------------------------------------------------
// Checkbox
// -----------------------------------------------------------------------------

int IsChecked(lua_State* L) {
  lua_pushboolean(L, CheckDerivedElement<UiCheckbox>(L)->IsChecked());
  return 1;
}

int SetChecked(lua_State* L) {
  const int checked = lua_toboolean(L, 2);
  CheckDerivedElement<UiCheckbox>(L)->SetChecked(checked != 0);
  return 0;
}

int Toggle(lua_State* L) {
  CheckDerivedElement<UiCheckbox>(L)->Toggle();
  return 0;
}

}  // namespace

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void UiElementBinding::Push(const script::LuaState& state, UiElement* element) {
  PushElement(state.Get(), element);
}

void UiElementBinding::Register(const script::LuaState& state) {
  lua_State* L = state.Get();

  if (luaL_newmetatable(L, kElementMeta.data())) {
    // element.__index = element
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Identity
    SetFunction(L, -1, "GetId", GetId);
    SetFunction(L, -1, "GetParent", GetParent);
    SetFunction(L, -1, "GetRoot", GetRoot);
    SetFunction(L, -1, "GetNodeType", GetNodeType);
    SetFunction(L, -1, "GetChildren", GetChildren);

    // Transform
    SetFunction(L, -1, "GetPos", GetPos);
    SetFunction(L, -1, "SetPos", SetPos);
    SetFunction(L, -1, "GetSize", GetSize);
    SetFunction(L, -1, "SetSize", SetSize);
    SetFunction(L, -1, "GetAnchor", GetAnchor);
    SetFunction(L, -1, "SetAnchor", SetAnchor);
    SetFunction(L, -1, "GetPivot", GetPivot);
    SetFunction(L, -1, "SetPivot", SetPivot);

    // Appearance
    SetFunction(L, -1, "GetColor", GetColor);
    SetFunction(L, -1, "SetColor", SetColor);
    SetFunction(L, -1, "GetBgColor", GetBgColor);
    SetFunction(L, -1, "SetBgColor", SetBgColor);
    SetFunction(L, -1, "GetRadius", GetRadius);
    SetFunction(L, -1, "SetRadius", SetRadius);
    SetFunction(L, -1, "GetBgRadius", GetBgRadius);
    SetFunction(L, -1, "SetBgRadius", SetBgRadius);
    SetFunction(L, -1, "GetPadding", GetPadding);
    SetFunction(L, -1, "SetPadding", SetPadding);
    SetFunction(L, -1, "SetBorderColor", SetBorderColor);
    SetFunction(L, -1, "GetBorderColor", GetBorderColor);
    SetFunction(L, -1, "SetBorderWidth", SetBorderWidth);
    SetFunction(L, -1, "GetBorderColor", GetBorderColor);

    // Properties
    SetFunction(L, -1, "IsVisible", IsVisible);
    SetFunction(L, -1, "SetVisible", SetVisible);
    SetFunction(L, -1, "GetStateFlags", GetStateFlags);
    SetFunction(L, -1, "SetStateFlags", SetStateFlags);
    SetFunction(L, -1, "IsState", IsState);

    // Layout
    SetFunction(L, -1, "Contains", Contains);

    // Eq
    SetFunction(L, -1, "__eq", ElementEq);

    // Checkbox-specific functions
    SetFunction(L, -1, "IsChecked", IsChecked);
    SetFunction(L, -1, "SetChecked", SetChecked);
    SetFunction(L, -1, "Toggle", Toggle);
  }

  lua_pop(L, 1);
}

}  // namespace gfx