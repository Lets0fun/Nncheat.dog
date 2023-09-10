#pragma once

#include "../includes.hpp"
#include "../sdk/Memory.h"
#include "../configs/configs.h"
#include "item_definitions.hpp"

struct DefItem_t
{
	DefItem_t( const char* displayName,
		const char* entityName,
		const char* entityModel,
		const char* killIcon = nullptr )
	{
		this->displayName = displayName;
		this->entityName = entityName;
		this->entityModel = entityModel;
		this->killIcon = killIcon;
	}

	const char* displayName = nullptr;
	const char* entityName = nullptr;
	const char* entityModel = nullptr;
	const char* killIcon = nullptr;
};


//const std::map<ItemDefinitionIndex, DefItem_t> ItemDefinitionIndexMap = {
//		//{ ItemDefinitionIndex::INVALID,						{ "<-Default->", "DEFAULT", "", "" } },
//		{ ItemDefinitionIndex::WEAPON_DEAGLE,				{ "#SFUI_WPNHUD_DesertEagle", "weapon_deagle", "models/weapons/v_pist_deagle.mdl", "deagle" } },
//		{ ItemDefinitionIndex::WEAPON_ELITE,				{ "#SFUI_WPNHUD_Elites", "weapon_elite", "models/weapons/v_pist_elite.mdl", "elite" } },
//		{ ItemDefinitionIndex::WEAPON_FIVESEVEN,			{ "#SFUI_WPNHUD_FiveSeven", "weapon_fiveseven", "models/weapons/v_pist_fiveseven.mdl", "fiveseven" } },
//		{ ItemDefinitionIndex::WEAPON_GLOCK,				{ "#SFUI_WPNHUD_Glock18", "weapon_glock", "models/weapons/v_pist_glock18.mdl", "glock" } },
//		{ ItemDefinitionIndex::WEAPON_AK47,					{ "#SFUI_WPNHUD_AK47", "weapon_ak47", "models/weapons/v_rif_ak47.mdl", "ak47" } },
//		{ ItemDefinitionIndex::WEAPON_AUG,					{ "#SFUI_WPNHUD_Aug", "weapon_aug", "models/weapons/v_rif_aug.mdl", "aug" } },
//		{ ItemDefinitionIndex::WEAPON_AWP,					{ "#SFUI_WPNHUD_AWP", "weapon_awp", "models/weapons/v_snip_awp.mdl", "awp" } },
//		{ ItemDefinitionIndex::WEAPON_FAMAS,				{ "#SFUI_WPNHUD_Famas", "weapon_famas", "models/weapons/v_rif_famas.mdl", "famas" } },
//		{ ItemDefinitionIndex::WEAPON_G3SG1,				{ "#SFUI_WPNHUD_G3SG1", "weapon_g3sg1", "models/weapons/v_snip_g3sg1.mdl", "g3sg1" } },
//		{ ItemDefinitionIndex::WEAPON_GALILAR,				{ "#SFUI_WPNHUD_GalilAR", "weapon_galilar", "models/weapons/v_rif_galilar.mdl", "galilar" } },
//		{ ItemDefinitionIndex::WEAPON_M249,					{ "#SFUI_WPNHUD_M249", "weapon_m249", "models/weapons/v_mach_m249para.mdl", "m249" } },
//		{ ItemDefinitionIndex::WEAPON_M4A1,					{ "#SFUI_WPNHUD_M4A1", "weapon_m4a1", "models/weapons/v_rif_m4a1.mdl", "m4a1" } },
//		{ ItemDefinitionIndex::WEAPON_MAC10,				{ "#SFUI_WPNHUD_MAC10", "weapon_mac10", "models/weapons/v_smg_mac10.mdl", "mac10" } },
//		{ ItemDefinitionIndex::WEAPON_P90,					{ "#SFUI_WPNHUD_P90", "weapon_p90", "models/weapons/v_smg_p90.mdl", "p90" } },
//		{ ItemDefinitionIndex::WEAPON_UMP45,				{ "#SFUI_WPNHUD_UMP45", "weapon_ump45", "models/weapons/v_smg_ump45.mdl", "ump45" } },
//		{ ItemDefinitionIndex::WEAPON_XM1014,				{ "#SFUI_WPNHUD_xm1014", "weapon_xm1014", "models/weapons/v_shot_xm1014.mdl", "xm1014" } },
//		{ ItemDefinitionIndex::WEAPON_BIZON,				{ "#SFUI_WPNHUD_Bizon", "weapon_bizon", "models/weapons/v_smg_bizon.mdl", "bizon" } },
//		{ ItemDefinitionIndex::WEAPON_MAG7,					{ "#SFUI_WPNHUD_Mag7", "weapon_mag7", "models/weapons/v_shot_mag7.mdl", "mag7" } },
//		{ ItemDefinitionIndex::WEAPON_NEGEV,				{ "#SFUI_WPNHUD_Negev", "weapon_negev", "models/weapons/v_mach_negev.mdl", "negev" } },
//		{ ItemDefinitionIndex::WEAPON_SAWEDOFF,				{ "#SFUI_WPNHUD_Sawedoff", "weapon_sawedoff", "models/weapons/v_shot_sawedoff.mdl", "sawedoff" } },
//		{ ItemDefinitionIndex::WEAPON_TEC9,					{ "#SFUI_WPNHUD_Tec9", "weapon_tec9", "models/weapons/v_pist_tec9.mdl", "tec9" } },
//		{ ItemDefinitionIndex::WEAPON_TASER,				{ "#SFUI_WPNHUD_Taser", "weapon_taser", "models/weapons/v_eq_taser.mdl", "taser" } },
//		{ ItemDefinitionIndex::WEAPON_HKP2000,				{ "#SFUI_WPNHUD_HKP2000", "weapon_hkp2000", "models/weapons/v_pist_hkp2000.mdl", "hkp2000" } },
//		{ ItemDefinitionIndex::WEAPON_MP7,					{ "#SFUI_WPNHUD_MP7", "weapon_mp7", "models/weapons/v_smg_mp7.mdl", "mp7" } },
//		{ ItemDefinitionIndex::WEAPON_MP9,					{ "#SFUI_WPNHUD_MP9", "weapon_mp9", "models/weapons/v_smg_mp9.mdl", "mp9" } },
//		{ ItemDefinitionIndex::WEAPON_NOVA,					{ "#SFUI_WPNHUD_Nova", "weapon_nova", "models/weapons/v_shot_nova.mdl", "nova" } },
//		{ ItemDefinitionIndex::WEAPON_P250,					{ "#SFUI_WPNHUD_P250", "weapon_p250", "models/weapons/v_pist_p250.mdl", "p250" } },
//		{ ItemDefinitionIndex::WEAPON_SCAR20,				{ "#SFUI_WPNHUD_SCAR20", "weapon_scar20", "models/weapons/v_snip_scar20.mdl", "scar20" } },
//		{ ItemDefinitionIndex::WEAPON_SG556,				{ "#SFUI_WPNHUD_SG556", "weapon_sg556", "models/weapons/v_rif_sg556.mdl", "sg556" } },
//		//{ ItemDefinitionIndex::WEAPON_SSG08,				{ "#SFUI_WPNHUD_SSG08", "weapon_ssg08", "models/weapons/v_snip_ssg08.mdl", "ssg08" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE,				{ "#SFUI_WPNHUD_Knife", "weapon_knife", "models/weapons/v_knife_default_ct.mdl", "knife_default_ct" } },
//		{ ItemDefinitionIndex::WEAPON_FLASHBANG,			{ "#SFUI_WPNHUD_FLASHBANG", "weapon_flashbang", "models/weapons/v_eq_flashbang.mdl", "flashbang" } },
//		{ ItemDefinitionIndex::WEAPON_HEGRENADE,			{ "#SFUI_WPNHUD_HE_Grenade", "weapon_hegrenade", "models/weapons/v_eq_fraggrenade.mdl", "hegrenade" } },
//		{ ItemDefinitionIndex::WEAPON_SMOKEGRENADE,			{ "#SFUI_WPNHUD_Smoke_Grenade", "weapon_smokegrenade", "models/weapons/v_eq_smokegrenade.mdl", "smokegrenade" } },
//		{ ItemDefinitionIndex::WEAPON_MOLOTOV,				{ "#SFUI_WPNHUD_MOLOTOV", "weapon_molotov", "models/weapons/v_eq_molotov.mdl", "inferno" } },
//		{ ItemDefinitionIndex::WEAPON_DECOY,				{ "#SFUI_WPNHUD_DECOY", "weapon_decoy", "models/weapons/v_eq_decoy.mdl", "decoy" } },
//		{ ItemDefinitionIndex::WEAPON_INCGRENADE,			{ "#SFUI_WPNHUD_IncGrenade", "weapon_incgrenade", "models/weapons/v_eq_incendiarygrenade.mdl", "inferno" } },
//		//{ ItemDefinitionIndex::WEAPON_C4,					{ "#SFUI_WPNHUD_C4", "weapon_c4", "models/weapons/v_ied.mdl" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_T,				{ "#SFUI_WPNHUD_Knife", "weapon_knife_t", "models/weapons/v_knife_default_t.mdl", "knife_t" } },
//		{ ItemDefinitionIndex::WEAPON_M4A1_SILENCER,		{ "#SFUI_WPNHUD_M4_SILENCER", "weapon_m4a1_silencer", "models/weapons/v_rif_m4a1_s.mdl", "m4a1_silencer" } },
//		{ ItemDefinitionIndex::WEAPON_USP_SILENCER,			{ "#SFUI_WPNHUD_USP_SILENCER", "weapon_usp_silencer", "models/weapons/v_pist_223.mdl", "usp_silencer" } },
//		{ ItemDefinitionIndex::WEAPON_CZ75A,				{ "#SFUI_WPNHUD_CZ75", "weapon_cz75a", "models/weapons/v_pist_cz_75.mdl", "cz75a" } },
//		{ ItemDefinitionIndex::WEAPON_REVOLVER,				{ "#SFUI_WPNHUD_REVOLVER", "weapon_revolver", "models/weapons/v_pist_revolver.mdl", "revolver" } },
//		{ ItemDefinitionIndex::WEAPON_BAYONET,				{ "#SFUI_WPNHUD_KnifeBayonet", "weapon_knife_bayonet", "models/weapons/v_knife_bayonet.mdl", "bayonet" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_FLIP,			{ "#SFUI_WPNHUD_KnifeFlip", "weapon_knife_flip", "models/weapons/v_knife_flip.mdl", "knife_flip" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_GUT,			{ "#SFUI_WPNHUD_KnifeGut", "weapon_knife_gut", "models/weapons/v_knife_gut.mdl", "knife_gut" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_KARAMBIT,		{ "#SFUI_WPNHUD_KnifeKaram", "weapon_knife_karambit", "models/weapons/v_knife_karam.mdl", "knife_karambit" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_M9_BAYONET,		{ "#SFUI_WPNHUD_KnifeM9", "weapon_knife_m9_bayonet", "models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_TACTICAL,		{ "#SFUI_WPNHUD_KnifeTactical", "weapon_knife_tactical", "models/weapons/v_knife_tactical.mdl", "knife_tactical" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_FALCHION,		{ "#SFUI_WPNHUD_knife_falchion_advanced", "weapon_knife_falchion", "models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_SURVIVAL_BOWIE,	{ "#SFUI_WPNHUD_knife_survival_bowie", "weapon_knife_survival_bowie", "models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_BUTTERFLY,		{ "#SFUI_WPNHUD_Knife_Butterfly", "weapon_knife_butterfly", "models/weapons/v_knife_butterfly.mdl", "knife_butterfly" } },
//		{ ItemDefinitionIndex::WEAPON_KNIFE_PUSH,			{ "#SFUI_WPNHUD_knife_push", "weapon_knife_push", "models/weapons/v_knife_push.mdl", "knife_push" } },
//		{ ItemDefinitionIndex::STUDDED_BLOODHOUND_GLOVES,	{ "#CSGO_Wearable_t_studdedgloves", "studded_bloodhound_gloves", "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl" } },
//		{ ItemDefinitionIndex::T_GLOVES,					{ "#CSGO_Wearable_t_defaultgloves", "t_gloves", "models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl" } },
//		{ ItemDefinitionIndex::CT_GLOVES,					{ "#CSGO_Wearable_ct_defaultgloves", "ct_gloves", "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl" } },
//		{ ItemDefinitionIndex::SPORTY_GLOVES,				{ "#CSGO_Wearable_v_sporty_glove", "sporty_gloves", "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" } },
//		{ ItemDefinitionIndex::SLICK_GLOVES,				{ "#CSGO_Wearable_v_slick_glove", "slick_gloves", "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl" } },
//		{ ItemDefinitionIndex::LEATHER_HANDWRAPS,			{ "#CSGO_Wearable_v_leather_handwrap", "leather_handwraps", "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl" } },
//		{ ItemDefinitionIndex::MOTORCYCLE_GLOVES,			{ "#CSGO_Wearable_v_motorcycle_glove", "motorcycle_gloves", "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl" } },
//		{ ItemDefinitionIndex::SPECIALIST_GLOVES,			{ "#CSGO_Wearable_v_specialist_glove", "specialist_gloves", "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl" } }
//};

