// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "player_esp.h"
#include "..\misc\misc.h"
#include "..\ragebot\aim.h"
#include "dormant_esp.h"
#include "..\..\utils\Render.h"
#include "../../Configuration/Config.h"
#include "../../Menu/MenuFramework/Framework.h"
#include "../../Menu/Menu.h"
#include "../../Menu/MenuFramework/Renderer.h"
auto cfgsys = c_config::get();

using namespace IdaLovesMe;


class RadarPlayer_t
{
public:
	Vector pos; //0x0000
	Vector angle; //0x000C
	Vector spotted_map_angle_related; //0x0018
	DWORD tab_related; //0x0024
	char pad_0x0028[0xC]; //0x0028
	float spotted_time; //0x0034
	float spotted_fraction; //0x0038
	float time; //0x003C
	char pad_0x0040[0x4]; //0x0040
	__int32 player_index; //0x0044
	__int32 entity_index; //0x0048
	char pad_0x004C[0x4]; //0x004C
	__int32 health; //0x0050
	char name[32]; //0x785888
	char pad_0x0074[0x75]; //0x0074
	unsigned char spotted; //0x00E9
	char pad_0x00EA[0x8A]; //0x00EA
};

class CCSGO_HudRadar
{
public:
	char pad_0x0000[0x14C];
	RadarPlayer_t radar_info[65];
};

void misc::Spoof(player_t* pLocal, int Level)
{
	static DWORD DT_CSPlayerResource = NULL;

	if (DT_CSPlayerResource == NULL)
		DT_CSPlayerResource = util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(5).c_str()) + 0x2;

	if (!DT_CSPlayerResource)
		return;

	DWORD ptrResource = **(DWORD**)DT_CSPlayerResource;
	DWORD m_nPersonaDataPublicLevel = (DWORD)ptrResource + 0x4dd4 + (pLocal->EntIndex() * 4);

	*(PINT)((DWORD)m_nPersonaDataPublicLevel) = Level;
}

