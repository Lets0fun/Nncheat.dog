#pragma once

#pragma comment(lib, "lua51.lib")
#pragma comment(lib, "luajit.lib")

#include "CLuaHook.h"
#include "menu_item.h"
#include "..\utils\singleton.h"
#include <filesystem>

enum MENUITEMTYPE {
	MENUITEM_CHECKBOX = 0,
	MENUITEM_SLIDERINT,
	MENUITEM_SLIDERFLOAT,
	MENUITEM_KEYBIND,
	MENUITEM_TEXT,
	MENUITEM_LISTBOX,
	MENUITEM_SINGLESELECT,
	MENUITEM_MULTISELECT,
	MENUITEM_COLORPICKER,
	MENUITEM_BUTTON,
	MENUITEM_TEXTBOX
};

struct MenuItem_t {
	MENUITEMTYPE type;
	int script = -1;
	std::string label = "";
	std::string key = "";

	static char buffer[124];

	bool is_visible = true;

	// defaults
	bool b_default = false;
	int i_default = 0;
	float f_default = 0.f;
	float c_default[4] = { 1.f, 1.f, 1.f, 1.f };
	std::map<int, bool> m_default = {};

	// keybinds
	bool allow_style_change = true;

	// singleselect & multiselect
	std::vector<const char*> items = {};

	std::vector<std::string> listitems = {};

	// slider_int
	int i_min = 0;
	int i_max = 100;

	// slider_float
	float f_min = 0.f;
	float f_max = 1.f;

	// sliders
	std::string format = "%d";

	// callbacks
	sol::function callback;
};

class c_lua : public singleton <c_lua>
{
public:
	void initialize();
	void refresh_scripts();

	void refresh_scripts_startup();

	void download_cloud_scripts();

	void load_script(int id);
	void unload_script(int id);

	void reload_all_scripts();
	void unload_all_scripts();


	bool is_cloud_lua(std::string name);

	int get_script_id(std::string name);
	int get_script_id_by_path(std::string path);

	bool luatabactive = false;

	std::vector <bool> loaded;
	std::vector <bool> startup;
	std::vector <std::string> scripts;
	std::vector<std::string> scripts_names;
	std::vector<std::string> scripts_cloud;
	std::vector <std::vector <std::pair <std::string, menu_item>>> items;
	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> menu_items = {};
	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> ida_items = {};
	std::unordered_map <int, std::unordered_map <std::string, sol::protected_function>> events;
	std::vector <std::filesystem::path> pathes;
	c_lua_hookManager hooks;
private:
	std::string get_script_path(std::string name);
	std::string get_script_path(int id);


};