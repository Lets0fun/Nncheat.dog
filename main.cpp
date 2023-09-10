// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <ShlObj.h>
#include <ShlObj_core.h>
#include "includes.hpp"
#include "utils\ctx.hpp"
#include "utils\recv.h"
#include "utils\imports.h"
#include "nSkinz\SkinChanger.h"
#include "steam/steam_api.h"
#include "cheats/lagcompensation/local_animations.h"
#include "sdk/Memory.h"
#include "discord_rpc_init.h"
#include <iostream>
#include <fstream>
#include "Menu/WndProc.h"
#include "xorstr.hpp"
#include <comdef.h>
#include <string> 
#include <Wininet.h>

#define WIN32_LEAN_AND_MEAN

#include "cheats/misc/fakelag.h"
int __cdecl sub_102A7940(void* Src, size_t Size)
{
	return 1;
};
int __cdecl sub_102A7960(int a1)
{
	return 1; //don't know 1/0
};

char __cdecl sub_102A7980(void* Src, size_t Size, LPCSTR lpModuleName)
{
	return 1; //don't know guess
};
char __cdecl sub_102A79A0(LPCSTR lpModuleName, int a2, int a3)
{
	return 1;
};

int __cdecl sub_102A79F0(LPCSTR lpModuleName, int a2)
{
	return 1;
};
int __cdecl sub_102A79C0(int a1, int a2, int a3, int a4)
{
	return 1;
};

