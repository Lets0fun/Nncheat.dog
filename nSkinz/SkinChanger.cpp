// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <algorithm>
#include <fstream>
#include "..\configs\configs.h"
#include "SkinChanger.h"
#include "..\cheats\misc\logs.h"
#include "../Menu/Menu.h"


#define INVALID_EHANDLE_INDEX 0xFFFFFFFF


static const char* player_models[] =
{
	//"models/player/custom_player/legacy/tm_phoenix.mdl",
	//"models/player/custom_player/legacy/ctm_sas.mdl",
	//"models/player/custom_player/legacy/tm_professional_varf1.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variante.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl",
	//"models/player/custom_player/legacy/tm_professional_varf2.mdl",
	//"models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_diver_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_diver_varianta.mdl",
	//"models/player/custom_player/legacy/tm_professional_varf3.mdl",
	//"models/player/custom_player/legacy/tm_professional_varf4.mdl",
	//"models/player/custom_player/legacy/tm_professional_varf.mdl",
	//"models/player/custom_player/legacy/tm_professional_varj.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl",
	//"models/player/custom_player/legacy/tm_professional_varf5.mdl",
	//"models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_diver_variantc.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variantс.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl",
	//"models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantk.mdl",
	//"models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantn.mdl",
	//"models/player/custom_player/legacy/ctm_sas_variantg.mdl",
	//"models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl",
	//"models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl",
	//"models/player/custom_player/legacy/tm_professional_vari.mdl",
	//"models/player/custom_player/legacy/tm_professional_varg.mdl",
	//"models/player/custom_player/legacy/tm_professional_varh.mdl",
	//"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
	//"models/player/custom_player/legacy/tm_balkan_variantk.mdl",
	//"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
	//"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
	//"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
	//"models/player/custom_player/legacy/tm_balkan_variantl.mdl",
	//"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantl.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantj.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variante.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
	//"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variante.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantf.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantg.mdl",
	//"models/player/custom_player/legacy/ctm_swat_varianth.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantj.mdl",
	//"models/player/custom_player/legacy/ctm_swat_varianti.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
	//"models/player/custom_player/legacy/tm_phoenix_varianti.mdl",
	//"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
	//"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
	//"models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
	//"models/player/custom_player/legacy/tm_leet_variantf.mdl",
	//"models/player/custom_player/legacy/tm_leet_varianti.mdl",
	//"models/player/custom_player/legacy/tm_leet_varianth.mdl",
	//"models/player/custom_player/legacy/tm_leet_variantg.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
	"models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl",
	"models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl",
	//"models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl",
	//"models/player/custom_player/legacy/tm_anarchist_varianta.mdl",
	//"models/player/custom_player/legacy/tm_anarchist_variantb.mdl",
	//"models/player/custom_player/legacy/tm_anarchist_variantc.mdl",
	//"models/player/custom_player/legacy/tm_separatist_varianta.mdl",
	//"models/player/custom_player/legacy/tm_separatist_variantb.mdl",
	//"models/player/custom_player/legacy/tm_separatist_variantc.mdl",
	//"models/player/custom_player/legacy/tm_separatist_variantd.mdl",
	//"models/player/custom_player/legacy/ctm_fbi.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_varianta.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_variantd.mdl",
	//"models/player/custom_player/legacy/ctm_fbi_variante.mdl",
	"models/player/custom_player/legacy/ctm_gign_varianta.mdl",
	//"models/player/custom_player/legacy/ctm_gign_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_gign_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_st6.mdl",
	//"models/player/custom_player/legacy/ctm_st6_varianta.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_st6_variantd.mdl",
	//"models/player/custom_player/legacy/ctm_idf_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_idf_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_idf_variantd.mdl",
	//"models/player/custom_player/legacy/ctm_idf_variante.mdl",
	//"models/player/custom_player/legacy/ctm_idf_variantf.mdl",
	//"models/player/custom_player/legacy/ctm_swat.mdl",
	//"models/player/custom_player/legacy/ctm_swat_varianta.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_swat_variantd.mdl",
	//"models/player/custom_player/legacy/ctm_sas.mdl",
	//"models/player/custom_player/legacy/ctm_gsg9.mdl",
	//"models/player/custom_player/legacy/ctm_gsg9_varianta.mdl",
	//"models/player/custom_player/legacy/ctm_gsg9_variantb.mdl",
	//"models/player/custom_player/legacy/ctm_gsg9_variantc.mdl",
	//"models/player/custom_player/legacy/ctm_gsg9_variantd.mdl",
	//"models/player/custom_player/legacy/tm_professional_var1.mdl",
	//"models/player/custom_player/legacy/tm_professional_var2.mdl",
	//"models/player/custom_player/legacy/tm_professional_var3.mdl",
	//"models/player/custom_player/legacy/tm_professional_var4.mdl",
	//"models/player/custom_player/legacy/tm_leet_varianta.mdl",
	//"models/player/custom_player/legacy/tm_leet_variantb.mdl",
	//"models/player/custom_player/legacy/tm_leet_variantc.mdl",
	//"models/player/custom_player/legacy/tm_leet_variantd.mdl",
	//"models/player/custom_player/legacy/tm_Balkan_varianta.mdl",
	//"models/player/custom_player/legacy/tm_Balkan_variantb.mdl",
	//"models/player/custom_player/legacy/tm_Balkan_variantc.mdl",
	//"models/player/custom_player/legacy/tm_Balkan_variantd.mdl",
	//"models/player/custom_player/legacy/tm_pirate_varianta.mdl",
	//"models/player/custom_player/legacy/tm_pirate_variantb.mdl",
	//"models/player/custom_player/legacy/tm_pirate_variantc.mdl",
};

