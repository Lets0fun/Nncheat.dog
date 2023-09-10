#include "Config.h"
#include <Windows.h>
#include <string>
#include <memory>

#include <filesystem>
#include <fstream>
#include "../configs/base64.h"

#include "..\cheats\misc\logs.h"

#pragma warning(disable: 4244)
#pragma warning(disable: 4800)
#pragma warning(disable: 4018)
#pragma warning(disable: 4715)
#pragma warning(disable: 4996)


std::string color_to_string(int col[4]) {
	return std::to_string((int)(col[0])) + "," + std::to_string((int)(col[1])) + "," + std::to_string((int)(col[2])) + "," + std::to_string((int)(col[3]));
}

int* string_to_color(std::string s) {
	static auto split = [](std::string str, const char* del) -> std::vector<std::string>
	{
		char* pTempStr = _strdup(str.c_str());
		char* pWord = strtok(pTempStr, del);
		std::vector<std::string> dest;

		while (pWord != NULL)
		{
			dest.push_back(pWord);
			pWord = strtok(NULL, del);
		}

		free(pTempStr);

		return dest;
	};

	std::vector<std::string> col = split(s, ",");
	return new int[4]{
		(int)std::stoi(col.at(0)),
		(int)std::stoi(col.at(1)),
		(int)std::stoi(col.at(2)),
		(int)std::stoi(col.at(3))
	};
}

void c_config::remove(size_t id) noexcept
{
	if (configs.empty()) {
		return;
	}
	else {
		std::error_code ec;
		std::filesystem::remove(configs_path[id], ec);
		configs.erase(configs.cbegin() + id);
		configs_path.erase(configs_path.cbegin() + id);
	}
}

bool is_number(const std::string& s)
{
	return(strspn(s.c_str(), "-.0123456789") == s.size());
}

void setcolor(int col[4], int r, int g, int b, int a) {
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = a;
}


auto read_file(std::string_view path) -> std::string {
	constexpr auto read_size = std::size_t(4096);
	auto stream = std::ifstream(path.data());
	stream.exceptions(std::ios_base::badbit);
	auto out = std::string();
	auto buf = std::string(read_size, '\0');
	while (stream.read(&buf[0], read_size)) {
		out.append(buf, 0, stream.gcount());
	}
	out.append(buf, 0, stream.gcount());
	return out;
}

void c_config::import() {

	if (is_number(std::to_string(c_config::get()->i["_preset"])) == false) {
		return;
	}
	else {

		HANDLE clip;
		if (OpenClipboard(NULL))
			clip = GetClipboardData(CF_TEXT);
		std::string text = (char*)clip;

		CloseClipboard();


		std::string cfg;

		Base64::decode(text, &cfg);
		//eventlogs::get().add(cfg, false);


		std::string config_name = Config_input_text + std::string(".dog");

		CreateFileA(("c:\\gs\\cfg\\" + config_name).c_str(), 0,0,0,0,0,0);
		std::ofstream fs("c:\\gs\\cfg\\" + config_name);
		fs << cfg;
		fs.close();

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		
		refresh_configs();
		load();
	}

	return;
}

void c_config::export_to_clipboard() {

	if (is_number(std::to_string(c_config::get()->i["_preset"])) == false) {
		return;
	}
	else {


		
		//get config path
		std::filesystem::path config_path = configs_path[c_config::get()->i["_preset"]];
		

		//eventlogs::get().add(config_path.filename().string(), true);

		std::string str;

		//std::ifstream file("C:\\gs\\cfg\\"+config_path.filename().string());
		str = read_file("C:\\gs\\cfg\\" + config_path.filename().string());
		//read_file("C:\\gs\\cfg\\" + config_path.filename().string());


		std::string encoded_cfg;
		Base64::encode(str, &encoded_cfg);

		//eventlogs::get().add(str, true);

		const size_t len = strlen(encoded_cfg.c_str()) + 1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), encoded_cfg.c_str(), len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();

	}

	return;
}

bool c_config::is_key_down(int key) {
	return HIWORD(GetKeyState(key));
}

bool c_config::is_key_up(int key) {
	return !HIWORD(GetKeyState(key));
}

