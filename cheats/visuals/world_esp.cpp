// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <algorithm>
#include "world_esp.h"
#include "../grenade project/grenade_warning.h"
#include "../menu_alpha.h"

#include "../../lodepng/lodepng.h"
#include "png_data.h"

void worldesp::paint_traverse()
{
	skybox_changer();
	for (int i = 1; i <= m_entitylist()->GetHighestEntityIndex(); i++)  //-V807
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_player())
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		switch (client_class->m_ClassID)
		{
		case CEnvTonemapController:
			//world_modulation(e);
			break;
		case CInferno:
			molotov_timer(e);
			break;
		case CSmokeGrenadeProjectile:
			//smoke_timer(e);
			break;
		case CPlantedC4:
			//bomb_timer(e);
			break;
		case CC4:
			if (g_cfg.player.type[ENEMY].flags[FLAGS_C4] || g_cfg.player.type[TEAM].flags[FLAGS_C4] || g_cfg.player.type[LOCAL].flags[FLAGS_C4] || g_cfg.esp.bomb_timer)
			{
				auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(e->m_hOwnerEntity());

				if ((g_cfg.player.type[ENEMY].flags[FLAGS_C4] || g_cfg.player.type[TEAM].flags[FLAGS_C4] || g_cfg.player.type[LOCAL].flags[FLAGS_C4]) && owner->valid(false, false))
					g_ctx.globals.bomb_carrier = owner->EntIndex();
				else if (g_cfg.esp.bomb_timer && !owner->is_player())
				{
					auto screen = ZERO;

					if (math::world_to_screen(e->GetAbsOrigin(), screen))
						render::get().text(fonts[ESP], screen.x, screen.y, Color(215, 20, 20), HFONT_CENTERED_X | HFONT_CENTERED_Y, "BOMB");
				}
			}
			break;
		default:
			grenade_projectiles(e);

			if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor) //-V648
				dropped_weapons(e);

			break;
		}
	}
}

void DrawBeamPaw2(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS; //TE_BEAMPOINTS
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_pszHaloName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_flHaloScale = 0;//0
	beamInfo.m_flWidth = g_cfg.esp.proximity_tracers_width;//11
	beamInfo.m_flEndWidth = g_cfg.esp.proximity_tracers_width;//11
	beamInfo.m_flFadeLength = 1.0f;
	beamInfo.m_flAmplitude = 2.3;
	beamInfo.m_flBrightness = 255.f;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;//40
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 0.03f;
	beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM; //FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM

	Beam_t* myBeam = m_viewrenderbeams()->CreateBeamPoints(beamInfo);

	if (myBeam)
		m_viewrenderbeams()->DrawBeam(myBeam);
}
auto cfgg = c_config::get();
void worldesp::skybox_changer()
{
	static auto load_skybox = reinterpret_cast<void(__fastcall*)(const char*)>(util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45")));
	auto skybox_name = backup_skybox;

	switch (cfgg->i["skybox"])
	{
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	}

	skybox_name = c_config::get()->c["skycol"][0], c_config::get()->c["skycol"][1], c_config::get()->c["skycol"][2], c_config::get()->c["skycol"][3];

	static auto skybox_number = 0;
	static auto old_skybox_name = skybox_name;

	static auto color_r = (unsigned char)255;
	static auto color_g = (unsigned char)255;
	static auto color_b = (unsigned char)255;

	if (skybox_number != cfgg->b[ "skybox" ] )
	{
		changed = true;
		skybox_number = cfgg->b[ "skybox" ];
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != c_config::get( )->c[ "skycol" ][0])
	{
		changed = true;
		color_r = c_config::get( )->c[ "skycol" ][0];
	}
	else if (color_g != c_config::get( )->c[ "skycol" ][1])
	{
		changed = true;
		color_g = c_config::get( )->c[ "skycol" ][1];
	}
	else if (color_b != c_config::get( )->c[ "skycol" ][2])
	{
		changed = true;
		color_b = c_config::get( )->c[ "skycol" ][2];
	}

	if (changed)
	{
		changed = false;
		load_skybox(skybox_name.c_str());

		auto materialsystem = m_materialsystem();

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), crypt_str("SkyBox")))
				material->ColorModulate( c_config::get( )->c[ "skycol" ][0] / 255.0f, c_config::get( )->c[ "skycol" ][1] / 255.0f, c_config::get( )->c[ "skycol" ][2] / 255.0f);
		}
	}
}