static const char* player_model_index_knife[] =
{
	"models/weapons/v_knife_skeleton.mdl",
	"models/weapons/v_knife_skeleton.mdl",
	"models/weapons/v_knife_skeleton.mdl"
};

static const char* mask_models[ ] =
{
		  ("models\\player\\holiday\\facemasks\\facemask_battlemask.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_hoxton.mdl"),
		  ("models\\player\\holiday\\facemasks\\porcelain_doll.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_skull.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_samurai.mdl"),
		  ("models\\player\\holiday\\facemasks\\evil_clown.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_wolf.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_sheep_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_bunny_gold.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_anaglyph.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_porcelain_doll_kabuki.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_dallas.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_pumpkin.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_sheep_bloody.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_devil_plastic.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_boar.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_chains.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tiki.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_bunny.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_sheep_gold.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_zombie_fortune_plastic.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_chicken.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_skull_gold.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_demo_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_engi_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_heavy_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_medic_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_pyro_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_scout_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_sniper_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_soldier_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_spy_model.mdl"),
		  ("models\\props\\holiday_light\\holiday_light.mdl"),
};



std::unordered_map <std::string, int> SkinChanger::model_indexes;
std::unordered_map <std::string, int> SkinChanger::player_model_indexes;
std::unordered_map<int, SkinChanger::EconomyItemCfg> g_SkinChangerCfg;

std::vector <SkinChanger::PaintKit> SkinChanger::skinKits;
std::vector <SkinChanger::PaintKit> SkinChanger::gloveKits;
std::vector <SkinChanger::PaintKit> SkinChanger::displayKits;

static std::unordered_map <std::string_view, const char*> iconOverrides;

static void erase_override_if_exists_by_index(const int definition_index) noexcept
{
	if (auto original_item = game_data::get_weapon_info(definition_index))
	{
		if (!c_config::get()->i["skins_knifechanger_model"])
			return;

		if (const auto override_entry = iconOverrides.find(original_item->icon); override_entry != end(iconOverrides))
			iconOverrides.erase(override_entry);
	}
}

static auto CreateWearable() -> CreateClientClassFn {
	auto client_class = m_client()->GetAllClasses();

	for (client_class = m_client()->GetAllClasses();
		client_class; client_class = client_class->m_pNext) {

		if (client_class->m_ClassID == ClassID::CEconWearable) {
			return client_class->m_pCreateFn;
		}
	}
}

float G::Matrix[4][4];

static void apply_config_on_attributable_item(attributableitem_t* item, const item_setting* config, const unsigned xuid_low) noexcept
{
	item->m_iItemIDHigh() = -1; //-V522
	item->m_iAccountID() = xuid_low;
	item->m_flFallbackWear() = config->wear;

	if (config->quality)
		item->m_iEntityQuality() = config->quality;

	if (config->custom_name[0])
		strcpy_s(item->m_szCustomName(), sizeof(config->custom_name), config->custom_name);

	if (config->paintKit)
		item->m_nFallbackPaintKit() = config->paintKit;

	if (config->seed)
		item->m_nFallbackSeed() = config->seed;

	if (config->stat_trak)
		item->m_nFallbackStatTrak() = config->stat_trak;

	auto& definition_index = item->m_iItemDefinitionIndex();

	if (config->definition_override_index && config->definition_override_index != definition_index)
	{
		if (auto replacement_item = game_data::get_weapon_info(config->definition_override_index))
		{
			auto old_definition_index = definition_index;
			definition_index = config->definition_override_index;

			if (SkinChanger::model_indexes.find(replacement_item->model) == SkinChanger::model_indexes.end())
				SkinChanger::model_indexes.emplace(replacement_item->model, m_modelinfo()->GetModelIndex(replacement_item->model));

			item->set_model_index(SkinChanger::model_indexes.at(replacement_item->model));
			item->PreDataUpdate(0);

			if (old_definition_index)
				if (auto original_item = game_data::get_weapon_info(old_definition_index); original_item && original_item->icon && replacement_item->icon)
					iconOverrides[original_item->icon] = replacement_item->icon;
		}
	}
	else
		erase_override_if_exists_by_index(definition_index);
}

static auto get_wearable_create_fn() -> CreateClientClassFn
{
	auto classes = m_client()->GetAllClasses();

	while (classes->m_ClassID != CEconWearable)
		classes = classes->m_pNext;

	return classes->m_pCreateFn;
}

