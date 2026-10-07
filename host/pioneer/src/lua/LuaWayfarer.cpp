// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LuaWayfarer.h"

#include "Lua.h"
#include "LuaUtils.h"
#include "wayfarer/WayfarerHost.h"

static int l_wayfarer_start_stage0(lua_State *l)
{
	wayfarer::WayfarerHost::Get().StartStage0();
	lua_pushboolean(l, 1);
	return 1;
}

static int l_wayfarer_shutdown(lua_State *l)
{
	wayfarer::WayfarerHost::Get().Shutdown();
	return 0;
}

static int l_wayfarer_is_active(lua_State *l)
{
	lua_pushboolean(l, wayfarer::WayfarerHost::Get().IsActive());
	return 1;
}

static int l_wayfarer_debug_panel(lua_State *l)
{
	const std::string text = wayfarer::WayfarerHost::Get().DebugPanelText();
	lua_pushlstring(l, text.data(), text.size());
	return 1;
}

static int l_wayfarer_accept_contract(lua_State *l)
{
	lua_pushboolean(l, wayfarer::WayfarerHost::Get().AcceptCurrentContract());
	return 1;
}

static int l_wayfarer_deliver_contract(lua_State *l)
{
	lua_pushboolean(l, wayfarer::WayfarerHost::Get().DeliverCurrentContract());
	return 1;
}

static int l_wayfarer_player_travel_to(lua_State *l)
{
	const char *key = luaL_checkstring(l, 1);
	lua_pushboolean(l, wayfarer::WayfarerHost::Get().PlayerTravelTo(key));
	return 1;
}

static int l_wayfarer_npc_count(lua_State *l)
{
	lua_pushinteger(l, wayfarer::WayfarerHost::Get().NpcCount());
	return 1;
}

static int l_wayfarer_npc_name(lua_State *l)
{
	const int index = luaL_checkinteger(l, 1);
	const std::string s = wayfarer::WayfarerHost::Get().NpcName(index);
	lua_pushlstring(l, s.data(), s.size());
	return 1;
}

static int l_wayfarer_npc_status(lua_State *l)
{
	const int index = luaL_checkinteger(l, 1);
	const std::string s = wayfarer::WayfarerHost::Get().NpcStatus(index);
	lua_pushlstring(l, s.data(), s.size());
	return 1;
}

static int l_wayfarer_npc_destination(lua_State *l)
{
	const int index = luaL_checkinteger(l, 1);
	const std::string s = wayfarer::WayfarerHost::Get().NpcDestinationName(index);
	lua_pushlstring(l, s.data(), s.size());
	return 1;
}

static int l_wayfarer_npc_progress(lua_State *l)
{
	const int index = luaL_checkinteger(l, 1);
	lua_pushnumber(l, wayfarer::WayfarerHost::Get().NpcProgress(index));
	return 1;
}

static int l_wayfarer_npc_cargo(lua_State *l)
{
	const int index = luaL_checkinteger(l, 1);
	const std::string s = wayfarer::WayfarerHost::Get().NpcCargoSummary(index);
	lua_pushlstring(l, s.data(), s.size());
	return 1;
}

void LuaWayfarer::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg methods[] = {
		{ "StartStage0", l_wayfarer_start_stage0 },
		{ "Shutdown", l_wayfarer_shutdown },
		{ "IsActive", l_wayfarer_is_active },
		{ "DebugPanel", l_wayfarer_debug_panel },
		{ "AcceptContract", l_wayfarer_accept_contract },
		{ "DeliverContract", l_wayfarer_deliver_contract },
		{ "PlayerTravelTo", l_wayfarer_player_travel_to },
		{ "NpcCount", l_wayfarer_npc_count },
		{ "NpcName", l_wayfarer_npc_name },
		{ "NpcStatus", l_wayfarer_npc_status },
		{ "NpcDestination", l_wayfarer_npc_destination },
		{ "NpcProgress", l_wayfarer_npc_progress },
		{ "NpcCargo", l_wayfarer_npc_cargo },
		{ nullptr, nullptr }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	luaL_newlib(l, methods);
	lua_setfield(l, -2, "Wayfarer");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