void worldesp::fog_changer()
{
	static auto fog_override = m_cvar()->FindVar(crypt_str("fog_override")); //-V807

	if (!g_cfg.esp.fog)
	{
		if (fog_override->GetBool())
			fog_override->SetValue(FALSE);

		return;
	}

	if (!fog_override->GetBool())
		fog_override->SetValue(TRUE);

	static auto fog_start = m_cvar()->FindVar(crypt_str("fog_start"));

	if (fog_start->GetInt())
		fog_start->SetValue(0);

	static auto fog_end = m_cvar()->FindVar(crypt_str("fog_end"));

	if (fog_end->GetInt() != g_cfg.esp.fog_distance)
		fog_end->SetValue(g_cfg.esp.fog_distance);

	static auto fog_maxdensity = m_cvar()->FindVar(crypt_str("fog_maxdensity"));

	if (fog_maxdensity->GetFloat() != (float)g_cfg.esp.fog_density * 0.01f) //-V550
		fog_maxdensity->SetValue((float)g_cfg.esp.fog_density * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, "%i %i %i", g_cfg.esp.fog_color.r(), g_cfg.esp.fog_color.g(), g_cfg.esp.fog_color.b());

	static auto fog_color = m_cvar()->FindVar(crypt_str("fog_color"));

	if (strcmp(fog_color->GetString(), buffer_color)) //-V526
		fog_color->SetValue(buffer_color);
}

void worldesp::world_modulation(entity_t* entity)
{
	if (!g_cfg.esp.world_modulation)
		return;

	entity->set_m_bUseCustomBloomScale(TRUE);
	entity->set_m_flCustomBloomScale(g_cfg.esp.bloom * 0.01f);

	entity->set_m_bUseCustomAutoExposureMin(TRUE);
	entity->set_m_flCustomAutoExposureMin(g_cfg.esp.exposure * 0.001f);

	entity->set_m_bUseCustomAutoExposureMax(TRUE);
	entity->set_m_flCustomAutoExposureMax(g_cfg.esp.exposure * 0.001f);
}

void filled_circle1(int x, int y, int radius, int segments, Color color) {
	std::vector< Vertex_t > vertices;

	float step = M_PI * 2.0f / segments;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.emplace_back(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y));

	//m_surface()->DrawTexturedPolygon(vertices.size(), vertices, color);
}

std::uint8_t* load_image(unsigned short id, int width, int height, unsigned char* data_in, unsigned int size)
{
	std::vector< std::uint8_t > image;

	auto w = std::uint32_t(width);
	auto h = std::uint32_t(height);

	if (const auto error = lodepng::decode(image, w, h, data_in, size))
		exit(0);

	const auto data = new std::uint8_t[image.size()];
	std::copy(image.begin(), image.end(), data);

	return data;
}

unsigned int in_out_id{ 0 };
unsigned int out_in_id{ 0 };
unsigned int fire_icon_id{ 0 };
unsigned char* in_out_texture = nullptr;
unsigned char* out_in_texture = nullptr;
unsigned char* fire_icon_texture = nullptr;

