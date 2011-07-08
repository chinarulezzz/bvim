/* Include the Lua API header files. */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bvi.h"
#include "set.h"
#include "bscript.h"

lua_State *lstate;

/* Save current buffer into file */
/* lua: save(filename, [start, end, flags]) */
static int bvi_save(lua_State *L)
{
	/* save(filename, start, end, flags); */
	return 0;
}

/* Load file into current buffer */
/* lua: load(filename, [mode]) */
static int bvi_load(lua_State *L)
{
	char* filename;
	int mode;
	int n = lua_gettop(L);
	
	if (n == 1)
	{
		filename = (char*) lua_tostring(L, 1);
		load(filename);
	}
	else if (n == 2)
	{
		filename = (char*) lua_tostring(L, 1);
		mode = (int) lua_tonumber(L, 2);
		if (mode == 1)
		{
			addfile(filename);
		}
		else
		{
			load(filename);
		}
	}
	return 0;
}

/* Get file path for current buffer */
static int bvi_file(lua_State *L)
{
	char *filename = "dummy";
	lua_pushstring(L, filename);
	return 1;
}

/* Execute bvi cmd */
/* lua: exec(command) */
static int bvi_exec(lua_State *L)
{
	char* cmdline;
	if (lua_gettop(L) != 0)
	{
		cmdline = (char*) lua_tostring(L, -1);
		docmdline(cmdline);
	}
	return 0;
}

/* Display error */
/* lua: display_error(message) */
static int bvi_display_error(lua_State *L)
{
	char* message;
	if (lua_gettop(L) != 0)
	{
		message = (char*) lua_tostring(L, -1);
		emsg(message);
	}
	return 0;
}

/* Display message in the status line */
/* lua: status_line_msg(message) */
static int bvi_status_line_msg(lua_State *L)
{
	char* message;
	if (lua_gettop(L) != 0)
	{
		message = (char*) lua_tostring(L, -1);
		msg(message);
	}
	return 0;
}

/* Display message in the window */
/* lua: msg_window(message) */
static int bvi_msg_window(lua_State *L)
{
	char* message;
	int height, width;
	int n = lua_gettop(L);
	if (n == 1)
	{
		message = (char*) lua_tostring(L, -1);
		wmsg(message, 3, strlen(message) + 4);
	}
	else if (n == 3)
	{
		message = (char*) lua_tostring(L, 1);
		height = (int) lua_tonumber(L, 2);
		width = (int) lua_tonumber(L, 3);
		wmsg(message, height, width);
	}
	return 0;
}

/* Undo */
static int bvi_undo(lua_State *L)
{
	do_undo();
	return 0;
}

/* Redo */
static int bvi_redo(lua_State *L)
{
	return 0;
}

/* Set any bvi parameter (analog of :set param cmd) */
static int bvi_set(lua_State *L)
{
	return 0;
}

static int bvi_scrolldown(lua_State *L)
{
	int lines;
	if (lua_gettop(L) > 0)
	{
		lines = (int) lua_tonumber(L, 1);
	}
	else lines = 1;
	scrolldown(lines);
	return 0;
}

static int bvi_scrollup(lua_State *L)
{
	int lines;
	if (lua_gettop(L) > 0)
	{
		lines = (int) lua_tonumber(L, 1);
	}
	else lines = 1;
	scrollup(lines);
	return 0;
}

/* Insert count of bytes at position */
static int bvi_insert(lua_State *L)
{
	return 0;
}

/* Overwrite count of bytes with custom data */
static int bvi_overwrite(lua_State *L)
{
	return 0;
}

/* Remove count of bytes from position */
static int bvi_remove(lua_State *L)
{
	return 0;
}

/* Get current cursor position */
static int bvi_cursor(lua_State *L)
{
	return 0;
}

/* Display arbitrary address on the screen */
static int bvi_setpage(lua_State *L)
{
	int address;
	if (lua_gettop(L) > 0)
	{
		address = (int) lua_tonumber(L, 1);
	}
	else address = 0;
	setpage(address);
	return 0;
}

/* -------------------------------------------------------------------------------------
 *
 *  Initialization of Lua scripting support and loading plugins ...
 *
 * -------------------------------------------------------------------------------------
 */

void bvi_lua_init()
{
	struct luaL_reg bvi_methods[] = {
		{ "save", bvi_save },
		{ "load", bvi_load },
		{ "file", bvi_file },
		{ "exec", bvi_exec },
		{ "display_error", bvi_display_error },
		{ "display_status_msg", bvi_status_line_msg },
		{ "msg_window", bvi_msg_window },
		{ "set", bvi_set },
		{ "undo", bvi_undo },
		{ "redo", bvi_redo },
		{ "insert", bvi_insert },
		{ "overwrite", bvi_overwrite },
		{ "remove", bvi_remove },
		{ "cursor", bvi_cursor },
		{ "scrolldown", bvi_scrolldown },
		{ "scrollup", bvi_scrollup },
		{ "setpage", bvi_setpage },
		{ NULL, NULL }
	};
	lstate = lua_open();
	luaL_openlibs(lstate);
	luaL_register(lstate, "bvi", bvi_methods);
}

int bvi_run_lua_script(char* name)
{
	char filename[256];
	int err;
	filename[0] = '\0';
	strcat(filename, "plugins/");
	strcat(filename, name);
	strcat(filename, ".lua");
	err = luaL_loadfile(lstate, filename);
	if (err)
	{
		emsg("Error: cant open lua script file!");
		lua_pop(lstate, 1);
	}
	else
	{
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
	}
	return 0;
}

int bvi_run_lua_string(char* string)
{
	int err = luaL_loadstring(lstate, string);
	if (err)
	{
		emsg("Error in lua script!");
		lua_pop(lstate, 1);
	}
	else
	{
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
	}
	return 0;
}

void bvi_lua_finish()
{
	lua_close(lstate);
}
