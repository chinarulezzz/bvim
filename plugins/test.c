#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "plugins.h"
#include "data.h"

core_t *core;
state_t *state;
api_t *api;

void (*plugin_error)(int, char*, ...);
void (*plugin_info)(int, char*, ...);
void (*plugin_debug)(int, char*, ...);

/* ---------------------------------------------------------------
 *                 exported commands
 * ---------------------------------------------------------------
 */

struct command cmds[] = {
	{ 35, "test", "do nothing", 1, BVI_HANDLER_EXTERNAL, { .func_name = "plg_command_test" }, 4, 1},
	{ 0, NULL, NULL, 0, 0, { NULL }, 0, 0}
};

/* ---------------------------------------------------------------
 *                 exported  keys
 * ---------------------------------------------------------------
 */


struct key keys[] = {
	{ BVI_CTRL('P'), "Ctrl-P", "", 1, BVI_HANDLER_EXTERNAL, { .func_name = "plg_key_test" }},
	{ 0, NULL, NULL, 0, 0, { NULL }}
};

/* ---------------------------------------------------------------
 *                  exported lua functions
 * ---------------------------------------------------------------
 */

struct luaF_item luaF_list[] = {
	{ 1, "bvi_test", "Do nothing, just testing purposes", BVI_HANDLER_EXTERNAL, { .func_name = "plg_lua_test" }},
	{ 0, NULL, NULL, 0, { NULL }}
};


/* ---------------------------------------------------------------
 *                 plugin registration info
 * ---------------------------------------------------------------
 */

plugin_t plugin_register()
{
	plugin_t plg;

	plg.name = "test";
	plg.author = "Anton Kochkov";
	plg.license = "GPLv2";
	plg.version.major = 0;
	plg.version.minor = 1;
	plg.description = "Just test plugin, for example purposes";
	plg.module = NULL;
	plg.exports.keys = keys;
	plg.exports.cmds = cmds;
	plg.exports.luaF_list = luaF_list;
	return plg;
}

/* ---------------------------------------------------------------
 *                 plugin initialization function
 * ---------------------------------------------------------------
 */

int plugin_init(core_t *bvim_core, state_t *bvim_state, api_t *bvim_api)
{
	plugin_error = bvim_api->error;
	plugin_info = bvim_api->info;
	plugin_debug = bvim_api->debug;
	core = bvim_core;
	state = bvim_state;
	api = bvim_api;
	return 0;
}

/* ---------------------------------------------------------------
 *                 exported command handlers
 * ---------------------------------------------------------------
 */

int plg_command_test(char flags, int c_argc, char** c_argv)
{
	plugin_info(state->mode, "Command from \"%s\" plugin successfully executed!", "test");
	return 0;
}

/* ---------------------------------------------------------------
 *                 exported key handlers
 * ---------------------------------------------------------------
 */

int plg_key_test()
{
	plugin_info(state->mode, "Key, defined in \"%s\" plugin, pressed!", "test");
	return 0;
}

/* ---------------------------------------------------------------
 *                 exported lua functions
 * ---------------------------------------------------------------
 */

int plg_lua_test(lua_State *L)
{
	plugin_info(state->mode, "Lua function from \"%s\" plugin successfully executed", "test");
	lua_pushstring(L, "test function");
	return 1;
}
