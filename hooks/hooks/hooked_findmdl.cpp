// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"

using FindMDL_t = void(__thiscall*)(void*, char*);

void __fastcall hooks::hooked_findmdl(void* ecx, void* edx, char* FilePath)
{
	static auto original_fn = modelcache_hook->get_func_address <FindMDL_t> (10);
	if (strstr(FilePath, crypt_str("facemask_battlemask.mdl")))
		sprintf(FilePath, crypt_str("models/player/holiday/facemasks/facemask_dallas.mdl"));

	return original_fn(ecx, FilePath);
}