void playeresp::paint_traverse()
{
	static auto alpha = 1.0f;
	c_dormant_esp::get().start();

	if (g_cfg.player.arrows && g_ctx.local()->is_alive())
	{
		static auto switch_alpha = false;

		if (alpha <= 0.0f || alpha >= 1.0f)
			switch_alpha = !switch_alpha;

		alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
		alpha = math::clamp(alpha, 0.0f, 1.0f);
	}

	static auto FindHudElement = (DWORD(__thiscall*)(void*, const char*))util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	static auto hud_ptr = *(DWORD**)(util::FindSignature(crypt_str("client.dll"), crypt_str("81 25 ? ? ? ? ? ? ? ? 8B 01")) + 0x2);

	auto radar_base = FindHudElement(hud_ptr, "CCSGO_HudRadar");
	auto hud_radar = (CCSGO_HudRadar*)(radar_base - 0x14);

	for (auto i = 1; i < m_globals()->m_maxclients; i++) //-V807
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		//playerlist		

		if (!e->valid(false, false))
			continue;

		if (g_cfg.player_list.disable_visuals[e->EntIndex()])
			return;

		auto type = ENEMY;

		if (e == g_ctx.local())
			type = LOCAL;
		else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
			type = TEAM;


		auto valid_dormant = false;
		auto backup_flags = e->m_fFlags();
		auto backup_origin = e->GetAbsOrigin();


		if (e->IsDormant())
			valid_dormant = c_dormant_esp::get().adjust_sound(e);
		else
		{
			health[i] = e->m_iHealth();
			c_dormant_esp::get().m_cSoundPlayers[i].reset(true, e->GetAbsOrigin(), e->m_fFlags());
		}

		if (radar_base && hud_radar && e->IsDormant() && e->m_iTeamNum() != g_ctx.local()->m_iTeamNum() && e->m_bSpotted())
			health[i] = hud_radar->radar_info[i].health;

		if (!health[i])
		{
			if (e->IsDormant())
			{
				e->m_fFlags() = backup_flags;
				e->set_abs_origin(backup_origin);
			}

			continue;
		}

		auto fast = 2.5f * m_globals()->m_frametime; //-V807
		auto slow = 0.25f * m_globals()->m_frametime;

		if (e->IsDormant())
		{
			auto origin = e->GetAbsOrigin();

			if (origin.IsZero())
				esp_alpha_fade[i] = 0.0f;
			else if (!valid_dormant && esp_alpha_fade[i] > 0.0f)
				esp_alpha_fade[i] -= slow;
			else if (valid_dormant && esp_alpha_fade[i] < 1.0f)
				esp_alpha_fade[i] += fast;
		}
		else if (esp_alpha_fade[i] < 1.0f)
			esp_alpha_fade[i] += fast;

		esp_alpha_fade[i] = math::clamp(esp_alpha_fade[i], 0.0f, 1.0f);

		if (c_config::get()->b["skeleton"])
		{
			Color colord
			{
				c_config::get()->c["skelecol"][0],
				c_config::get()->c["skelecol"][1],
				c_config::get()->c["skelecol"][2],
				c_config::get()->c["skelecol"][3]
			};
			colord.SetAlpha(min(255.0f * esp_alpha_fade[i], colord.a()));


			draw_skeleton(e, colord, e->m_CachedBoneData().Base());

		}

		Box box;

		if (util::get_bbox(e, box, true) && type == ENEMY)
		{
			draw_box(e, box);
			draw_name(e, box);

			auto& hpbox = hp_info[i];

			if (hpbox.hp == -1)
				hpbox.hp = math::clamp(health[i], 0, 100);
			else
			{
				auto hp = math::clamp(health[i], 0, 100);

				if (hp != hpbox.hp)
				{
					if (hpbox.hp > hp)
					{
						if (hpbox.hp_difference_time) //-V550
							hpbox.hp_difference += hpbox.hp - hp;
						else
							hpbox.hp_difference = hpbox.hp - hp;

						hpbox.hp_difference_time = m_globals()->m_curtime;
					}
					else
					{
						hpbox.hp_difference = 0;
						hpbox.hp_difference_time = 0.0f;
					}

					hpbox.hp = hp;
				}

				if (m_globals()->m_curtime - hpbox.hp_difference_time > 0.2f && hpbox.hp_difference)
				{
					auto difference_factor = 4.0f * m_globals()->m_frametime * hpbox.hp_difference;

					hpbox.hp_difference -= difference_factor;
					hpbox.hp_difference = math::clamp(hpbox.hp_difference, 0, 100);

					if (!hpbox.hp_difference)
						hpbox.hp_difference_time = 0.0f;
				}
			}

			draw_health(e, box, hpbox);
			draw_weapon(e, box, draw_ammobar(e, box));
			draw_flags(e, box);
			draw_multi_points(e);
		}

		if (type == ENEMY)
		{
			//draw_lines(e);

			if (type == ENEMY)
			{
				if (cfgsys->b["oof"] && g_ctx.local()->is_alive())
				{
					Color arrowscol
					{
						cfgsys->c["oofcoll"][0],
						cfgsys->c["oofcoll"][1],
						cfgsys->c["oofcoll"][2],
						cfgsys->c["oofcoll"][3]
					};

					auto color = arrowscol;
					color.SetAlpha((int)(min(255.0f * esp_alpha_fade[i] * alpha, color.a())));

					if (e->IsDormant())
						color = Color(130, 130, 130, (int)(255.0f * esp_alpha_fade[i]));

					misc::get().PovArrows(e, color);

						
				}
			}
		}

		if (e->IsDormant())
		{
			e->m_fFlags() = backup_flags;
			e->set_abs_origin(backup_origin);
		}
		if (m_engine()->IsInGame())
		{
			misc::get().Spoof(g_ctx.local(), 2244);
		}
	}
}