static attributableitem_t* make_glove(int entry, int serial) noexcept
{
	static auto create_wearable_fn = CreateWearable();
	create_wearable_fn(entry, serial);

	const auto glove = reinterpret_cast<attributableitem_t*>(m_entitylist()->GetClientEntity(entry));
	assert(glove); {
		static auto set_abs_origin_addr = util::FindSignature("client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");
		const auto set_abs_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const std::array<float, 3>&)>(set_abs_origin_addr);
		static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
		set_abs_origin_fn(glove, new_pos);
	}
	return glove;
}


bool apply_knife_model(attributableitem_t* weapon, const char* model) noexcept {

	auto local_player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()));

	if (!local_player)
		return false;

	auto viewmodel = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(local_player->m_hViewModel()));

	if (!viewmodel)
		return false;

	auto h_view_model_weapon = viewmodel->weapon();

	if (!h_view_model_weapon)
		return false;

	auto view_model_weapon = reinterpret_cast<attributableitem_t*>(m_entitylist()->GetClientEntityFromHandle(h_view_model_weapon));

	if (view_model_weapon != weapon)
		return false;

	viewmodel->m_nModelIndex() = m_modelinfo()->GetModelIndex(model);

	return true;
}

bool apply_knife_skin(attributableitem_t* weapon, int item_definition_index, int paint_kit, int model_index) noexcept {
	weapon->m_iItemDefinitionIndex() = item_definition_index;
	weapon->m_nFallbackPaintKit() = paint_kit;
	weapon->m_nModelIndex() = model_index;

	return true;
}



bool apply_glove_model(attributableitem_t* glove, const char* model) noexcept {
	player_info_t info;
	m_engine()->GetPlayerInfo(m_engine()->GetLocalPlayer(), &info);
	glove->m_iAccountID() = info.xuid_low;
	glove->set_model_index( m_modelinfo( )->GetModelIndex( model ) );

	return true;
}

bool apply_glove_skin(attributableitem_t* glove, int item_definition_index, int paint_kit, int model_index, int entity_quality, float fallback_wear) noexcept {
	glove->m_iItemDefinitionIndex() = item_definition_index;
	glove->m_nFallbackPaintKit() = paint_kit;
	glove->set_model_index(model_index);
	glove->m_iEntityQuality() = entity_quality;
	glove->m_flFallbackWear() = fallback_wear;

	return true;
}

static float last_skins_update = 0.0f;

static void post_data_update_start(player_t* local) noexcept
{
	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(local->EntIndex(), &player_info))
		return;

	static auto glove_handle = CBaseHandle(0);

	auto wearables = local->m_hMyWearables();
	auto glove_config = get_by_definition_index(T_GLOVES);
	auto glove = reinterpret_cast <attributableitem_t*> (m_entitylist()->GetClientEntityFromHandle(wearables[0]));

	if (!glove)
	{
		auto our_glove = reinterpret_cast <attributableitem_t*> (m_entitylist()->GetClientEntityFromHandle(glove_handle));

		if (our_glove)
		{
			//wearables[0] = glove_handle;
			glove = our_glove;
		}
	}

	if (!local->is_alive())
	{
		if (glove)
		{
			glove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
			glove->GetClientNetworkable()->Release();
		}

		return;
	}

	if (glove_config && glove_config->definition_override_index)
	{
		if (!glove)
		{
			auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
			auto serial = rand() % 0x1000;

			glove = make_glove(entry, serial);
			wearables[0] = entry | serial << 16;
			glove_handle = wearables[0];
		}

		*reinterpret_cast <int*> (uintptr_t(glove) + 0x64) = -1;
		apply_config_on_attributable_item(glove, glove_config, player_info.xuid_low);
	}

	auto weapons = local->m_hMyWeapons();

	for (auto weapon_handle = 0; weapons[weapon_handle].IsValid(); weapon_handle++)
	{
		auto weapon = (weapon_t*)m_entitylist()->GetClientEntityFromHandle(weapons[weapon_handle]); //-V807

		if (!weapon)
			continue;

		auto& definition_index = weapon->m_iItemDefinitionIndex();

		if (auto active_conf = get_by_definition_index(is_knife(definition_index) ? WEAPON_KNIFE : definition_index))
			apply_config_on_attributable_item(weapon, active_conf, player_info.xuid_low);
		else
			erase_override_if_exists_by_index(definition_index);
	}

	auto view_model = (weapon_t*)m_entitylist()->GetClientEntityFromHandle(local->m_hViewModel());

	if (!view_model)
		return;

	auto view_model_weapon = (weapon_t*)m_entitylist()->GetClientEntityFromHandle(view_model->weapon());

	if (!view_model_weapon)
		return;

	auto override_info = game_data::get_weapon_info(view_model_weapon->m_iItemDefinitionIndex());

	if (!override_info)
		return;

	auto world_model = (weapon_t*)m_entitylist()->GetClientEntityFromHandle(view_model_weapon->m_hWeaponWorldModel());

	if (!world_model)
		return;

	if (SkinChanger::model_indexes.find(override_info->model) == SkinChanger::model_indexes.end())
		SkinChanger::model_indexes.emplace(override_info->model, m_modelinfo()->GetModelIndex(override_info->model));

	view_model->m_nModelIndex() = SkinChanger::model_indexes.at(override_info->model);
	world_model->m_nModelIndex() = SkinChanger::model_indexes.at(override_info->model) + 1;
}