void worldesp::molotov_timer(entity_t* entity)
{
	if (!c_config::get()->b["grenprox"])
		return;

	float icon_scale = 0.75;
	auto weapon = (weapon_t*)entity;
	auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

	int min_size = 32;
	int max_size = 70;
	int radius = min_size;

	if (distance < 25) {
		float scale = (float)(25 - distance) / (float)(25);
		radius = (int)(min_size + scale * (max_size - min_size));
	}

	int min_alpha = 0;
	int max_alpha = 255;
	int radius_alpha = min_alpha;

	if (distance < 25) {
		float scale = (float)(25 - distance) / (float)(25);
		radius_alpha = (int)(min_alpha + scale * (max_alpha - min_alpha));
	}

	auto inferno = reinterpret_cast<inferno_t*>(entity);
	auto& origin = inferno->GetAbsOrigin();

	Vector screen_origin, origin2;

	if (!math::world_to_screen(origin, screen_origin))
		return;

	if (distance > 32)
		return;

	static bool once = false;

	if (!in_out_texture)
		in_out_texture = load_image(1, 512, 512, png_data::in_out, 74031);
	if (!out_in_texture)
		out_in_texture = load_image(1, 512, 512, png_data::out_in, 34920);
	if (!fire_icon_texture)
		fire_icon_texture = load_image(1, 30, 29, png_data::fire_icon, 581);

	if (!once)
	{
		in_out_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(in_out_id, in_out_texture, 512, 512);
		out_in_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(out_in_id, out_in_texture, 512, 512);
		fire_icon_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(fire_icon_id, fire_icon_texture, 30, 29);
		once = true;
	}

	if (!m_surface()->IsTextureIDValid(in_out_id)) {
		in_out_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(in_out_id, in_out_texture, 512, 512);
	}

	if (!m_surface()->IsTextureIDValid(out_in_id)) {
		out_in_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(out_in_id, out_in_texture, 512, 512);
	}
	if (!m_surface()->IsTextureIDValid(fire_icon_id)) {
		fire_icon_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(fire_icon_id, fire_icon_texture, 30, 29);
	}

	auto spawn_time = inferno->get_spawn_time();

	auto factor = (spawn_time + inferno_t::get_expiry_time() - m_globals()->m_curtime) / inferno_t::get_expiry_time();

	Vector mins, maxs;
	inferno->GetClientRenderable()->GetRenderBounds(mins, maxs);

	render::get().circle_filled(screen_origin.x, screen_origin.y, 85, radius, Color(0, 0, 0));

	if (distance < 33)
	{
		m_surface()->DrawSetColor(Color(255, 0, 0, radius_alpha));
		m_surface()->DrawSetTexture(in_out_id);
		m_surface()->DrawTexturedRect(screen_origin.x - radius, screen_origin.y - radius, screen_origin.x + radius, screen_origin.y + radius);
		render::get().circle_filled(screen_origin.x, screen_origin.y, 85, radius, Color(0, 0, 0, 205));
		m_surface()->DrawSetTexture(0);
    }

	icon_scale = std::clamp( sqrt( distance * distance ) / 10 , 0.1f , 1.f );

	m_surface()->DrawSetColor(Color(205, 205, 205, 220));
	m_surface()->DrawSetTexture(fire_icon_id);
	m_surface()->DrawTexturedRect(screen_origin.x - (30 * icon_scale) / 2, screen_origin.y - (29 * icon_scale), screen_origin.x + (30 * icon_scale) / 2, screen_origin.y);
	m_surface( )->DrawSetTexture( 0 );

	render::get().DrawArc( Vector2D( screen_origin.x, screen_origin.y ), radius, factor * 2, Color( 205, 205, 205, 220 ), 2, 2048 );

	render::get().text(fonts[HE2], screen_origin.x, screen_origin.y + 6, Color(205, 205, 205), HFONT_CENTERED_X, "%i", (int)distance);
}

void worldesp::smoke_timer(entity_t* entity)
{
	if (!g_cfg.esp.smoke_timer)
		return;

	auto smoke = reinterpret_cast<smoke_t*>(entity);

	if (!smoke->m_nSmokeEffectTickBegin() || !smoke->m_bDidSmokeEffect())
		return;

	auto origin = smoke->GetAbsOrigin();

	Vector screen_origin;

	if (!math::world_to_screen(origin, screen_origin))
		return;

	auto spawn_time = TICKS_TO_TIME(smoke->m_nSmokeEffectTickBegin());
	auto factor = (spawn_time + smoke_t::get_expiry_time() - m_globals()->m_curtime) / smoke_t::get_expiry_time();

	static auto size = Vector2D(35.0f, 5.0f);
	render::get().circle_filled(screen_origin.x, screen_origin.y - size.y * 0.5f, 60, 20, Color(15, 15, 15, 255));

	static auto alpha = 2.0f;
	// render::get().Draw3DFilledCircle(origin, 100.0f, g_cfg.esp.smoke_timer_color);

	//render::get().CircularProgressBar(screen_origin.x, screen_origin.y - size.y * 0.5f, 17, 20, 0, 360 * factor, Color(255, 255, 255, 144), true);
	render::get().CircularProgressBar(screen_origin.x, screen_origin.y - size.y * 0.5f, 20, -90, (360 * factor), 2, Color(255, 255, 255, 144));

	render::get().text(fonts[GRENADES], screen_origin.x + 1.0f, screen_origin.y - size.y * 0.5f - 6.0f, Color(255, 255, 255, 144), HFONT_CENTERED_X | HFONT_CENTERED_Y, "k");

	auto weapon = (weapon_t*)entity; //-V1027
	auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

	render::get().text(fonts[ESP], screen_origin.x, screen_origin.y - size.y * 0.5f + 8.0f, Color(255, 255, 255), HFONT_CENTERED_X | HFONT_CENTERED_Y, "%i FT", (int)distance);
}