bool c_config::auto_check(int key, int style) {
	switch (style) {
		case 0:
			return true;
		case 1:
			return this->is_key_down(key);
		case 2:
			return LOWORD(GetKeyState(key));
		case 3:
			return this->is_key_up(key);
		default:
			return true;
	}
}

void c_config::save() {
	char file_path[MAX_PATH] = { 0 };

	sprintf(file_path, "C:/gs/cfg/%s.dog", (std::string(Config_input_text)).c_str());

	for (auto e : b) {
		if (!std::string(e.first).find("_")) continue;
		char buffer[8] = { 0 }; _itoa(e.second, buffer, 10);
		WritePrivateProfileStringA("b", e.first.c_str(), std::string(buffer).c_str(), file_path);
	}

	for (auto e : i) {
		if (!std::string(e.first).find("_")) continue;
		char buffer[32] = { 0 }; _itoa(e.second, buffer, 10);
		WritePrivateProfileStringA("i", e.first.c_str(), std::string(buffer).c_str(), file_path);
	}

	for (auto e : f) {
		if (!std::string(e.first).find("_")) continue;
		char buffer[64] = { 0 }; sprintf(buffer, "%f", e.second);
		WritePrivateProfileStringA("f", e.first.c_str(), std::string(buffer).c_str(), file_path);
	}

	for (auto e : c) {
		if (!std::string(e.first).find("_")) continue;
		WritePrivateProfileStringA("c", e.first.c_str(), color_to_string(e.second).c_str(), file_path);
	}

	for (auto e : m) {
		if (!std::string(e.first).find("_")) continue;

		std::string vs = "";
		for (auto v : e.second)
			vs += std::to_string(v.first) + ":" + std::to_string(v.second) + "|";

		WritePrivateProfileStringA("m", e.first.c_str(), vs.c_str(), file_path);
	}


	this->save_keys();
}

void c_config::refresh_configs() {

	configs_path.clear();
	configs.clear();

	for (auto& entry : std::filesystem::directory_iterator(("C:\\gs\\cfg"))) {
		if (entry.path().extension() == (".dog")) {
			auto path = entry.path();
			auto path_text = entry.path().string();

			auto filename = path.filename().string();

			if (filename == "csgo_keys.dog") {

			}
			else {
				configs_path.push_back(path);
				configs.push_back(filename);
			}
		}
	}
}

int c_config::get_config_id(std::string name) {
	for (int i = 0; i < this->configs.size(); i++) {
		if (this->configs.at(i) == name)
			return i;
	}

	return -1;
}

void c_config::load() {
	if (configs.empty()) {
		return;
	}
	this->load_defaults();

	char file_path[MAX_PATH] = { 0 };
	sprintf(file_path, "C:/gs/cfg/%s", (configs.at(i["_preset"]).c_str()));

	char b_buffer[65536], i_buffer[65536], f_buffer[65536], c_buffer[65536], m_buffer[65536] = { 0 };

	int b_read = GetPrivateProfileSectionA("b", b_buffer, 65536, file_path);
	int i_read = GetPrivateProfileSectionA("i", i_buffer, 65536, file_path);
	int f_read = GetPrivateProfileSectionA("f", f_buffer, 65536, file_path);
	int c_read = GetPrivateProfileSectionA("c", c_buffer, 65536, file_path);
	int m_read = GetPrivateProfileSectionA("m", m_buffer, 65536, file_path);

	if ((0 < b_read) && ((65536 - 2) > b_read)) {
		const char* pSubstr = b_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				b[name] = atoi(value);
			}

			pSubstr += (substrLen + 1);
		}
	}

	if ((0 < i_read) && ((65536 - 2) > i_read)) {
		const char* pSubstr = i_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				i[name] = atoi(value);
			}

			pSubstr += (substrLen + 1);
		}
	}

	if ((0 < f_read) && ((65536 - 2) > f_read)) {
		const char* pSubstr = f_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				f[name] = atof(value);
			}

			pSubstr += (substrLen + 1);
		}
	}

	if ((0 < c_read) && ((65536 - 2) > c_read)) {
		const char* pSubstr = c_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				auto col = string_to_color(value);
				c[name][0] = col[0];
				c[name][1] = col[1];
				c[name][2] = col[2];
				c[name][3] = col[3];
			}

			pSubstr += (substrLen + 1);
		}
	}

	static auto split = [](std::string str, const char* del) -> std::vector<std::string>
	{
		char* pTempStr = _strdup(str.c_str());
		char* pWord = strtok(pTempStr, del);
		std::vector<std::string> dest;

		while (pWord != NULL)
		{
			dest.push_back(pWord);
			pWord = strtok(NULL, del);
		}

		free(pTempStr);

		return dest;
	};

	if ((0 < m_read) && ((65536 - 2) > m_read)) {
		const char* pSubstr = m_buffer;

		while ('\0' != *pSubstr) {
			size_t substrLen = strlen(pSubstr);

			const char* pos = strchr(pSubstr, '=');
			if (NULL != pos) {
				char name[256] = "";
				char value[256] = "";

				strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
				strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

				std::vector<std::string> kvpa = split(value, "|");
				std::unordered_map<int, bool> vl = {};
				for (auto kvp : kvpa) {
					if (kvp == "")
						continue; // ��� ������

					std::vector<std::string> kv = split(kvp, ":");
					vl[std::stoi(kv.at(0))] = std::stoi(kv.at(1));
				}

				m[name] = vl;
			}

			pSubstr += (substrLen + 1);
		}
	}

	// ���

	this->load_keys();
}