__inline void DetourPatchRetAddr()
{
	const char* lpModuleName[66];

	lpModuleName[0] = "client.dll";
	lpModuleName[1] = "engine.dll";
	lpModuleName[2] = "inputsystem.dll";
	lpModuleName[3] = "materialsystem.dll";
	lpModuleName[4] = "server.dll";
	lpModuleName[5] = "matchmaking.dll";
	lpModuleName[6] = "antitamper.dll";
	lpModuleName[7] = "cairo.dll";
	lpModuleName[8] = "datacache.dll";
	lpModuleName[9] = "filesystem_stdio.dll";
	lpModuleName[10] = "icui18n.dll";
	lpModuleName[11] = "icuuc.dll";
	lpModuleName[12] = "imemanager.dll";
	lpModuleName[13] = "launcher.dll";
	lpModuleName[14] = "libavcodec-56.dll";
	lpModuleName[15] = "libavformat-56.dll";
	lpModuleName[16] = "libavresample-2.dll";
	lpModuleName[17] = "libavutil-54.dll";
	lpModuleName[18] = "libfontconfig-1.dll";
	lpModuleName[19] = "libfreetype-6.dll";
	lpModuleName[20] = "libglib-2.0-0.dll";
	lpModuleName[21] = "libgmodule-2.0-0.dll";
	lpModuleName[22] = "libgobject-2.0-0.dll";
	lpModuleName[23] = "libpango-1.0-0.dll";
	lpModuleName[24] = "libpangoft2-1.0-0.dll";
	lpModuleName[25] = "libswscale-3.dll";
	lpModuleName[26] = "localize.dll";
	lpModuleName[27] = "mss32.dll";
	lpModuleName[28] = "mssdolby.flt";
	lpModuleName[29] = "mssds3d.flt";
	lpModuleName[30] = "mssdsp.flt";
	lpModuleName[31] = "msseax.flt";
	lpModuleName[32] = "mssmp3.asi";
	lpModuleName[33] = "msssrs.flt";
	lpModuleName[34] = "panorama.dll";
	lpModuleName[35] = "panorama_text_pango.dll";
	lpModuleName[36] = "panoramauiclient.dll";
	lpModuleName[37] = "parsifal.dll";
	lpModuleName[38] = "phonon.dll";
	lpModuleName[39] = "scenefilecache.dll";
	lpModuleName[40] = "serverbrowser.dll";
	lpModuleName[41] = "shaderapidx9.dll";
	lpModuleName[42] = "soundemittersystem.dll";
	lpModuleName[43] = "soundsystem.dll";
	lpModuleName[44] = "stdshader_dbg.dll";
	lpModuleName[45] = "stdshader_dx9.dll";
	lpModuleName[46] = "steam_api.dll";
	lpModuleName[47] = "steamnetworkingsockets.dll";
	lpModuleName[48] = "studiorender.dll";
	lpModuleName[49] = "tier0.dll";
	lpModuleName[50] = "v8.dll";
	lpModuleName[51] = "v8_libbase.dll";
	lpModuleName[52] = "v8_libplatform.dll";
	lpModuleName[53] = "valve_avi.dll";
	lpModuleName[54] = "vaudio_celt.dll";
	lpModuleName[55] = "vaudio_miles.dll";
	lpModuleName[56] = "vaudio_speex.dll";
	lpModuleName[57] = "vgui2.dll";
	lpModuleName[58] = "vguimatsurface.dll";
	lpModuleName[59] = "video.dll";
	lpModuleName[60] = "vphysics.dll";
	lpModuleName[61] = "vscript.dll";
	lpModuleName[62] = "vstdlib.dll";
	lpModuleName[63] = "vtex_dll.dll";
	lpModuleName[64] = "fmod.dll";
	lpModuleName[65] = "fmodstudio.dll";


	for (int i = 0; i < sizeof(lpModuleName); i++)
	{
		static auto sub_102A7940_sig = (DWORD)(util::FindSignature((const char*)lpModuleName[i], crypt_str("6A 00 E8 ? ? ? ? 83 C4 0C 6A 00")));
		static auto sub_102A7960_sig = (DWORD)(util::FindSignature((const char*)lpModuleName[i], crypt_str("E8 ? ? ? ? 83 C4 08 6A 0A")));
		static auto sub_102A7980_sig = (DWORD)(util::FindSignature((const char*)lpModuleName[i], crypt_str("E8 ? ? ? ? 83 C4 10 6A 00 6A 0A")));
		static auto sub_102A79A0_sig = (DWORD)(util::FindSignature((const char*)lpModuleName[i], crypt_str("E8 ? ? ? ? 83 C4 0C 6A 0A")));
		static auto sub_102A79F0_sig = (DWORD)(util::FindSignature((const char*)lpModuleName[i], crypt_str("E8 ? ? ? ? 83 C4 08 6A 02 6A 01")));
		static auto sub_102A79C0_sig = (DWORD)(util::FindSignature((const char*)lpModuleName[i], crypt_str("6A 0A E8 ? ? ? ? 83 C4 10")));

		if (sub_102A7940_sig)
			DetourFunction((PBYTE)sub_102A7940_sig, (PBYTE)sub_102A7940);

		if (sub_102A7960_sig)
			DetourFunction((PBYTE)sub_102A7960_sig, (PBYTE)sub_102A7960);

		if (sub_102A7980_sig)
			DetourFunction((PBYTE)sub_102A7980_sig, (PBYTE)sub_102A7980);

		if (sub_102A79A0_sig)
			DetourFunction((PBYTE)sub_102A79A0_sig, (PBYTE)sub_102A79A0);

		if (sub_102A79F0_sig)
			DetourFunction((PBYTE)sub_102A79F0_sig, (PBYTE)sub_102A79F0);

		if (sub_102A79C0_sig)
			DetourFunction((PBYTE)sub_102A79C0_sig, (PBYTE)sub_102A79C0);
	}
}


PVOID base_address = nullptr;
__forceinline void setup_render();
__forceinline void setup_netvars();

__forceinline void setup_hooks();
__forceinline void setup_steam();
__forceinline void setup_skins();
//__forceinline void setup_sub();
__forceinline void discord_main();

Memory memory;

__forceinline void setup_steam()
{
	typedef uint32_t SteamPipeHandle;
	typedef uint32_t SteamUserHandle;

	SteamUserHandle hSteamUser = ((SteamUserHandle(__cdecl*)(void))GetProcAddress(GetModuleHandle("steam_api.dll"), "SteamAPI_GetHSteamUser"))();
	SteamPipeHandle hSteamPipe = ((SteamPipeHandle(__cdecl*)(void))GetProcAddress(GetModuleHandle("steam_api.dll"), "SteamAPI_GetHSteamPipe"))();

	SteamClient = ((ISteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandle("steam_api.dll"), "SteamClient"))();
	SteamGameCoordinator = (ISteamGameCoordinator*)SteamClient->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamGameCoordinator001");
	SteamUser = (ISteamUser*)SteamClient->GetISteamUser(hSteamUser, hSteamPipe, "SteamUser019");
	SteamFriends = SteamClient->GetISteamFriends(hSteamUser, hSteamPipe, "SteamFriends015");
	SteamUtils = SteamClient->GetISteamUtils(hSteamPipe, "SteamUtils009");
}



