/*
   This file is part of darktable,
   Copyright (C) 2015-2020 darktable developers.

   darktable is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darktable is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darktable.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "lua/lua.h"

// DarkTableNext currently ships English source text only. Keep the Lua API
// stable for scripts while making every translation lookup deterministic.

static int lua_gettext(lua_State *L)
{
    const char *msgid = luaL_checkstring(L, 1);
    lua_pushstring(L, msgid);
    return 1;
}

static int lua_dgettext(lua_State *L)
{
    const char *msgid = luaL_checkstring(L, 2);
    lua_pushstring(L, msgid);
    return 1;
}

static int lua_ngettext(lua_State *L)
{
    const char *msgid = luaL_checkstring(L, 1);
    const char *msgid_plural = luaL_checkstring(L, 2);
    int n = luaL_checkinteger(L, 3);
    lua_pushstring(L, n == 1 ? msgid : msgid_plural);
    return 1;
}

static int lua_dngettext(lua_State *L)
{
    const char *msgid = luaL_checkstring(L, 2);
    const char *msgid_plural = luaL_checkstring(L, 3);
    int n = luaL_checkinteger(L, 4);
    lua_pushstring(L, n == 1 ? msgid : msgid_plural);
    return 1;
}

static int lua_bindtextdomain(lua_State *L)
{
    luaL_checkstring(L, 1);
    luaL_checkstring(L, 2);
    return 0;
}

int dt_lua_init_gettext(lua_State *L)
{
    dt_lua_push_darktable_lib(L);
    dt_lua_goto_subtable(L, "gettext");

    lua_pushcfunction(L, lua_gettext);
    lua_setfield(L, -2, "gettext");
    lua_pushcfunction(L, lua_dgettext);
    lua_setfield(L, -2, "dgettext");
    lua_pushcfunction(L, lua_ngettext);
    lua_setfield(L, -2, "ngettext");
    lua_pushcfunction(L, lua_dngettext);
    lua_setfield(L, -2, "dngettext");
    lua_pushcfunction(L, lua_bindtextdomain);
    lua_setfield(L, -2, "bindtextdomain");

    lua_pop(L, 1);
    return 0;
}
