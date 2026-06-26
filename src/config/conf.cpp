#include "conf.hpp"

#include <filesystem>
#include <string>

#include <fmt/core.h>

#include "lua.hpp"

using lua_state = lua_State;

namespace config {
constinit Image g_image{};
constinit bool g_image_init{false};

static int l_image_set(lua_state *L) {
  g_image.path = std::filesystem::path(luaL_checkstring(L, 1));
  g_image.cell_w = luaL_checknumber(L, 2);
  g_image.cell_h = luaL_checknumber(L, 3);
  g_image.padding_w = luaL_checknumber(L, 4);
  g_image.padding_h = luaL_checknumber(L, 5);

  return 0;
}

constinit Text g_text{};

static int l_text_set(lua_state *L) {
  g_text.padding_top = luaL_checknumber(L, 1);

  return 0;
}

static int l_text_add(lua_state *L) {
  g_text.str.append(luaL_checkstring(L, 1));

  return 0;
}

constexpr struct luaL_Reg g_api_funcs[] = {
    {.name = "image_set", .func = l_image_set},
    {.name = "text_set", .func = l_text_set},
    {.name = "text_add", .func = l_text_add},
    {.name = nullptr, .func = nullptr}};

void run_config(std::filesystem::path const &config) {
  {
    if (config.empty()) {
      throw std::runtime_error(fmt::format(
          "(run_config) failed to run config: config path is empty: {}",
#ifdef _WIN32
          reinterpret_cast<const char *>(config.u8string().c_str())
#else
          config.c_str()
#endif
              ));

      auto fstat = std::filesystem::status(config);

      if (!std::filesystem::exists(fstat)) {
        throw std::runtime_error(fmt::format(
            "(run_config) failed to run config: config path doesn't exist: {}",
#ifdef _WIN32
            reinterpret_cast<const char *>(config.u8string().c_str())
#else
            config.c_str()
#endif
                ));
      }

      if (std::filesystem::is_directory(fstat)) {
        throw std::runtime_error(fmt::format(
            "(run_config) failed to run config: config path is a directory: {}",
#ifdef _WIN32
            reinterpret_cast<const char *>(config.u8string().c_str())
#else
            config.c_str()
#endif
                ));
      }
    }

    LuaLuaState state{};

    lua_newtable(state.L);
    luaL_setfuncs(state.L, g_api_funcs, 0);
    lua_setglobal(state.L, "g");

    if (luaL_dofile(state.L,
#ifdef _WIN32
                    reinterpret_cast<const char *>(config.u8string().c_str()
#else
                    config.c_str()
#endif
                                                       ))) {
      throw std::runtime_error(
          fmt::format("(run_config) failed to run config: Lua error: {}",
                      luaL_checkstring(state.L, -1)));
    }
  }
}
} // namespace config