inline bool exists_test0(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}



std::string replaceAll_2(std::string subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::string DownloadString_new(std::string URL) {
	HINTERNET interwebs = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HINTERNET urlFile;
	std::string rtn;
	if (interwebs) {
		urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, NULL);
		if (urlFile) {
			char buffer[2000];
			DWORD bytesRead;
			do {
				InternetReadFile(urlFile, buffer, 2000, &bytesRead);
				rtn.append(buffer, bytesRead);
				memset(buffer, 0, 2000);
			} while (bytesRead);
			InternetCloseHandle(interwebs);
			InternetCloseHandle(urlFile);
			std::string p = replaceAll_2(rtn, "|n", "\r\n");
			return p;
		}
	}
	InternetCloseHandle(interwebs);
	std::string p = replaceAll_2(rtn, "|n", "\r\n");
	return p;
}

__forceinline void setup_sub()
{
	DWORD BufferSize = 20000;
	char szUsername[20000] = {};
	std::string username;
	std::string niger;
	niger = RegGetValue(HKEY_CURRENT_USER, "Software\gsfun", "login", RRF_RT_ANY, NULL, (PVOID)&szUsername, &BufferSize);
	if (niger == "sano", "Spencer") {
		std::this_thread::sleep_for(std::chrono::milliseconds(10000000000000000000));
	}
}

Discord* g_Discord;

