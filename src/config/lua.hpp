#ifndef INFOCH_LUA_LUA_H
#define INFOCH_LUA_LUA_H

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace config {
struct LuaLuaState {
  lua_State *L;

  LuaLuaState() noexcept {
    L = luaL_newstate();
    luaL_openlibs(L);
  }

  ~LuaLuaState() noexcept { lua_close(L); }
};
} // namespace config

#endif