void worldesp::grenade_projectiles(entity_t* entity)
{
	if (!c_config::get()->b["grenprox"])
		return;

	c_grenade_prediction::get().grenade_warning((projectile_t*)entity);
}

void worldesp::bomb_timer(entity_t* entity)
{
	if (!g_cfg.esp.bomb_timer)
		return;

	if (!g_ctx.globals.bomb_timer_enable)
		return;

	static auto mp_c4timer = m_cvar()->FindVar(crypt_str("mp_c4timer"));
	auto bomb = (CCSBomb*)entity;

	auto c4timer = mp_c4timer->GetFloat();
	auto bomb_timer = bomb->m_flC4Blow() - m_globals()->m_curtime;

	if (bomb_timer < 0.0f)
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto factor = bomb_timer / c4timer * height;

	auto red_factor = (int)(255.0f - bomb_timer / c4timer * 255.0f);
	auto green_factor = (int)(bomb_timer / c4timer * 255.0f);

	render::get().rect_filled(0, height - factor, 26, factor, Color(red_factor, green_factor, 0, 100));

	auto text_position = height - factor + 11;

	if (text_position > height - 9)
		text_position = height - 9;

	render::get().text(fonts[ESP], 13, text_position, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, "%0.1f", bomb_timer);

	Vector screen;

	if (math::world_to_screen(entity->GetAbsOrigin(), screen))
		render::get().text(fonts[ESP], screen.x, screen.y, Color(red_factor, green_factor, 0), HFONT_CENTERED_X | HFONT_CENTERED_Y, "BOMB");
}

void worldesp::dropped_weapons(entity_t* entity)
{
	auto weapon = (weapon_t*)entity; //-V1027
	auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(weapon->m_hOwnerEntity());

	if (owner->is_player())
		return;

	Box box;

	//if (util::get_bbox(weapon, box, false))
	{
		auto offset = 0;

		if (g_cfg.esp.weapon[WEAPON_BOX])
		{
			render::get().rect(box.x, box.y, box.w, box.h, g_cfg.esp.box_color);

			if (g_cfg.esp.weapon[WEAPON_ICON])
			{
				render::get().text(fonts[SUBTABWEAPONS], box.x + box.w / 2, box.y - 14, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());
				offset = 14;
			}

			if (g_cfg.esp.weapon[WEAPON_TEXT])
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

			if (g_cfg.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				render::get().rect_filled(box.x - 1, box.y + box.h + 14, box.w + 2, 4, inner_back_color);
				render::get().rect_filled(box.x, box.y + box.h + 15, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, g_cfg.esp.weapon_ammo_color);
			}

			if (g_cfg.esp.weapon[WEAPON_DISTANCE])
			{
				auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y - 13 - offset, g_cfg.esp.weapon_color, HFONT_CENTERED_X, "%i FT", (int)distance);
			}
		}
		else
		{
			if (g_cfg.esp.weapon[WEAPON_ICON])
				render::get().text(fonts[SUBTABWEAPONS], box.x + box.w / 2, box.y + box.h / 2 - 7, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());

			if (g_cfg.esp.weapon[WEAPON_TEXT])
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 6, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

			if (g_cfg.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				static auto pos = 0;

				if (g_cfg.esp.weapon[WEAPON_ICON] && g_cfg.esp.weapon[WEAPON_TEXT])
					pos = 19;
				else if (g_cfg.esp.weapon[WEAPON_ICON])
					pos = 8;
				else if (g_cfg.esp.weapon[WEAPON_TEXT])
					pos = 19;

				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				render::get().rect_filled(box.x - 1, box.y + box.h / 2 + pos - 1, box.w + 2, 4, inner_back_color);
				render::get().rect_filled(box.x, box.y + box.h / 2 + pos, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, g_cfg.esp.weapon_ammo_color);
			}

			if (g_cfg.esp.weapon[WEAPON_DISTANCE])
			{
				auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

				if (g_cfg.esp.weapon[WEAPON_ICON] && g_cfg.esp.weapon[WEAPON_TEXT])
					offset = 21;
				else if (g_cfg.esp.weapon[WEAPON_ICON])
					offset = 21;
				else if (g_cfg.esp.weapon[WEAPON_TEXT])
					offset = 8;

				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 - offset, g_cfg.esp.weapon_color, HFONT_CENTERED_X, "%i FT", (int)distance);
			}
		}
	}
}