__forceinline void discord_main()
{
	const char* fart[]{ "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long amongus = 0x69690004C201B0;
	for (auto sex : fart) WriteProcessMemory(GetCurrentProcess(), (LPVOID)util::FindSignature(sex, "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &amongus, 7, 0);

	g_Discord->Initialize();
	g_Discord->Update();

}


#include <cheats/MultiThread/Multithread.hpp>
#include "Menu/MenuFramework/Renderer.h"
DWORD WINAPI main(PVOID base)
{
	g_ctx.signatures =
	{
			crypt_str("A1 ? ? ? ? 50 8B 08 FF 51 0C"),
			crypt_str("B9 ?? ?? ?? ?? A1 ?? ?? ?? ?? FF 10 A1 ?? ?? ?? ?? B9"),
			crypt_str("0F 11 05 ?? ?? ?? ?? 83 C8 01"),
			crypt_str("8B 0D ?? ?? ?? ?? 8B 46 08 68"),
			crypt_str("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10"),
			crypt_str("8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7"),
			crypt_str("A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6"),
			crypt_str("80 3D ? ? ? ? ? 53 56 57 0F 85"),
			crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C"),
			crypt_str("80 3D ? ? ? ? ? 74 06 B8"),
			crypt_str("55 8B EC 83 E4 F0 B8 D8"),
			crypt_str("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"),
			crypt_str("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"),
			crypt_str("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36"),
			crypt_str("56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23"),
			crypt_str("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14 83 7F 60"),
			crypt_str("55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9"),
			crypt_str("57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"),
			crypt_str("55 8B EC 81 EC ? ? ? ? 53 8B D9 89 5D F8"),
			crypt_str("53 0F B7 1D ? ? ? ? 56"),
			crypt_str("8B 0D ? ? ? ? 8D 95 ? ? ? ? 6A 00 C6"),
			crypt_str("8B 35 ? ? ? ? FF 10 0F B7 C0")
	};

	g_ctx.indexes =
	{
		5,
		33,
		340,
		219,
		220,
		34,
		158,
		75,
		461,
		483,
		453,
		484,
		285,
		224,
		247,
		27,
		17,
		123
	};

	while (!IFH(GetModuleHandle)(crypt_str("serverbrowser.dll")))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	base_address = base;

	setup_sounds();

	setup_skins();

	setup_netvars();

	c_lua::get().initialize();

	cfg_manager->setup();

	key_binds::get().initialize_key_binds();

	setup_hooks();

	setup_sub();

	discord_main();

	Netvars::Netvars();

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	return EXIT_SUCCESS;
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		IFH(DisableThreadLibraryCalls)(hModule);

		auto current_process = IFH(GetCurrentProcess)();
		auto priority_class = IFH(GetPriorityClass)(current_process);

		if (priority_class != HIGH_PRIORITY_CLASS && priority_class != REALTIME_PRIORITY_CLASS)
			IFH(SetPriorityClass)(current_process, HIGH_PRIORITY_CLASS);

		CreateThread(nullptr, 0, main, hModule, 0, nullptr); //-V718 //-V513
	}

	return TRUE;
}

vmthook* sv_cheats;
using GetBool_t = bool(__thiscall*)(void*);
bool __stdcall sv_cheatshk() {
	const auto return_address = reinterpret_cast<DWORD>(_ReturnAddress());
	static const auto cam_think = util::FindSignature("client.dll", "85 C0 75 30 38 87");

	if (return_address == cam_think)
		return true;

	return sv_cheats->get_func_address<GetBool_t>(13);
}

using eye_angles_valve = Vector * (__thiscall*)(void*);
inline eye_angles_valve _eye_angles;

Vector* __fastcall hkGetEyeAngles(void* ecx, void* edx)
{
	static int* WantedReturnAddress1 = (int*)util::FindSignature("client.dll", crypt_str("8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5"));
	static int* WantedReturnAddress2 = (int*)util::FindSignature("client.dll", crypt_str("8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?")); //Update Animations X/Y
	static int* WantedReturnAddress3 = (int*)util::FindSignature("client.dll", crypt_str("F3 0F 10 55 ? 51 8B 8E ? ? ? ?"));                                    //Update Animations X/Y

	static auto oGetEyeAngles = _eye_angles;

	if (_ReturnAddress() != WantedReturnAddress1 && _ReturnAddress() != WantedReturnAddress2 && _ReturnAddress() != WantedReturnAddress3)
		return oGetEyeAngles(ecx);

	if (!ecx || ((player_t*)ecx)->EntIndex() != m_engine()->GetLocalPlayer())
		return oGetEyeAngles(ecx);

	return &local_animations::get().local_data.real_angles;
}

__forceinline void setup_render()
{
	static auto create_font = [](const char* name, int size, int weight, DWORD flags) -> vgui::HFont
	{
		g_ctx.last_font_name = name;

		auto font = m_surface()->FontCreate();
		m_surface()->SetFontGlyphSet(font, name, size, weight, 0, 0, flags);

		return font;
	};

	fonts[LOGS] = create_font(crypt_str("Lucida Console"), 10, FW_MEDIUM, FONTFLAG_DROPSHADOW);
	fonts[ESP] = create_font(crypt_str("undefeated"), 11, FW_MEDIUM, FONTFLAG_OUTLINE); //Smallest Pixel-7
	fonts[NAME] = create_font(crypt_str("Verdana"), 12, FW_MEDIUM, FONTFLAG_ANTIALIAS);
	fonts[SUBTABWEAPONS] = create_font(crypt_str("undefeated"), 13 * CMenu::get()->GetDPINum(), FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[KNIFES] = create_font(crypt_str("icomoon"), 13 * CMenu::get()->GetDPINum(), FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[GRENADES] = create_font(crypt_str("undefeated"), 20 * CMenu::get()->GetDPINum(), FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

	g_ctx.last_font_name.clear();
}

__forceinline void setup_netvars()
{
	netvars::get().tables.clear();
	auto client = m_client()->GetAllClasses();

	if (!client)
		return;

	while (client)
	{
		auto recvTable = client->m_pRecvTable;

		if (recvTable)
			netvars::get().tables.emplace(std::string(client->m_pNetworkName), recvTable);

		client = client->m_pNext;
	}
}

__forceinline void setup_skins()
{
	auto items = std::ifstream(crypt_str("csgo/scripts/items/items_game_cdn.txt"));
	auto gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();

	memory.initialize();

	for (auto i = 0; i <= memory.itemSchema()->paintKits.lastElement; i++)
	{
		auto paintKit = memory.itemSchema()->paintKits.memory[i].value;

		if (paintKit->id == 9001)
			continue;

		auto itemName = m_localize()->FindSafe(paintKit->itemName.buffer + 1);
		auto itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

		if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
		{
			if (paintKit->id < 10000)
			{
				//if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } + '='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } + '=') == std::string::npos)
				//{
				//	if (auto weaponName = gameItems.rfind(crypt_str("weapon_"), pos); weaponName != std::string::npos)
				//	{
				//		name.back() = ' ';
				//	}
				//}
				SkinChanger::skinKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
			else
			{
				std::string_view gloveName{ paintKit->name.buffer };
				name.back() = ' ';
				SkinChanger::gloveKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
		}
	}

	std::sort(SkinChanger::skinKits.begin(), SkinChanger::skinKits.end());
	std::sort(SkinChanger::gloveKits.begin(), SkinChanger::gloveKits.end());
}

__forceinline void setup_hooks()
{

	/* init multithread */
	Threading::InitThreads();

	static auto getforeignfallbackfontname = (DWORD)(util::FindSignature(crypt_str("vguimatsurface.dll"), g_ctx.signatures.at(9).c_str()));
	hooks::original_getforeignfallbackfontname = (DWORD)DetourFunction((PBYTE)getforeignfallbackfontname, (PBYTE)hooks::hooked_getforeignfallbackfontname); //-V206

	static auto setupbones = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(10).c_str()));
	hooks::original_setupbones = (DWORD)DetourFunction((PBYTE)setupbones, (PBYTE)hooks::hooked_setupbones); //-V206

	static auto eye_angles = (DWORD)(util::FindSignature(crypt_str("client.dll"), "56 8B F1 85 F6 74 32"));
	_eye_angles = (eye_angles_valve)DetourFunction((PBYTE)eye_angles, (PBYTE)hkGetEyeAngles);

	static auto doextrabonesprocessing = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(11).c_str()));
	hooks::original_doextrabonesprocessing = (DWORD)DetourFunction((PBYTE)doextrabonesprocessing, (PBYTE)hooks::hooked_doextrabonesprocessing); //-V206

	static auto standardblendingrules = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(12).c_str()));
	hooks::original_standardblendingrules = (DWORD)DetourFunction((PBYTE)standardblendingrules, (PBYTE)hooks::hooked_standardblendingrules); //-V206

	static auto updateclientsideanimation = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(13).c_str()));
	hooks::original_updateclientsideanimation = (DWORD)DetourFunction((PBYTE)updateclientsideanimation, (PBYTE)hooks::hooked_updateclientsideanimation); //-V206

	static auto physicssimulate = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(14).c_str()));
	hooks::original_physicssimulate = (DWORD)DetourFunction((PBYTE)physicssimulate, (PBYTE)hooks::hooked_physicssimulate);

	static auto ClampBonesInBBox = (DWORD)(util::FindSignature("client.dll", "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38 83"));
	hooks::original_oClampBonesInBBox = (DWORD)DetourFunction((byte*)ClampBonesInBBox, (byte*)hooks::ClampBonesInBBox); //-V206

	static auto clmove = (DWORD)(util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 81 EC 64 01 00 00 53 56 8A F9")));
	hooks::original_clmove = (DWORD)DetourFunction((PBYTE)clmove, (PBYTE)hooks::hooked_clmove);

	static auto modifyeyeposition = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(15).c_str()));
	hooks::original_modifyeyeposition = (DWORD)DetourFunction((PBYTE)modifyeyeposition, (PBYTE)hooks::hooked_modifyeyeposition);

	static auto calcviewmodelbob = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(16).c_str()));
	hooks::original_calcviewmodelbob = (DWORD)DetourFunction((PBYTE)calcviewmodelbob, (PBYTE)hooks::hooked_calcviewmodelbob);

	static auto shouldskipanimframe = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(17).c_str()));
	DetourFunction((PBYTE)shouldskipanimframe, (PBYTE)hooks::hooked_shouldskipanimframe);

	static auto checkfilecrcswithserver = (DWORD)(util::FindSignature(crypt_str("engine.dll"), g_ctx.signatures.at(18).c_str()));
	DetourFunction((PBYTE)checkfilecrcswithserver, (PBYTE)hooks::hooked_checkfilecrcswithserver);

	static auto processinterpolatedlist = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(19).c_str()));
	hooks::original_processinterpolatedlist = (DWORD)DetourFunction((byte*)processinterpolatedlist, (byte*)hooks::processinterpolatedlist); //-V206

	hooks::client_hook = new vmthook(reinterpret_cast<DWORD**>(m_client()));
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_fsn), 37); //-V107 //-V221
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_writeusercmddeltatobuffer), 24); //-V107 //-V221

	hooks::clientstate_hook = new vmthook(reinterpret_cast<DWORD**>((CClientState*)(uint32_t(m_clientstate()) + 0x8)));
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_packetstart), 5); //-V107 //-V221
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_packetend), 6); //-V107 //-V221

	hooks::panel_hook = new vmthook(reinterpret_cast<DWORD**>(m_panel())); //-V1032
	hooks::panel_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_painttraverse), 41); //-V107 //-V221

	hooks::clientmode_hook = new vmthook(reinterpret_cast<DWORD**>(m_clientmode()));
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_createmove_proxy), 22);
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_postscreeneffects), 44); //-V107 //-V221
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_overrideview), 18); //-V107 //-V221
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_drawfog), 17); //-V107 //-V221

	hooks::inputinternal_hook = new vmthook(reinterpret_cast<DWORD**>(m_inputinternal())); //-V114
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_setkeycodestate), 91); //-V107 //-V221
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_setmousecodestate), 92); //-V107 //-V221

	hooks::engine_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()));
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_isconnected), 27); //-V107 //-V221
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_getscreenaspectratio), 101); //-V107 //-V221
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_ishltv), 93); //-V107 //-V221

	hooks::renderview_hook = new vmthook(reinterpret_cast<DWORD**>(m_renderview()));
	hooks::renderview_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_sceneend), 9); //-V107 //-V221

	hooks::materialsys_hook = new vmthook(reinterpret_cast<DWORD**>(m_materialsystem())); //-V1032
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_beginframe), 42); //-V107 //-V221
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_getmaterial), 84); //-V107 //-V221

	hooks::modelrender_hook = new vmthook(reinterpret_cast<DWORD**>(m_modelrender()));
	hooks::modelrender_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_dme), 21); //-V107 //-V221

	hooks::surface_hook = new vmthook(reinterpret_cast<DWORD**>(m_surface()));
	hooks::surface_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_lockcursor), 67); //-V107 //-V221

	hooks::bspquery_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()->GetBSPTreeQuery()));
	hooks::bspquery_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_listleavesinbox), 6); //-V107 //-V221

	hooks::prediction_hook = new vmthook(reinterpret_cast<DWORD**>(m_prediction())); //-V1032
	hooks::prediction_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_runcommand), 19); //-V107 //-V221

	hooks::modelcache_hook = new vmthook(reinterpret_cast<DWORD**>(m_modelcache())); //-V1032
	hooks::modelcache_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_findmdl), 10); //-V107 //-V221

	hooks::trace_hook = new vmthook(reinterpret_cast<DWORD**>(m_trace()));

	auto sv_cheats_cvar = m_cvar()->FindVar("sv_cheats");
	sv_cheats = new vmthook(reinterpret_cast<DWORD**>(sv_cheats_cvar));
	sv_cheats->hook_function(reinterpret_cast<uintptr_t>(sv_cheatshk), 13);

	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_clip_ray_collideable), 4); //-V107 //-V221
	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_trace_ray), 5); //-V107 //-V221

	hooks::filesystem_hook = new vmthook(reinterpret_cast<DWORD**>(util::FindSignature(crypt_str("engine.dll"), g_ctx.signatures.at(20).c_str()) + 0x2));
	hooks::filesystem_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_loosefileallowed), 128); //-V107 //-V221

	while (!(INIT::Window = IFH(FindWindow)(crypt_str("Valve001"), nullptr)))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	INIT::OldWindow = (WNDPROC)SetWindowLongPtr(FindWindow("Valve001", 0), GWL_WNDPROC, (long)&Hooked::wndProc);

	oWndProc = INIT::OldWindow;

	hooks::directx_hook = new vmthook(reinterpret_cast<DWORD**>(m_device()));
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene_Reset), 16); //-V107 //-V221
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_present), 17); //-V107 //-V221
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene), 42); //-V107 //-V221

	hooks::hooked_events.RegisterSelf();
}