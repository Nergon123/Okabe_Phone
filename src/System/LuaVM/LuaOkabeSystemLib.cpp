#include "lua/lua.hpp"
#include "LuaOkabeLib.h"
#include <GlobalVariables.h>

extern "C" {
	static int system_ischarging(lua_State *L)
	{
		int b = hw->isCharging();
		lua_pushboolean(L, b);
		
		return 1;
	}

	static int system_get_battery_charge(lua_State *L)
	{
		int l = hw->getBatteryCharge();

		lua_pushinteger(L, l);
		return 1;
	}

	static int system_get_wifi_strength(lua_State *L)
	{
		int l = hw->getWifiStrength();

		lua_pushinteger(L, l);
		return 1;
	}

	static int system_get_key_input(lua_State *L)
	{
		int i = hw->getKeyInput();

		lua_pushinteger(L, i);
		return 1;
	}

	static int system_get_char_input(lua_State *L)
	{
		char c = hw->getCharInput();

		lua_pushinteger(L, c);
		return 1;
	}

	static int system_millis(lua_State *L)
	{
		ulong ms = hw->millis();

		lua_pushinteger(L, ms);
		return 1;
	}

	static int system_delay(lua_State *L)
	{
		lua_Integer l = luaL_checklong(L, 1);

		if (l > 0) {
			hw->delay(l);
		}
		return 1;
	}
		
	static const struct luaL_Reg systemlib[] = {
 		{"ischarging", system_ischarging},
 		{"get_battery_charge", system_get_battery_charge},
 		{"get_wifi_strength", system_get_wifi_strength},
 		{"get_key_input", system_get_key_input},
 		{"get_char_input", system_get_char_input},
 		{"millis", system_millis},
 		{"delay", system_delay},
 		{NULL, NULL}
 	};
 	
 	int luaopen_system(lua_State *L) 
 	{
 		luaL_register(L, OKABE_SYSTEMLIBNAME, systemlib);
 		return 1;
 	} 	
}