static bool UpdateRequired = false;
static bool hudUpdateRequired = false;



bool ApplyGloveModel(attributableitem_t* glove, const char* model) noexcept {

	*reinterpret_cast<int*>(uintptr_t(glove) + 0x64) = -1;
	return true;
}

bool ApplyGloveSkins(attributableitem_t* glove, int item_definition_index, int paint_kit, int model_index, int world_model_index, int entity_quality, float fallback_wear) noexcept {
	
	glove->m_iItemDefinitionIndex() = item_definition_index;
	glove->m_nFallbackPaintKit() = paint_kit;
	glove->m_nModelIndex() = model_index;
	glove->m_iEntityQuality() = entity_quality;
	glove->m_flFallbackWear() = fallback_wear;

	return true;
};

inline bool ApplyCustomSkin(attributableitem_t* pWeapon, int nWeaponIndex) {
	// Check if this weapon has a valid override defined.
	if (g_SkinChangerCfg.find(nWeaponIndex) == g_SkinChangerCfg.end())
		return false;

	// Apply our changes to the fallback variables.
	pWeapon->m_nFallbackPaintKit() = g_SkinChangerCfg[nWeaponIndex].nFallbackPaintKit;
	pWeapon->m_iEntityQuality() = g_SkinChangerCfg[nWeaponIndex].iEntityQuality;
	pWeapon->m_nFallbackSeed() = g_SkinChangerCfg[nWeaponIndex].nFallbackSeed;
	pWeapon->m_nFallbackStatTrak() = g_SkinChangerCfg[nWeaponIndex].nFallbackStatTrak;
	pWeapon->m_flFallbackWear() = g_SkinChangerCfg[nWeaponIndex].flFallbackWear;

	if (g_SkinChangerCfg[nWeaponIndex].iItemDefinitionIndex)
		pWeapon->m_iItemDefinitionIndex() = g_SkinChangerCfg[nWeaponIndex].iItemDefinitionIndex;

	// If a name is defined, write it now.
	if (g_SkinChangerCfg[nWeaponIndex].szCustomName) {
		sprintf_s(pWeapon->m_szCustomName(), 32, "%s", g_SkinChangerCfg[nWeaponIndex].szCustomName);
	}

	// Edit "m_iItemIDHigh" so fallback values will be used.
	pWeapon->m_iItemIDHigh() = -1;

	return true;
}

int GetGloveSkinByMenu(int gloveIndex, int skinIndex)
{
	if (gloveIndex == 0) // bloudhound
	{
		switch (skinIndex)
		{
		case 0:
			return 10006;
		case 1:
			return 10007;
		case 2:
			return 10008;
		case 3:
			return 10039;
		default:
			return 0;
		}
	}
	else if (gloveIndex == 1) // Sport
	{
		switch (skinIndex)
		{
		case 0:
			return 10038;
		case 1:
			return 10037;
		case 2:
			return 10018;
		case 3:
			return 10019;
		case 4:
			return 10048;
		case 5:
			return 10047;
		case 6:
			return 10045;
		case 7:
			return 10046;
		default:
			return 0;
		}
	}
	else if (gloveIndex == 2) // Driver
	{
		switch (skinIndex)
		{
		case 0:
			return 10013;
		case 1:
			return 10015;
		case 2:
			return 10016;
		case 3:
			return 10040;
		case 4:
			return 10043;
		case 5:
			return 10044;
		case 6:
			return 10041;
		case 7:
			return 10042;
		default:
			return 0;
		}
	}
	else if (gloveIndex == 3) // Wraps
	{
		switch (skinIndex)
		{
		case 0:
			return 10009;
		case 1:
			return 10010;
		case 2:
			return 10021;
		case 3:
			return 10036;
		case 4:
			return 10053;
		case 5:
			return 10054;
		case 6:
			return 10055;
		case 7:
			return 10056;
		default:
			return 0;
		}
	}
	else if (gloveIndex == 4) // Moto
	{
		switch (skinIndex)
		{
		case 0:
			return 10024;
		case 1:
			return 10026;
		case 2:
			return 10027;
		case 3:
			return 10028;
		case 4:
			return 10050;
		case 5:
			return 10051;
		case 6:
			return 10052;
		case 7:
			return 10049;

		default:
			return 0;
		}
	}
	else if (gloveIndex == 5) // Specialist
	{
		switch (skinIndex)
		{
		case 0:
			return 10030;
		case 1:
			return 10033;
		case 2:
			return 10034;
		case 3:
			return 10035;
		case 4:
			return 10061;
		case 5:
			return 10062;
		case 6:
			return 10063;
		case 7:
			return 10064;
		default:
			return 0;
		}
	}
	else if (gloveIndex == 6)
	{
		switch (skinIndex)
		{
		case 0:
			return 10057;
		case 1:
			return 10058;
		case 2:
			return 10059;
		case 3:
			return 10060;
		}
	}
	else if (gloveIndex == 7)
	{
		switch (skinIndex)
		{
		case 0:
			return 10085;
		case 1:
			return 10087;
		case 2:
			return 10088;
		case 3:
			return 10086;
		}
	}
	else
		return 0;
	return 0;
};