void playeresp::centerinds() {
	if (!c_config::get()->b["Info_bar"])
		return;

	static int x = 0;
	static int y = 0;

	if (!x || !y)
		m_engine()->GetScreenSize(x, y);

	Vector2D scr = Vector2D(x / 2, y);

	auto& auf = g_ctx.local()->m_vecAbsVelocity();
	int speed = auf.Length();

	auto nci = m_engine()->GetNetChannelInfo();
	auto ping = 1.f / nci->GetLatency(FLOW_INCOMING);

	auto var = 1.f / m_globals()->m_absoluteframetime - 1.f / m_globals()->m_frametime;

	auto fps = 1.f / m_globals()->m_absoluteframetime;

	Render::Draw->Text_test(std::to_string((int)ping).c_str(), scr.x - 120 - 50, y - 20, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(200, 255, 82, 255));
	Render::Draw->Text_test("ping", scr.x - 120 - 50 + 22, y - 18, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(255, 255, 255, 225));

	Render::Draw->Text_test(std::to_string((int)fps).c_str(), scr.x - 30 - 50, y - 20, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(200, 255, 82, 255));
	Render::Draw->Text_test("fps", scr.x - 30 - 50 + 26, y - 18, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(255, 255, 255, 225));

	Render::Draw->Text_test(std::to_string((int)var).c_str(), scr.x + 30, y - 20, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(200, 255, 82, 255));
	Render::Draw->Text_test("var", scr.x + 30 + 22, y - 18, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(255, 255, 255, 225));

	Render::Draw->Text_test(std::to_string((int)speed).c_str(), scr.x + 120, y - 20, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(200, 255, 82, 255));
	Render::Draw->Text_test("speed", scr.x + 120 + 22, y - 18, 0, Render::Fonts::SmallFont, false, D3DCOLOR_RGBA(255, 255, 255, 225));


	Render::Draw->Gradient(Vec2(scr.x - 120 - 50 - 20 - 180, y - 28), Vec2(370, 28), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, 200));
	Render::Draw->Gradient(Vec2(scr.x, y - 28), Vec2(420, 28), D3DCOLOR_RGBA(0, 0, 0, 200), D3DCOLOR_RGBA(0, 0, 0, 0));
}

void playeresp::draw_skeleton(player_t* e, Color color, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	auto type = ENEMY;

	if (e == g_ctx.local())
		type = LOCAL;
	else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
		type = TEAM;
	if (type == LOCAL)
		return;

	if (!cfgsys->b["teammates"] && type == TEAM)
		return;

	auto model = e->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto get_bone_position = [&](int bone) -> Vector
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	auto upper_direction = get_bone_position(7) - get_bone_position(6);
	auto breast_bone = get_bone_position(6) + upper_direction * 0.5f;

	for (auto i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->pBone(i);

		if (!bone)
			continue;

		if (bone->parent == -1)
			continue;

		if (!(bone->flags & BONE_USED_BY_HITBOX))
			continue;

		auto child = get_bone_position(i);
		auto parent = get_bone_position(bone->parent);

		auto delta_child = child - breast_bone;
		auto delta_parent = parent - breast_bone;

		if (delta_parent.Length() < 9.0f && delta_child.Length() < 9.0f)
			parent = breast_bone;

		if (i == 5)
			child = breast_bone;

		if (fabs(delta_child.z) < 5.0f && delta_parent.Length() < 5.0f && delta_child.Length() < 5.0f || i == 6)
			continue;

		auto schild = ZERO;
		auto sparent = ZERO;

		if (math::world_to_screen(child, schild) && math::world_to_screen(parent, sparent))
			render::get().dual_circle(schild.x, schild.y, 5, 2048, D3DCOLOR_RGBA(c_config::get()->c["visabcoll"][0], c_config::get()->c["visabcoll"][1], c_config::get()->c["visabcoll"][2], 1), D3DCOLOR_RGBA(c_config::get()->c["visabcoll"][0], c_config::get()->c["visabcoll"][1], c_config::get()->c["visabcoll"][2], 125), esp_device);
	}
}