namespace SkinChanger
{
	void InitCustomModels();
	void InitCustomModels_ct();
	void glovchanger() noexcept;
	inline bool precached_on_round_start;
	void run(ClientFrameStage_t stage) noexcept;
	void scheduleHudUpdate() noexcept;
	void mask_changer(int stage);
	bool should_precache(const char* name);
	void overrideHudIcon(IGameEvent* event) noexcept;

	namespace Gloves
	{
		extern bool Enable;
		extern int TGlove;
		extern int CTGlove;
		extern std::vector<std::string> GloveOptions;
		extern std::vector<std::vector<std::string>> SkinsByGlove;
		extern int TSkin;
		extern std::vector<std::string> TSkinOptions;
		extern int CTSkin;
		extern std::vector<std::string> CTSkinOptions;
	}

	struct PaintKit
	{
		int id;
		std::string name;
		std::string skin_name;

		PaintKit(int id, std::string&& name, std::string&& skin_name) noexcept : id(id), name(name), skin_name(skin_name)
		{

		}

		auto operator<(const PaintKit& other) const noexcept
		{
			return name < other.name;
		}
	};

	struct EconomyItemCfg {
		int iItemDefinitionIndex = 0;
		int nFallbackPaintKit = 0;
		int nFallbackSeed = 0;
		int nFallbackStatTrak = -1;
		int iEntityQuality = 4;
		char* szCustomName = nullptr;
		float flFallbackWear = 0.1f;
	};