bool LoadPlayerMdlOnce = false;

bool bGlovesNeedUpdate;

bool prechace_model(const char* thisModelName)
{
	const auto CustomModel = m_networkStringTableContainer()->findTable("modelprecache");

	if (CustomModel)
	{
		m_modelinfo()->GetModelIndex(thisModelName);
		int MdlNum = CustomModel->addString(false, thisModelName);

		if (MdlNum == NULL)
			return false;
	}
	return true;
}

void SkinChanger::InitCustomModels()
{
	if (!LoadPlayerMdlOnce)
	{
		auto player_ml = 0;

		switch (g_ctx.local()->m_iTeamNum())
		{
		case 2:
			player_ml = c_config::get()->i["skin_changer_model_agent"];
			break;
		case 3:
			player_ml = c_config::get()->i["skin_changer_model_agent"];
			break;
		}

		// Тут все модельки которые не прогруженны по дефолту из-за самой кски
		// Потом эти же модельки надо добавлять в основной модель-ченджер чтоб использовать их

		// Danger Zone
		for (int i = 0; i < ARRAYSIZE(player_models); i++) {
			prechace_model(
				player_models[i]
			);
		}

		LoadPlayerMdlOnce = true;
	}
}

bool LoadModel(const char* thisModelName)
{
	const auto CustomModel = m_networkStringTableContainer()->findTable("modelprecache");

	if (CustomModel)
	{
		m_modelinfo()->GetModelIndex(thisModelName);
		int MdlNum = CustomModel->addString(false, thisModelName);

		if (MdlNum == NULL)
			return false;
	}
	return true;
}

void custom_models()
{
	LoadModel("models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl");
	LoadModel("models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl");
	LoadModel("models/player/custom_player/legacy/ctm_gign_varianta.mdl");
}

void SkinChanger::glovchanger() noexcept {

	if (!m_engine()->IsConnected() && !m_engine()->IsInGame())
		return;

	auto local_player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()));
	if (!local_player)
		return;

	if (!c_config::get()->b["skins_glovechanger"])
		return;


		auto	model_blood = "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl";
		auto	model_sport = "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl";
		auto	model_slick = "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl";
		auto	model_leath = "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl";
		auto	model_moto = "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl";
		auto	model_speci = "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl";
		auto	model_hydra = "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl";

		auto	index_blood = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
		auto	index_sport = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
		auto	index_slick = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
		auto	index_leath = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
		auto	index_moto = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
		auto	index_speci = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));
		auto	index_hydra = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl"));

		//credit to namazso for nskinz

		uintptr_t* const wearables = local_player->m_hMyWearables();
		if (!wearables)
			return;

		static uintptr_t glove_handle = uintptr_t(0);

		auto glove = reinterpret_cast<attributableitem_t*>(m_entitylist()->GetClientEntityFromHandle(wearables[0]));

		if (!glove) // There is no glove
		{
			const auto our_glove = reinterpret_cast<attributableitem_t*>(m_entitylist()->GetClientEntityFromHandle(glove_handle));

			if (our_glove) // Try to get our last created glove
			{
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}
		if (!local_player || // We are dead but we have a glove, destroy it
			!local_player->is_alive() ||
			!m_engine()->IsConnected() ||
			!m_engine()->IsInGame()
			) {
			if (glove) {
				glove->SetDestroyedOnRecreateEntities();
				glove->Release();
			}
			return;
		}
		if (!glove) // We don't have a glove, but we should
		{
			const auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
			const auto serial = rand() % 0x1000;
			glove = make_glove(entry, serial);   // He he
			wearables[0] = entry | serial << 16;
			glove_handle = wearables[0]; // Let's store it in case we somehow lose it.
		}
		if (glove)
		{
			//glove conditions
			float g_wear = 0.f;
			/*switch (config_system.item.glove_wear) {
			case 0:
				g_wear = 0.0000001f;
				break;
			case 1:
				g_wear = 0.07f;
				break;
			case 2:
				g_wear = 0.15f;
				break;
			case 3:
				g_wear = 0.38f;
				break;
			case 4:
				g_wear = 0.45f;
				break;
			}*/

			//apply glove model

			switch (c_config::get()->i["skin_changer_model_glove"]) {
			case 0:
				break;
			case 1:
				apply_glove_model(glove, model_blood);
				break;
			case 2:
				apply_glove_model(glove, model_sport);
				break;
			case 3:
				apply_glove_model(glove, model_slick);
				break;
			case 4:
				apply_glove_model(glove, model_leath);
				break;
			case 5:
				apply_glove_model(glove, model_moto);
				break;
			case 6:
				apply_glove_model(glove, model_speci);
				break;
			case 7:
				apply_glove_model(glove, model_hydra);
				break;
			}

			//apply glove skins
			switch (c_config::get()->i["skin_changer_model_glove"]) {
			case 0:
				break;
			case 1:
				apply_glove_skin(glove, STUDDED_BLOODHOUND_GLOVES, 10048, index_blood, 3, g_wear);
				break;
			case 2:
				apply_glove_skin(glove, SPORTY_GLOVES, 10048, index_sport, 3, g_wear);
				break;
			case 3:
				apply_glove_skin(glove, SLICK_GLOVES, 10048, index_slick, 3, g_wear);
				break;
			case 4:
				apply_glove_skin(glove, LEATHER_HANDWRAPS, 10048, index_leath, 3, g_wear);
				break;
			case 5:
				apply_glove_skin(glove, MOTORCYCLE_GLOVES, 10048, index_moto, 3, g_wear);
				break;
			case 6:
				apply_glove_skin(glove, SPECIALIST_GLOVES, 10048, index_speci, 3, g_wear);
				break;
			//case 7:
			//	apply_glove_skin(glove, GLOVES, config_system.item.paint_kit_index_glove, index_hydra, 3, g_wear);
			//	break;
			}

			glove->m_iItemIDHigh() = -1;
			glove->m_nFallbackSeed() = 0;
			glove->m_nFallbackStatTrak() = -1;

			glove->PreDataUpdate(DATA_UPDATE_CREATED);
	}
}