void playeresp::draw_box(player_t* m_entity, const Box& box)
{
	if (!cfgsys->b["bounding"])
		return;

	auto alpha = 255.0f * esp_alpha_fade[m_entity->EntIndex()];
	auto outline_alpha = (int)(alpha * 0.6f);

	Color outline_color
	{
		0,
		0,
		0,
		outline_alpha
	};

	Color boxclr
	{
		cfgsys->c["boundingcoll"][0],
		cfgsys->c["boundingcoll"][1],
		cfgsys->c["boundingcoll"][2],
		cfgsys->c["boundingcoll"][3]
	};

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : boxclr;
	color.SetAlpha(min(alpha, color.a()));


	Render::Draw->Rect( Vec2( box.x - 1, box.y - 1 ), Vec2( box.w + 2, box.h +2 ), 0, D3DCOLOR_RGBA( outline_color.r(), outline_color.g(), outline_color.b(), outline_color.a() ) );
	Render::Draw->Rect( Vec2( box.x, box.y ), Vec2( box.w, box.h ), 0, D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	Render::Draw->Rect( Vec2( box.x + 1, box.y + 1 ), Vec2( box.w - 2, box.h - 2 ), 0, D3DCOLOR_RGBA( outline_color.r( ), outline_color.g( ), outline_color.b( ), outline_color.a( ) ) );
}

void playeresp::draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox)
{
	int v89 = min(100, m_entity->m_iHealth());
	int unk1 = (int)((box.h * v89) / 100);
	int clamped_player_health = min( m_entity->m_iHealth( ), 100 );

	int red = 80;
	int green = 255;

	if ( clamped_player_health >= 25 ) {
		if ( clamped_player_health <= 50 )
		{
			red = 215;
			green = 50;
		}
	}
	else
	{
		red = 255;
		green = 50;
	}


	int health = m_entity->m_iHealth();
	int max_health = 100;
	float health_ratio = health / (float)max_health;

	int offset = box.h - 2;
	offset -= (offset * health_ratio);

	D3DCOLOR color = m_entity->IsDormant() ? D3DCOLOR_RGBA(150, 150, 150, 200) : D3DCOLOR_RGBA( red, green, 0, 255 ) ;

	if ( c_config::get( )->b[ "healbar_custom_col" ] )
	{
		color = m_entity->IsDormant( ) ? D3DCOLOR_RGBA( 150, 150, 150, 200 ) : D3DCOLOR_RGBA( c_config::get( )->c[ "healbarcoll" ][0], c_config::get( )->c[ "healbarcoll" ][ 1 ], c_config::get( )->c[ "healbarcoll" ][ 2 ], 255 );
	}

	if (c_config::get()->b["healbar"])
	{
		Render::Draw->FilledRect(Vec2(box.x - 5, box.y), Vec2(4, box.h), D3DCOLOR_RGBA(0, 0, 0, 130));
		Render::Draw->FilledRect(Vec2(box.x - 4, box.y + 1 + offset), Vec2(2, box.h - 2 - offset), color);

		if ( health_ratio < 0.95 ) {
			if ( CMenu::get( )->GetDPINum( ) >= 1.25f ) {

				Render::Draw->Text_test( std::to_string( health ).c_str( ), box.x - 6, box.y + offset, 0, CMenu::get( )->GetFontDPI( ), false, D3DCOLOR_RGBA( 255, 255, 255, 200 ) );
			}
			else
			{
				Render::Draw->Text_test( std::to_string( health ).c_str( ), box.x - 6, box.y + offset, 0, Render::Fonts::SmallFont, true, D3DCOLOR_RGBA( 255, 255, 255, 200 ) );
			}
		}
	}	
}

bool playeresp::draw_ammobar(player_t* m_entity, const Box& box)
{
	if (!m_entity->is_alive())
		return false;

	if (!cfgsys->b["ammo"])
		return false;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return false;

	auto ammo = weapon->m_iClip1();

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	auto outline_alpha = (int)(alpha * 0.7f);
	auto inner_back_alpha = (int)(alpha * 0.6f);

	Color outline_color =
	{
		0,
		0,
		0,
		outline_alpha
	};

	Color inner_back_color =
	{
		0,
		0,
		0,
		inner_back_alpha
	};

	Color ammocol
	{
		cfgsys->c["ammocoll"][0],
		cfgsys->c["ammocoll"][1],
		cfgsys->c["ammocoll"][2],
		cfgsys->c["ammocoll"][3]
	};

	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : ammocol;

	color.SetAlpha(min(alpha, color.a()));

	Box n_box =
	{
		box.x,
		box.y + box.h + 3,
		box.w + 2,
		2
	};

	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	auto bar_width = ammo * box.w / weapon_info->iMaxClip1;
	auto reloading = false;

	auto animlayer = m_entity->get_animlayers()[1];

	if (animlayer.m_nSequence)
	{
		auto activity = m_entity->sequence_activity(animlayer.m_nSequence);

		reloading = activity == ACT_CSGO_RELOAD && animlayer.m_flWeight;

		if (reloading && animlayer.m_flCycle < 1.0f)
			bar_width = animlayer.m_flCycle * box.w;
	}
	Render::Draw->FilledRect( Vec2( n_box.x - 1, n_box.y - 1 ), Vec2( n_box.w, 4 ), D3DCOLOR_RGBA( inner_back_color.r(), inner_back_color.g(), inner_back_color.b(), inner_back_color.a() ) );
	Render::Draw->FilledRect( Vec2( n_box.x, n_box.y ), Vec2( n_box.w, bar_width ), D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );

	if ( weapon->m_iClip1( ) != weapon_info->iMaxClip1 && !reloading )
		Render::Draw->Text_test( std::to_string( ammo ).c_str( ), n_box.x + bar_width, n_box.y + 1, 1, Render::Fonts::SmallFont, true, D3DCOLOR_RGBA( text_color .r(), text_color.g(), text_color.b(), text_color.a() ) );

	return true;
}

