#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include "../Menu/Menu.h"


#include <filesystem>
#include <iterator>
#include <fstream>

#include <unordered_map>
#include <array>
#include <algorithm>

#pragma comment(lib,"urlmon.lib")

class c_config : public singleton2<c_config> {
public:
	std::unordered_map<std::string, bool> LuaVisible;
	std::unordered_map<std::string, bool> b;
	std::unordered_map<std::string, int> i;
	std::unordered_map<std::string, float> f;
	std::unordered_map<std::string, std::string> s;
	std::unordered_map<std::string, int[4]> c;
	std::unordered_map<std::string, std::vector<int>> v;
	std::unordered_map<std::string, bool> PlaceHolder;
	char Config_input_text[128];
	char Skins_input_text[128];
	std::unordered_map<std::string, std::unordered_map<int, bool>> m;
	std::vector<std::filesystem::path> configs_path;
	std::vector<std::string> configs;

	std::vector<const char*> presets = { "Rage", "Legit", "HvH", "Body-aim", "Secret", "Headshot", "Test" };

	void remove(size_t id) noexcept;

	void export_to_clipboard();
	void import();

	bool InfoBar;

	bool is_key_down(int key);

	bool is_key_up(int key);

	bool auto_check(int key, int style);

	void save();
	void refresh_configs();
	int get_config_id(std::string name);
	void save_keys();
	void save_menucolor();
	void load();
	void setup_item(std::vector<int>* pointer, int size);
	void load_defaults();
	void load_keys();

};

extern IDirect3DTexture9* iconTexture;