const char* default_mask = "models/player/holiday/facemasks/facemask_battlemask.mdl";
constexpr auto mask_flags = 0x10000;



bool SkinChanger::should_precache(const char* name)
{
	if (name == "")
		return false;

	if (name == nullptr)
		return false;

	auto modelprecache = m_networkStringTableContainer()->findTable(crypt_str("modelprecache"));
	if (!modelprecache)
		return false;

	auto idx = modelprecache->addString(false, name);
	if (idx == -1)
		return false;

	return true;
}

void SkinChanger::mask_changer(int stage)
{
	static auto current_mask = *reinterpret_cast<char***>(util::FindSignature("client.dll", "FF 35 ? ? ? ? FF 90 ? ? ? ? 8B 8F") + 2);

	static auto fn = reinterpret_cast<void(__thiscall*)(void*, bool)>(util::FindSignature("client.dll", "55 8B EC 83 EC ? 53 8B D9 8D 45 ? 8B 08"));

	static int old_mask = -1;

	if (!g_ctx.local() || stage != FRAME_RENDER_START && stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!precached_on_round_start)
		return;

	auto mask = "models\\player\\holiday\\facemasks\\facemask_battlemask.mdl";

	if (!should_precache(default_mask) || !should_precache(mask))
		return;

	if (c_config::get()->i["skin_changer_model_mask"] > 0)
	{
		g_ctx.local()->m_iAddonBits() |= mask_flags;

		if (old_mask != c_config::get()->i["skin_changer_model_mask"])
		{
			*current_mask = (char*)mask;
			fn(g_ctx.local(), true);
			old_mask = c_config::get()->i["skin_changer_model_mask"];
		}
	}
	else
	{
		if (g_ctx.local()->m_iAddonBits() & mask_flags)
			g_ctx.local()->m_iAddonBits() &= ~mask_flags;
	}
}