void playeresp::draw_name(player_t* m_entity, const Box& box)
{
	if (!cfgsys->b["name"])
		return;

	static auto sanitize = [](char* name) -> std::string
	{
		name[127] = '\0';

		std::string tmp(name);

		if (tmp.length() > 20)
		{
			tmp.erase(20, tmp.length() - 20);
			tmp.append("...");
		}

		return tmp;
	};

	player_info_t player_info;

	Color Namecolor
	{
		cfgsys->c["namecoll"][0],
		cfgsys->c["namecoll"][1],
		cfgsys->c["namecoll"][2],
		cfgsys->c["namecoll"][3]
	};

	if (m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info))
	{
		auto name = sanitize(player_info.szName);

		//auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : Namecolor;
		//color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

		D3DCOLOR color = m_entity->IsDormant() ? D3DCOLOR_RGBA(150, 150, 150, 160) : D3DCOLOR_RGBA(255, 255, 255, c_config::get()->c["namecoll"][3]);

		Render::Draw->Text_test(name.c_str(), (box.x + box.w / 2), box.y - 12, 1, CMenu::get()->GetFontDPI(), false, color);
	}
}

void playeresp::draw_weapon(player_t* m_entity, const Box& box, bool space)
{
	if (!cfgsys->b["wepicon"] && !cfgsys->b["weptext"])
		return;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto pos = box.y + box.h + 2;

	if (space)
		pos += 5;

	Color wepcolor
	{
		cfgsys->c["wepcoll"][0],
		cfgsys->c["wepcoll"][1],
		cfgsys->c["wepcoll"][2],
		cfgsys->c["wepcoll"][3]
	};

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : wepcolor;
	color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

	if (cfgsys->b["weptext"])
	{
		Render::Draw->Text_test( weapon->get_name( ).c_str( ), box.x + box.w / 2, pos, 1 , Render::Fonts::SmallFont, true, D3DCOLOR_RGBA( color.r(), color.g(), color.b(), color.a() ));
		pos += 11;
	}

	if (cfgsys->b["wepicon"])
	{
		Render::Draw->Text_test( weapon->get_icon( ), box.x + box.w / 2, pos, 1, Render::Fonts::Undefeated, false, D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}
}

LPD3DXFONT font_return( )
{
	if ( CMenu::get( )->GetDPINum( ) >= 1.25f )
		return CMenu::get( )->GetFontDPI( );
	else
		return Render::Fonts::SmallFont;
}

void playeresp::draw_flags(player_t* e, const Box& box)
{
	std::vector<SideInfo> info;

	if( c_config::get()->b["money"] )
		info.push_back(SideInfo("$" + std::to_string(e->m_iAccount()), D3DCOLOR_RGBA( 153, 225, 1, 255 )));
	if (e->m_ArmorValue() <= 0)
		info.push_back(SideInfo("K", D3DCOLOR_RGBA(255, 255, 255, 220)));
	else		info.push_back(SideInfo("HK", D3DCOLOR_RGBA(255, 255, 255, 220)));
	if ( e->m_bIsScoped() )
		info.push_back(SideInfo("ZOOM", D3DCOLOR_RGBA(65, 169, 244, 220)));
	if ( e->m_bInBombZone() )
		info.push_back(SideInfo("C4", D3DCOLOR_RGBA(255, 0, 0, 220)));

	for (size_t i = 0; i < info.size(); i++) {
		SideInfo flag = info.at(i);
		D3DCOLOR color = e->IsDormant() ? D3DCOLOR_RGBA(150, 150, 150, 200) : flag.color;



		if ( CMenu::get( )->GetDPINum( ) >= 1.25f ) {
			Render::Draw->Text_test( flag.text.c_str( ), box.x + box.w + 2, box.y + 2 + 10 * i, 0, CMenu::get( )->GetFontDPI( ), false, color );
		}
		else
		{
			Render::Draw->Text_test( flag.text.c_str( ), box.x + box.w + 2, box.y + 2 + 10 * i, 0, Render::Fonts::SmallFont, true, color );
		}
	}
}

void playeresp::draw_lines(player_t* e)
{
	if (!g_cfg.player.type[type].snap_lines)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	Vector angle;

	if (!math::world_to_screen(e->GetAbsOrigin(), angle))
		return;

	auto color = e->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.type[type].snap_lines_color;
	color.SetAlpha(min(255.0f * esp_alpha_fade[e->EntIndex()], color.a()));

	render::get().line(width / 2, height, angle.x, angle.y, color);
}

void render::dual_circle(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2 = nullptr;
	std::vector <CUSTOMVERTEX2> circle(resolution + 2);

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0.0f;

	circle[0].rhw = 1.0f;
	circle[0].color = color2;

	for (auto i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - radius * sin(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0.0f;

		circle[i].rhw = 1.0f;
		circle[i].color = color;
	}

	device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX2), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, nullptr);

	if (!g_pVB2)
		return;

	void* pVertices;

	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX2), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX2));
	g_pVB2->Unlock();

	device->SetTexture(0, nullptr);
	device->SetPixelShader(nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX2));
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);

	g_pVB2->Release();
}