void c_config::setup_item(std::vector< int >* pointer, int size)
{
	pointer->clear();

	for (int i = 0; i < size; i++)
		pointer->push_back(FALSE);
}

void c_config::load_defaults() {
	int _preset = this->i["_preset"];

	b = std::unordered_map<std::string, bool>();
	i = std::unordered_map<std::string, int>();
	f = std::unordered_map<std::string, float>();
	c = std::unordered_map<std::string, int[4]>();
	s = std::unordered_map<std::string, std::string>();

	i["_preset"] = _preset;



	setup_item(&this->v["rage_hitbox"], 9);

	//			D3DCOLOR_RGBA(153, 225, 1, g.MenuAlpha)
	c["menucolor"][0] = 153;
	c["menucolor"][1] = 225;
	c["menucolor"][2] = 1;
	c["menucolor"][3] = 255;
	
	i["rage_key_enabled_st"] = 1;

	c["grentrajcoll"][0] = 255;
	c["grentrajcoll"][1] = 50;
	c["grentrajcoll"][2] = 1;
	c["grentrajcoll"][3] = 255;

	c["grenade_hitcolor"][0] = 153;
	c["grenade_hitcolor"][1] = 225;
	c["grenade_hitcolor"][2] = 1;
	c["grenade_hitcolor"][3] = 255;

	v["rage_hitbox"][0] = true;

	c["misc_prefixcolor"][0] = 0.678f;
	c["misc_prefixcolor"][1] = 0.956f;
	c["misc_prefixcolor"][2] = 0.019f;
	c["misc_prefixcolor"][3] = 1.00f;

	c["vis_boundingbox_color"][0] = 1.f;
	c["vis_boundingbox_color"][1] = 1.f;
	c["vis_boundingbox_color"][2] = 1.f;
	c["vis_boundingbox_color"][3] = 0.5f;

	c["vis_name_color"][0] = 1.f;
	c["vis_name_color"][1] = 1.f;
	c["vis_name_color"][2] = 1.f;
	c["vis_name_color"][3] = 1.f;

	c["vis_ammo_color"][0] = 1.f;
	c["vis_ammo_color"][1] = 1.f;
	c["vis_ammo_color"][2] = 1.f;
	c["vis_ammo_color"][3] = 1.f;

	c["vis_outoffovarrows_color"][0] = 1.f;
	c["vis_outoffovarrows_color"][1] = 1.f;
	c["vis_outoffovarrows_color"][2] = 1.f;
	c["vis_outoffovarrows_color"][3] = 1.f;

	c["vis_glow_color"][0] = 1.f;
	c["vis_glow_color"][1] = 0.f;
	c["vis_glow_color"][2] = 170 / 255.f;
	c["vis_glow_color"][3] = 0.5f;

	c["vis_glowself_color"][0] = 1.f;
	c["vis_glowself_color"][1] = 1.f;
	c["vis_glowself_color"][2] = 1.f;
	c["vis_glowself_color"][3] = 1.f;
	b["lua_enabled"] = true;
	i["oofradius"] = 20;
	i["rage_fd_enabled_style"] = 1;

	setcolor(c["boundingcoll"], 253, 253, 253, 255);
	setcolor(c["namecoll"], 253, 253, 253, 255);
	setcolor(c["healbarcoll"], 253, 0, 253, 255);
	setcolor(c["wepcoll"], 253, 253, 253, 255);
	setcolor(c["ammocoll"], 90, 143, 195, 255);
	setcolor(c["glowcoll"], 186, 75, 131, 150);
	setcolor(c["visabcoll"], 19, 92, 230, 255);
	setcolor(c["visabspcoll"], 254, 19, 19, 255);
	setcolor(c["vissoundcoll"], 250, 76, 76, 255);
	setcolor(c["loscoll"], 254, 19, 19, 255);
	setcolor(c["skelecol"], 254, 254, 176, 255);
	setcolor(c["vis_name_color"], 255, 255, 255, 200);
	setcolor(c["skycol" ], 255, 255, 255, 255 );
	
	setcolor(c["oofcoll"], 253, 253, 253, 255);
	setcolor(c["playercoll"], 158, 203, 74, 255);
	setcolor(c["adjcol"], 61, 57, 62, 255);
	setcolor(c["bullcoll"], 175, 101, 188, 255);
	setcolor(c["xqzcoll"], 74, 130, 185, 255);
	setcolor(c["teamcoll"], 65, 65, 65, 255);
	setcolor(c["txqzcoll"], 253, 253, 253, 255);
	setcolor(c["lpcoll"], 69, 69, 69, 255);
	setcolor(c["lpfcoll"], 24, 24, 24, 255);
	setcolor(c["handscoll"], 253, 253, 253, 255);
	setcolor(c["weppvmcoll"], 95, 53, 168, 255);
	setcolor(c["shadowcoll"], 24, 24, 24, 255);
	setcolor(c["propecoll"], 253, 253, 253, 255);
	setcolor(c["dropwepcoll"], 253, 253, 253, 255);
	setcolor(c["grencoll"], 253, 253, 253, 255);
	setcolor(c["inaccoll"], 24, 24, 24, 255);
	setcolor(c["bomcoll"], 251, 104, 42, 255);
	setcolor(c["grenadetraj"], 250, 76, 76, 255);
	setcolor(c["hostages"], 74, 130, 185, 255);

	c["vis_soundesp_color"][0] = 1.f;
	c["vis_soundesp_color"][1] = 1.f;
	c["vis_soundesp_color"][2] = 1.f;
	c["vis_soundesp_color"][3] = 1.f;

	c["vis_bullettracer_color"][0] = 1.f;
	c["vis_bullettracer_color"][1] = 0.f;
	c["vis_bullettracer_color"][2] = 159 / 255.f;
	c["vis_bullettracer_color"][3] = 1.f;

	c["vis_chamally_color"][0] = 255 ;
	c["vis_chamally_color"][1] = 0 ;
	c["vis_chamally_color"][2] = 179 ;
	c["vis_chamally_color"][3] = 255;

	c["vis_chamenemy_color"][0] = 173 / 255.f;
	c["vis_chamenemy_color"][1] = 244 / 255.f;
	c["vis_chamenemy_color"][2] = 5 / 255.f;
	c["vis_chamenemy_color"][3] = 255 / 255.f;

	c["vis_chamenemy_xqz_color"][0] = 0 / 255.f;
	c["vis_chamenemy_xqz_color"][1] = 170 / 255.f;
	c["vis_chamenemy_xqz_color"][2] = 255 / 255.f;
	c["vis_chamenemy_xqz_color"][3] = 255 / 255.f;

	c["vis_chamself_color"][0] = 0 / 255.f;
	c["vis_chamself_color"][1] = 0 / 255.f;
	c["vis_chamself_color"][2] = 0 / 255.f;
	c["vis_chamself_color"][3] = 0 / 255.f;

	c["vis_chamshadow_color"][0] = 0 / 255.f;
	c["vis_chamshadow_color"][1] = 0 / 255.f;
	c["vis_chamshadow_color"][2] = 0 / 255.f;
	c["vis_chamshadow_color"][3] = 255 / 255.f;

	c["vis_fakeangles_color"][0] = 255 / 255.f;
	c["vis_fakeangles_color"][1] = 255 / 255.f;
	c["vis_fakeangles_color"][2] = 255 / 255.f;
	c["vis_fakeangles_color"][3] = 255 / 255.f;

	c["vis_droppedwpns_color"][0] = 255 / 255.f;
	c["vis_droppedwpns_color"][1] = 255 / 255.f;
	c["vis_droppedwpns_color"][2] = 255 / 255.f;
	c["vis_droppedwpns_color"][3] = 255 / 255.f;

	c["vis_grenades_color"][0] = 255 / 255.f;
	c["vis_grenades_color"][1] = 255 / 255.f;
	c["vis_grenades_color"][2] = 255 / 255.f;
	c["vis_grenades_color"][3] = 255 / 255.f;

	c["vis_inaccuracyoverlay_color"][0] = 255 / 255.f;
	c["vis_inaccuracyoverlay_color"][1] = 255 / 255.f;
	c["vis_inaccuracyoverlay_color"][2] = 255 / 255.f;
	c["vis_inaccuracyoverlay_color"][3] = 100 / 255.f;

	c["vis_bomb_color"][0] = 255 / 255.f;
	c["vis_bomb_color"][1] = 0 / 255.f;
	c["vis_bomb_color"][2] = 0 / 255.f;
	c["vis_bomb_color"][3] = 255 / 255.f;

	i["misc_menukey"] = VK_DELETE;
	i["misc_fov_over"] = 90;
	i["misc_over_zoom_fov"] = 100;
	i["vis_wallstransp"] = 100;
	i["vis_propstransp"] = 100;
	i["vis_maximalespdistance"] = 500;
	i["vis_outoffovarrows_dist"] = 200;
	i["fl_limit"] = 13;
	i["vis_outoffovarrows_size"] = 32;
	i["rage_antiaim_desync_limit"] = 60;
	i["sel_lua"] = -1;

	i["rage_aimbot_keystyle"] = 0;
	i["rage_quickstop_keystyle"] = 1;
	i["rage_forcebaim_keystyle"] = 1;
	i["rage_slowmotion_keystyle"] = 1;
	i["rage_fakeduck_keystyle"] = 1;
	i["min_hit_chan"] = 50;
	i["transwalls"] = 100;
	i["transprops"] = 100;
	i["min_hit_dam"] = 10;
	i["rage_fov"] = 180;
	i["rage_flip_keystyle"] = 1;
	i["vis_thirdperson_keystyle"] = 2;
	i["misc_circlestrafe_keystyle"] = 1;
	i["misc_lagexploit_keystyle"] = 1;
	i["misc_pingspike_keystyle"] = 2;

	b["info_bar"] = true;
	b["misc_showprefix"] = true;
	b["misc_showprefix"] = true;
	b["misc_luas_loaded"] = false;
	b["misc_antiuntrusted"] = true;
	b["rage_friendlyfire"] = false;
	b["rage_logs"] = true;

	f["legitspeed"] = 0.65;
	f["legitspeedatk"] = 0.65;

	for (int i = 0; i < 6; i++)
		v["rage_hitbox"].push_back(FALSE);


}

void c_config::load_keys() {
	for (int k = 0; k < presets.size(); k++) {
		char buffer[32] = { 0 }; sprintf(buffer, "_preset_%i", k);
		i[buffer] = GetPrivateProfileIntA("k", buffer, 0, "C:/gs/cfg/csgo_keys.dog");
	}
}
void c_config::save_menucolor()
{

}

void c_config::save_keys() {
	for (int k = 0; k < presets.size(); k++) {
		char buffer[32] = { 0 }; sprintf(buffer, "_preset_%i", k);
		char value[32] = { 0 }; sprintf(value, "%i", i[buffer]);
		WritePrivateProfileStringA("k", buffer, value, "C:/gs/cfg/csgo_keys.dog");
	}
}