#include "settings.hpp"

#include <filesystem>

#include <fmt/format.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#define INFOCH_CONTEXT_LUA "INFOCH_CONTEXT_LUA"

using lua_state = lua_State;

namespace settings {
struct Config_LuaState {
  lua_state *L;

  Config_LuaState() noexcept {
    L = luaL_newstate();
    luaL_openlibs(L);
  }

  Config_LuaState(Config_LuaState const &) = delete;
  Config_LuaState &operator=(Config_LuaState const &) = delete;

  Config_LuaState(Config_LuaState &&lhs) noexcept {
    if (&lhs != this) {
      if (this->L != nullptr) {
        lua_close(this->L);
      }

      this->L = lhs.L;
      lhs.L = nullptr;
    }
  }
  Config_LuaState &operator=(Config_LuaState &&lhs) noexcept {
    if (&lhs != this) {
      if (this->L != nullptr) {
        lua_close(this->L);
      }

      this->L = lhs.L;
      lhs.L = nullptr;
    }

    return *this;
  }

  ~Config_LuaState() noexcept { lua_close(this->L); }
};

static Settings *get_context_settings(lua_state *L) noexcept {
  lua_getfield(L, LUA_REGISTRYINDEX, INFOCH_CONTEXT_LUA);
  auto *ctx = static_cast<Settings *>(lua_touserdata(L, -1));
  lua_pop(L, 1);
  return ctx;
}

static int l_text_set(lua_state *L) noexcept {
  auto *ctx = get_context_settings(L);
  if (ctx == nullptr) {
    return luaL_error(L, "cannot get the context");
  }
  auto d = luaL_checknumber(L, 1);
  if (d < 0 || d > UINT16_MAX) {
    return luaL_argerror(L, 1, "invalid range (must in range: [0, 65535])");
  }
  ctx->text_padding_t = static_cast<std::uint16_t>(d);

  return 0;
}

static int l_image_set(lua_state *L) noexcept {
  auto *ctx = get_context_settings(L);

  if (ctx == nullptr) {
    return luaL_error(L, "cannot get the context");
  }
  size_t path_len = 0;
  const auto *path = luaL_checklstring(L, 1, &path_len);
  if (path_len <= 0) {
    return luaL_argerror(L, 1, "image_set path must not empty");
  }

  auto w = luaL_checknumber(L, 2);
  auto h = luaL_checknumber(L, 3);
  auto pw = luaL_checknumber(L, 4);
  auto ph = luaL_checknumber(L, 5);

  if (w < 0 || w > UINT16_MAX) {
    return luaL_argerror(L, 2, "invalid range (must in range: [0, 65535])");
  }
  if (h < 0 || h > UINT16_MAX) {
    return luaL_argerror(L, 3, "invalid range (must in range: [0, 65535])");
  }
  if (pw < 0 || pw > UINT16_MAX) {
    return luaL_argerror(L, 4, "invalid range (must in range: [0, 65535])");
  }
  if (ph < 0 || ph > UINT16_MAX) {
    return luaL_argerror(L, 5, "invalid range (must in range: [0, 65535])");
  }

  ctx->image.path = std::filesystem::path(std::string_view(path, path_len));
  ctx->image.cell_width = static_cast<std::uint16_t>(w);
  ctx->image.cell_height = static_cast<std::uint16_t>(h);
  ctx->image.padding_width = static_cast<std::uint16_t>(pw);
  ctx->image.padding_height = static_cast<std::uint16_t>(ph);

  return 0;
}

static int l_text_add(lua_state *L) {
  auto *ctx = get_context_settings(L);
  if (ctx == nullptr) {
    return luaL_error(L, "cannot get the context");
  }
  size_t d_len = 0;
  auto const *d = luaL_checklstring(L, 1, &d_len);
  // I don't care about empty or not

  ctx->text_str.append(d, d_len);
  return 0;
}

void run_config(std::filesystem::path const &path, Settings &set) {
  Config_LuaState lua{};

  lua_pushlightuserdata(lua.L, &set);
  lua_setfield(lua.L, LUA_REGISTRYINDEX, INFOCH_CONTEXT_LUA);

  constexpr struct luaL_Reg g_api_funcs[] = {
      {.name = "image_set", .func = l_image_set},
      {.name = "text_set", .func = l_text_set},
      {.name = "text_add", .func = l_text_add},
      {.name = nullptr, .func = nullptr}};

  {
    if (path.empty()) {
      throw std::runtime_error(fmt::format(
          "(run_config) failed to run config: config path is empty: {}",
#ifdef _WIN32
          reinterpret_cast<const char *>(path.u8string().c_str())
#else
          path.c_str()
#endif
              ));

      auto fstat = std::filesystem::status(path);

      if (!std::filesystem::exists(fstat)) {
        throw std::runtime_error(fmt::format(
            "(run_config) failed to run config: config path doesn't exist: {}",
#ifdef _WIN32
            reinterpret_cast<const char *>(path.u8string().c_str())
#else
            path.c_str()
#endif
                ));
      }

      if (std::filesystem::is_directory(path)) {
        throw std::runtime_error(fmt::format(
            "(run_config) failed to run config: config path is a directory: {}",
#ifdef _WIN32
            reinterpret_cast<const char *>(path.u8string().c_str())
#else
            path.c_str()
#endif
                ));
      }
    }
  }

  lua_pushglobaltable(lua.L);
  luaL_setfuncs(lua.L, g_api_funcs, 0);
  lua_pop(lua.L, 1);

  if (luaL_dofile(lua.L,
#ifndef _WIN32
                  path.c_str()
#else
                  reinterpret_cast<const char *>(path.u8string().c_str())
#endif
                      ) != LUA_OK) {
    throw std::runtime_error(
        fmt::format("(run_config) failed to run config: Lua error: {}",
                    lua_tostring(lua.L, -1)));
  }
}
} // namespace settings