void playeresp::draw_multi_points(player_t* e)
{
	if (!cfgsys->b["rage_enabled"])
		return;

	if (!cfgsys->b["visaimbot"])
		return;

	if (!g_ctx.local()->is_alive()) //-V807
		return;

	if (g_ctx.local()->get_move_type() == MOVETYPE_NOCLIP)
		return;

	if (g_ctx.globals.current_weapon == -1)
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	auto records = &player_records[e->EntIndex()]; //-V826

	if (records->empty())
		return;

	auto record = &records->front();

	if (!record->valid(false))
		return;

	std::vector <scan_point> points; //-V826
	auto hitboxes = aim::get().get_hitboxes(record);

	for (auto& hitbox : hitboxes)
	{
		auto current_points = aim::get().get_points(record, hitbox, false);

		for (auto& point : current_points)
			points.emplace_back(point);
	}

	for (auto& point : points)
	{
		if (points.empty())
			break;

		if (point.hitbox == HITBOX_HEAD)
			continue;

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			if (point.point == it->point)
				continue;

			auto first_angle = math::calculate_angle(g_ctx.globals.eye_pos, point.point);
			auto second_angle = math::calculate_angle(g_ctx.globals.eye_pos, it->point);

			auto distance = g_ctx.globals.eye_pos.DistTo(point.point);
			auto fov = math::fast_sin(DEG2RAD(math::get_fov(first_angle, second_angle))) * distance;

			if (fov < 5.0f)
			{
				points.erase(it);
				break;
			}
		}
	}

	if (points.empty())
		return;

	for (auto& point : points)
	{
		Vector screen;

		if (!math::world_to_screen(point.point, screen))
			continue;

		render::get().dual_circle(screen.x - 1, screen.y - 1, 3, 2048, D3DCOLOR_RGBA(c_config::get()->c["visabcoll"][0], c_config::get()->c["visabcoll"][1], c_config::get()->c["visabcoll"][2], 1), D3DCOLOR_RGBA(c_config::get()->c["visabcoll"][0], c_config::get()->c["visabcoll"][1], c_config::get()->c["visabcoll"][2], 255), esp_device);
	}
}