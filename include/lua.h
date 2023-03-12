//
//  lua.h
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#ifndef llua_h
#define llua_h
#include "minilua.h"
#include "bitmap.h"
#include "filesystem.h"
#include "maths.h"

void LuaDumpTable(lua_State* L, int table_idx);
int LuaDumpStack(lua_State* L);
void LuaFail(lua_State *L, char *msg, bool die);
int LuaSettings(lua_State *L);
int LuaDelta(lua_State *L);
lua_State* LoadLuaScript(const char *filename);

void LuaCallPreframe(lua_State *L);
void LuaCallFrame(lua_State *L, unsigned char *heightmap, int w, int h);

#endif /* llua_h */