void SkinChanger::run(ClientFrameStage_t stage) noexcept
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!g_ctx.local())
		return;

	auto active_weapon = g_ctx.local()->m_hActiveWeapon();


	post_data_update_start(g_ctx.local());

	custom_models();

	player_info_t player_info;


	if (!g_ctx.local()->is_alive()) //-V807
	{
		UpdateRequired = false;
		hudUpdateRequired = false;
		return;
	}

	glovchanger();

	SkinChanger::InitCustomModels();

	auto my_weapons = g_ctx.local()->m_hMyWeapons();

	auto model_bayonet = "models/weapons/v_knife_bayonet.mdl";
	auto model_m9 = "models/weapons/v_knife_m9_bay.mdl";
	auto model_karambit = "models/weapons/v_knife_karam.mdl";
	auto model_bowie = "models/weapons/v_knife_survival_bowie.mdl";
	auto model_butterfly = "models/weapons/v_knife_butterfly.mdl";
	auto model_falchion = "models/weapons/v_knife_falchion_advanced.mdl";
	auto model_flip = "models/weapons/v_knife_flip.mdl";
	auto model_gut = "models/weapons/v_knife_gut.mdl";
	auto model_huntsman = "models/weapons/v_knife_tactical.mdl";
	auto model_shadow_daggers = "models/weapons/v_knife_push.mdl";
	auto model_navaja = "models/weapons/v_knife_gypsy_jackknife.mdl";
	auto model_stiletto = "models/weapons/v_knife_stiletto.mdl";
	auto model_talon = "models/weapons/v_knife_widowmaker.mdl";
	auto model_ursus = "models/weapons/v_knife_ursus.mdl";
	auto model_nomad = "models/weapons/v_knife_outdoor.mdl";
	auto model_skeleton = "models/weapons/v_knife_skeleton.mdl";
	auto model_survival = "models/weapons/v_knife_canis.mdl";
	auto model_paracord = "models/weapons/v_knife_cord.mdl";
	auto model_css = "models/weapons/v_knife_css.mdl";

	auto index_bayonet = m_modelinfo()->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	auto index_m9 = m_modelinfo()->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	auto index_karambit = m_modelinfo()->GetModelIndex("models/weapons/v_knife_karam.mdl");
	auto index_bowie = m_modelinfo()->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
	auto index_butterfly = m_modelinfo()->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	auto index_falchion = m_modelinfo()->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	auto index_flip = m_modelinfo()->GetModelIndex("models/weapons/v_knife_flip.mdl");
	auto index_gut = m_modelinfo()->GetModelIndex("models/weapons/v_knife_gut.mdl");
	auto index_huntsman = m_modelinfo()->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	auto index_shadow_daggers = m_modelinfo()->GetModelIndex("models/weapons/v_knife_push.mdl");
	auto index_navaja = m_modelinfo()->GetModelIndex("models/weapons/v_knife_gypsy_jackknife.mdl");
	auto index_stiletto = m_modelinfo()->GetModelIndex("models/weapons/v_knife_stiletto.mdl");
	auto index_talon = m_modelinfo()->GetModelIndex("models/weapons/v_knife_widowmaker.mdl");
	auto index_ursus = m_modelinfo()->GetModelIndex("models/weapons/v_knife_ursus.mdl");
	auto index_nomad = m_modelinfo()->GetModelIndex("models/weapons/v_knife_outdoor.mdl");
	auto index_skeleton = m_modelinfo()->GetModelIndex("models/weapons/v_knife_skeleton.mdl");
	auto index_survival = m_modelinfo()->GetModelIndex("models/weapons/v_knife_canis.mdl");
	auto index_paracord = m_modelinfo()->GetModelIndex("models/weapons/v_knife_cord.mdl");
	auto index_css = m_modelinfo()->GetModelIndex("models/weapons/v_knife_css.mdl");

	auto model_blood = "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl";
	auto model_sport = "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl";
	auto model_slick = "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl";
	auto model_leath = "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl";
	auto model_moto = "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl";
	auto model_speci = "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl";
	auto model_hydra = "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl";
	auto model_fang = "models/weapons/v_models/arms/anarchist/v_glove_anarchist.mdl";

	auto index_blood = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
	auto index_sport = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
	auto index_slick = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
	auto index_leath = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
	auto index_moto = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
	auto index_speci = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));
	auto index_hydra = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl"));
	auto index_fang = m_modelinfo()->GetModelIndex(("models/weapons/v_models/arms/anarchist/v_glove_anarchist.mdl"));



	for (int i = 0; my_weapons[i] != INVALID_EHANDLE_INDEX; i++) {
		auto weapon_atr = reinterpret_cast<attributableitem_t*>(m_entitylist()->GetClientEntityFromHandle(my_weapons[i]));
		auto weapon = reinterpret_cast<attributableitem_t*>(m_entitylist()->GetClientEntityFromHandle(my_weapons[i]));

		if (!weapon)
			return;
		if (!c_config::get()->b["skins_knifechanger"])
			return;

		//apply knife model
		if (active_weapon->GetClientClass()->m_ClassID == ClassID::CKnife) {
			switch (c_config::get()->i["skins_knifechanger_items"]) {
			case 0:
				break;
			case 1:
				apply_knife_model(weapon, model_bayonet);
				break;
			case 2:
				apply_knife_model(weapon, model_m9);
				break;
			case 3:
				apply_knife_model(weapon, model_karambit);
				break;
			case 4:
				apply_knife_model(weapon, model_bowie);
				break;
			case 5:
				apply_knife_model(weapon, model_butterfly);
				break;
			case 6:
				apply_knife_model(weapon, model_falchion);
				break;
			case 7:
				apply_knife_model(weapon, model_flip);
				break;
			case 8:
				apply_knife_model(weapon, model_gut);
				break;
			case 9:
				apply_knife_model(weapon, model_huntsman);
				break;
			case 10:
				apply_knife_model(weapon, model_shadow_daggers);
				break;
			case 11:
				apply_knife_model(weapon, model_navaja);
				break;
			case 12:
				apply_knife_model(weapon, model_stiletto);
				break;
			case 13:
				apply_knife_model(weapon, model_talon);
				break;
			case 14:
				apply_knife_model(weapon, model_ursus);
				break;
				case 15:
					apply_knife_model(weapon, model_nomad);
					break;
				case 16:
					apply_knife_model(weapon, model_skeleton);
					break;
				case 17:
					apply_knife_model(weapon, model_survival);
					break;
				case 18:
					apply_knife_model(weapon, model_paracord);
					break;
				case 19:
					apply_knife_model(weapon, model_css);
					break;
			}
		}

		//apply knife skins
		//if (active_weapon->GetClientClass()->m_ClassID == ClassID::CKnife) {
		//	switch (c_config::get()->i["skins_knifechanger_items"]) {
		//	case 0:
		//		break;
		//	case 1:
		//		apply_knife_skin(weapon, WEAPON_BAYONET, CMenu::get()->index_knife, index_bayonet);
		//		break;
		//	case 2:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_M9_BAYONET, CMenu::get()->index_knife, index_m9);
		//		break;
		//	case 3:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_KARAMBIT, CMenu::get()->index_knife, index_karambit);
		//		break;
		//	case 4:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_SURVIVAL_BOWIE, CMenu::get()->index_knife, index_bowie);
		//		break;
		//	case 5:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_BUTTERFLY, CMenu::get()->index_knife, index_butterfly);
		//		break;
		//	case 6:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_FALCHION, CMenu::get()->index_knife, index_falchion);
		//		break;
		//	case 7:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_FLIP, CMenu::get()->index_knife, index_flip);
		//		break;
		//	case 8:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_GUT, CMenu::get()->index_knife, index_gut);
		//		break;
		//	case 9:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_TACTICAL, CMenu::get()->index_knife, index_huntsman);
		//		break;
		//	case 10:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_PUSH, CMenu::get()->index_knife, index_shadow_daggers);
		//		break;
		//	case 11:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_GYPSY_JACKKNIFE, CMenu::get()->index_knife, index_navaja);
		//		break;
		//	case 12:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_STILETTO, CMenu::get()->index_knife, index_stiletto);
		//		break;
		//	case 13:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_WIDOWMAKER, CMenu::get()->index_knife, index_talon);
		//		break;
		//	case 14:
		//		apply_knife_skin(weapon, WEAPON_KNIFE_URSUS, CMenu::get()->index_knife, index_ursus);
		//		break;


	}

	static auto backup_model_index = -1;

	//const char** player_model_index_knife = nullptr;
	//auto player_model = 0;

	//player_model_index_knife = player_model_index_t;
	//player_model_knife = c_config::get()->i["skins_knifechanger"];
