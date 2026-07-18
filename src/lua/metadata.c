/*
   This file is part of darktable,
   Copyright (C) 2026 darktable developers.

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

#include "common/metadata.h"
#include "lua/metadata.h"
#include "lua/glist.h"
#include "lua/types.h"

static int exists(lua_State *L)
{
    gboolean result = false;
    dt_metadata_t *md;

    const char *tagname = luaL_checkstring(L, 1);

    md = dt_metadata_get_metadata_by_tagname(tagname);

    if (md)
        result = true;

    lua_pushboolean(L, result);
    return 1;
}

int dt_lua_init_metadata(lua_State *L)
{
    dt_lua_push_darktable_lib(L);
    dt_lua_goto_subtable(L, "metadata");

    lua_pushcfunction(L, exists);
    lua_setfield(L, -2, "exists");

    lua_pop(L, 1);

    return 0;
}
