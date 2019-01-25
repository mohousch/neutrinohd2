/*
	$Id: neutrino_lua.cpp 25.01.2019 mohousch Exp $

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <config.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <interfaces/lua/neutrino_lua.h>

#include <system/helpers.h>


neutrinoLua::neutrinoLua()
{
	// Create the intepreter object
	lua = luaL_newstate();

	// register standard + custom functions
	//LuaInstRegisterFunctions(lua);
}

neutrinoLua::~neutrinoLua()
{
	if (lua != NULL)
	{
		lua_close(lua);
		lua = NULL;
	}
}

// Run the given script.
void neutrinoLua::runScript(const char *fileName, std::vector<std::string> *argv, std::string *result_code, std::string *result_string, std::string *error_string)
{
	luaL_dofile(lua, fileName);

#if 0
	// run the script 
	int status = luaL_loadfile(lua, fileName);

	if (status) 
	{
		bool isString = lua_isstring(lua, -1);
		const char *null = "NULL";

		fprintf(stderr, "[CLuaInstance::%s] Can't load file: %s\n", __func__, isString ? lua_tostring(lua, -1):null);

		//DisplayErrorMessage(isString ? lua_tostring(lua, -1):null, "Lua Script Error:");

		if (error_string)
			*error_string = std::string(lua_tostring(lua, -1));
		return;
	}

	int argvSize = 1;
	int n = 0;
	//set_lua_variables(lua);

	if (argv && (!argv->empty()))
		argvSize += argv->size();

	lua_createtable(lua, argvSize, 0);

	// arg0 is scriptname
	lua_pushstring(lua, fileName);
	lua_rawseti(lua, -2, n++);

	if (argv && (!argv->empty())) 
	{
		for(std::vector<std::string>::iterator it = argv->begin(); it != argv->end(); ++it) {
			lua_pushstring(lua, it->c_str());
			lua_rawseti(lua, -2, n++);
		}
	}

	lua_setglobal(lua, "arg");

	status = lua_pcall(lua, 0, LUA_MULTRET, 0);

	if (result_code)
		*result_code = to_string(status);
	if (result_string && lua_isstring(lua, -1))
		*result_string = std::string(lua_tostring(lua, -1));
	if (status)
	{
		bool isString = lua_isstring(lua,-1);
		const char *null = "NULL";
		fprintf(stderr, "[CLuaInstance::%s] error in script: %s\n", __func__, isString ? lua_tostring(lua, -1):null);
		//DisplayErrorMessage(isString ? lua_tostring(lua, -1):null, "Lua Script Error:");
		if (error_string)
			*error_string = std::string(lua_tostring(lua, -1));
	}
#endif
}

// Example: runScript(fileName, "Arg1", "Arg2", "Arg3", ..., NULL);
//	Type of all parameters: const char*
//	The last parameter to NULL is imperative.
void neutrinoLua::runScript(const char *fileName, const char *arg0, ...)
{
	int i = 0;
	std::vector<std::string> args;
	args.push_back(arg0);
	va_list list;
	va_start(list, arg0);
	const char* temp = va_arg(list, const char*);

	while (temp != NULL) 
	{
		if (i >= 64) 
		{
			//fprintf(stderr, "CLuaInstance::runScript: too many arguments!\n");
			args.clear();
			va_end(list);
			return;
		}
		args.push_back(temp);
		temp = va_arg(list, const char*);
		i++;
	}
	va_end(list);
	runScript(fileName, &args);
	args.clear();
}