//	break;



	if (c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
	{
		const char** player_model_index = nullptr;
		auto player_model = 0;

		switch (g_ctx.local()->m_iTeamNum())
		{
		case 2:
			player_model_index = player_models;
			player_model = c_config::get()->i["skin_changer_model_agent"];
			break;
		case 3:
			player_model_index = player_models;
			player_model = c_config::get()->i["skin_changer_model_agent"];
			break;
		}

		if (player_model)
		{
			if (!g_ctx.globals.backup_model)
			{
				auto model = g_ctx.local()->GetModel();

				if (model)
				{
					auto studio_model = m_modelinfo()->GetStudioModel(model);

					if (studio_model)
					{
						auto name = crypt_str("models/") + (std::string)studio_model->szName;
						backup_model_index = m_modelinfo()->GetModelIndex(name.c_str());
					}
				}
			}

			if (SkinChanger::player_model_indexes.find(player_model_index[player_model - 1]) == SkinChanger::player_model_indexes.end()) //-V522
				SkinChanger::player_model_indexes.emplace(player_model_index[player_model - 1], m_modelinfo()->GetModelIndex(player_model_index[player_model - 1]));

			g_ctx.local()->set_model_index(SkinChanger::player_model_indexes[player_model_index[player_model - 1]]);
			g_ctx.globals.backup_model = true;
		}
		else if (g_ctx.globals.backup_model)
		{
			g_ctx.local()->set_model_index(backup_model_index);
			g_ctx.globals.backup_model = false;
		}
	}

	if (UpdateRequired)
	{
		UpdateRequired = false;
		hudUpdateRequired = true;

		m_clientstate()->iDeltaTick = -1;
		g_ctx.globals.updating_skins = true;
	}

	else if (hudUpdateRequired && !g_ctx.globals.updating_skins)
	{
		hudUpdateRequired = false;
		//updateHud();
	}
}

void SkinChanger::scheduleHudUpdate() noexcept
{
	if (!g_ctx.local()->is_alive())
		return;

	if (m_globals()->m_realtime - last_skins_update < 1.0f)
		return;

	UpdateRequired = true;
	last_skins_update = m_globals()->m_realtime;
}

void SkinChanger::overrideHudIcon(IGameEvent* event) noexcept
{
	if (auto iconOverride = iconOverrides[event->GetString(crypt_str("weapon"))])
		event->SetString(crypt_str("weapon"), iconOverride);
}

item_setting* get_by_definition_index(const int definition_index)
{
	return nullptr;
}