	extern std::unordered_map <std::string, int> model_indexes;
	extern std::unordered_map <std::string, int> player_model_indexes;

	extern std::vector <PaintKit> skinKits;
	extern std::vector <PaintKit> gloveKits;
	extern std::vector <PaintKit> displayKits;

	extern bool updating_skins;
}




struct item_setting
{
	void update()
	{
		itemId = game_data::weapon_names[itemIdIndex].definition_index;
		quality = game_data::quality_names[entity_quality_vector_index].index;

		const std::vector <SkinChanger::PaintKit>* kit_names;
		const game_data::weapon_name* defindex_names;

		if (itemId == T_GLOVES)
		{
			kit_names = &SkinChanger::gloveKits;
			defindex_names = game_data::glove_names;
		}
		else
		{
			kit_names = &SkinChanger::skinKits;
			defindex_names = game_data::knife_names;
		}

		paintKit = (*kit_names)[paint_kit_vector_index].id;
		definition_override_index = defindex_names[definition_override_vector_index].definition_index;
		skin_name = (*kit_names)[paint_kit_vector_index].skin_name;
	}

	int itemIdIndex = 0;
	int itemId = 1;
	int entity_quality_vector_index = 0;
	int quality = 0;
	int paint_kit_vector_index = 0;
	int paintKit = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	int stat_trak = 0;
	float wear = 0.0f;
	char custom_name[24] = "\0";
	std::string skin_name;
};

item_setting* get_by_definition_index(const int definition_index);

struct Skins_t
{
	bool rare_animations;
	std::map<int, item_setting> skinChanger;
	std::string custom_name_tag[36];
};

namespace G
{
	extern float Matrix[4][4];
}