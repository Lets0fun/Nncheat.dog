// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "grenade_warning.h"
#include "../../lodepng/lodepng.h"
#include "..\visuals\png_data.h"

const char* index_to_grenade_name(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE: return "smoke"; break;
	case WEAPON_HEGRENADE: return "he grenade"; break;
	case WEAPON_MOLOTOV:return "molotov"; break;
	case WEAPON_INCGRENADE:return "molotov"; break;
	}
}
const char* index_to_grenade_name_icon(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE: return "k"; break;
	case WEAPON_HEGRENADE: return "j"; break;
	case WEAPON_MOLOTOV:return "l"; break;
	case WEAPON_INCGRENADE:return "n"; break;
	}
}

void rotate_point(Vector2D& point, Vector2D origin, bool clockwise, float angle) {
	Vector2D delta = point - origin;
	Vector2D rotated;

	if (clockwise) {
		rotated = Vector2D(delta.x * cosf(angle) - delta.y * sinf(angle), delta.x * sinf(angle) + delta.y * cosf(angle));
	}
	else {
		rotated = Vector2D(delta.x * sinf(angle) - delta.y * cosf(angle), delta.x * cosf(angle) + delta.y * sinf(angle));
	}

	point = rotated + origin;
}

void draw_arc(int x, int y, int radius, int start_angle, int percent, int thickness, Color color) {
	auto precision = (2 * M_PI) / 30;
	auto step = M_PI / 180;
	auto inner = radius - thickness;
	auto end_angle = (start_angle + percent) * step;
	auto start_angles = (start_angle * M_PI) / 180;

	for (; radius > inner; --radius) {
		for (auto angle = start_angles; angle < end_angle; angle += precision) {
			auto cx = std::round(x + radius * std::cos(angle));
			auto cy = std::round(y + radius * std::sin(angle));

			auto cx2 = std::round(x + radius * std::cos(angle + precision));
			auto cy2 = std::round(y + radius * std::sin(angle + precision));

			render::get().line(cx, cy, cx2, cy2, color);
		}
	}
}

//void textured_polygon(int n, std::vector< Vertex_t > vertice, Color color) {
//	static int texture_id = m_surface()->CreateNewTextureID(true);
//	static unsigned char buf[4] = { 255, 255, 255, 255 };
//
//	m_surface()->DrawSetTextureRGBA(texture_id, buf, 1, 1);
//	m_surface()->DrawSetColor(color);
//	m_surface()->DrawSetTexture(texture_id);
//	m_surface()->DrawTexturedPolygon(n, vertice.data());
//}

void filled_circle(int x, int y, int radius, int segments, Color color) {
	std::vector< Vertex_t > vertices;

	float step = M_PI * 2.0f / segments;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.emplace_back(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y));

	//textured_polygon(vertices.size(), vertices, color);
}

void DrawBeamPaw(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS; //TE_BEAMPOINTS
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_pszHaloName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_flHaloScale = 0;//0
	beamInfo.m_flWidth = 3;//11
	beamInfo.m_flEndWidth = 3;//11
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

std::uint8_t* load_image2(unsigned short id, int width, int height, unsigned char* data_in, unsigned int size)
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

inline float CSGO_Armor( float flDamage, int ArmorValue ) {
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if ( ArmorValue > 0 ) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if ( flArmor > static_cast< float >(ArmorValue) ) {
			flArmor = static_cast< float >(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}

unsigned int in2_out_id{ 0 };
unsigned int out2_in_id{ 0 };
unsigned int fire2_icon_id{ 0 };
unsigned char* in2_out_texture = nullptr;
unsigned char* out2_in_texture = nullptr;
unsigned char* fire2_icon_texture = nullptr;

bool c_grenade_prediction::data_t::draw(entity_t* entity) const
{
	if (m_path.size() <= 1u || m_globals()->m_curtime >= m_expire_time)
		return false;


	float distance = g_ctx.local()->m_vecOrigin().DistTo(m_origin) / 12;
	int min_size = 30;
	int max_size = 60;
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



	static bool once = false;


	if (!in2_out_texture)
		in2_out_texture = load_image2(1, 512, 512, png_data::in_out, 74031);
	if (!out2_in_texture)
		out2_in_texture = load_image2(1, 512, 512, png_data::out_in, 34920);

	if (!once)
	{
		in2_out_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(in2_out_id, in2_out_texture, 512, 512);
		out2_in_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(out2_in_id, out2_in_texture, 512, 512);
		once = true;
	}

	if (!m_surface()->IsTextureIDValid(in2_out_id)) {
		in2_out_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(in2_out_id, in2_out_texture, 512, 512);
	}

	if (!m_surface()->IsTextureIDValid(out2_in_id)) {
		out2_in_id = m_surface()->CreateNewTextureID(true);
		m_surface()->DrawSetTextureRGBA(out2_in_id, out2_in_texture, 512, 512);
	}

	static int iScreenWidth, iScreenHeight;
	m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

	auto prev_screen = ZERO;
	auto prev_on_screen = math::world_to_screen(std::get< Vector >(m_path.front()), prev_screen);

	for (auto i = 1u; i < m_path.size(); ++i)
	{
		auto cur_screen = ZERO, last_cur_screen = ZERO;
		const auto cur_on_screen = math::world_to_screen(std::get< Vector >(m_path.at(i)), cur_screen);

		auto color = g_cfg.esp.grenade_proximity_tracers_colors;

		if (g_cfg.esp.grenade_proximity_tracers_rainbow)
			color = Color::FromHSB((360 / m_path.size() * i) / 360.f, 0.9f, 0.8f);

		if (prev_on_screen && cur_on_screen)
		{
			DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), Color(c_config::get()->c["grentrajcoll"][0], c_config::get()->c["grentrajcoll"][1], c_config::get()->c["grentrajcoll"][2], c_config::get()->c["grentrajcoll"][3]));

		}
		Vector screen_origin;

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
		if (cur_on_screen && m_index == WEAPON_MOLOTOV || m_index == WEAPON_INCGRENADE || m_index == WEAPON_HEGRENADE) {
			static auto size = Vector2D(35.0f, 5.0f);
			render::get().text(fonts[NAME], screen_origin.x, screen_origin.y - size.y * 0.5f + 8.0f, Color(255, 255, 255), HFONT_CENTERED_X, "%i", (int)distance);
		}
	}

	if (distance > 70)
		return false;

	if (m_index != WEAPON_MOLOTOV)
	{
		static auto size = Vector2D(35.0f, 5.0f);
		render::get().text(fonts[NAME], prev_screen.x, prev_screen.y - size.y * 0.5f + 8.0f, Color(255, 255, 255), HFONT_CENTERED_X, "%i", (int)distance);

		float percent = ((m_expire_time - m_globals()->m_curtime) / TICKS_TO_TIME(m_tick));

		int alpha_damage = 0;

		if (m_index == WEAPON_HEGRENADE && distance <= 35)
		{
			alpha_damage = 255 - 255 * (distance / -2);
		}

		if ((m_index == WEAPON_MOLOTOV || m_index == WEAPON_INCGRENADE) && distance <= 25)
		{
			alpha_damage = 255 - 255 * (distance / -2);
		}




		auto is_on_screen = [](Vector origin, Vector& screen) -> bool
		{
			if (!math::world_to_screen(origin, screen))
				return false;

			auto xOk = iScreenWidth > screen.x;
			auto yOk = iScreenHeight > screen.y;

			return xOk && yOk;
		};

		Vector screenPos2;
		Vector vEnemyOrigin2 = m_origin;
		Vector vLocalOrigin2 = g_ctx.local()->GetAbsOrigin();

		if (!g_ctx.local()->is_alive())
			vLocalOrigin2 = m_input()->vecCameraOffset;

		//static auto size = Vector2D(35.0f, 5.0f);
		render::get().circle_filled(prev_screen.x, prev_screen.y - size.y * 0.5f, 460, radius, Color(0, 0, 0));

		if (distance < 25)
		{

			m_surface()->DrawSetColor(Color(255, 0, 0, radius_alpha));
			m_surface()->DrawSetTexture(in2_out_id);
			m_surface()->DrawTexturedRect(prev_screen.x - radius, prev_screen.y - radius, prev_screen.x + radius, prev_screen.y + radius);
			render::get().circle_filled(prev_screen.x, prev_screen.y, 460, radius, Color(0, 0, 0, radius_alpha));
			m_surface()->DrawSetTexture(0);
		}

		static int width, height;
		m_engine()->GetScreenSize(width, height);

		render::get().DrawArc(Vector2D(prev_screen.x, prev_screen.y - size.y * 0.5f ), radius - 1, percent * 2,  Color(255, 255, 255), 2, 460 );
		render::get().text(fonts[GRENADES], prev_screen.x + 1.0f, prev_screen.y - size.y * 0.5f - 9.0f, Color(255, 255, 255, 144), HFONT_CENTERED_X | HFONT_CENTERED_Y, "j");
		render::get().text(fonts[HE2], prev_screen.x, prev_screen.y + 6, Color(205, 205, 205), HFONT_CENTERED_X, "%i", (int) distance);
	}

	const float wm = iScreenWidth / 2, hm = iScreenHeight / 2;
	Vector last_pos = std::get< Vector >(m_path.at(m_path.size() - 1));

	Vector dir;

	m_engine()->GetViewAngles(dir);

	float view_angle = dir.y;

	if (view_angle < 0)
		view_angle += 360;

	Vector screenPos;
	Vector vEnemyOrigin = m_origin;
	Vector vLocalOrigin = g_ctx.local()->GetAbsOrigin();

	if (!g_ctx.local()->is_alive())
		vLocalOrigin = m_input()->vecCameraOffset;

	view_angle = DEG2RAD(view_angle);

	auto entity_angle = math::calculate_angle(vLocalOrigin, vEnemyOrigin);
	entity_angle.NormalizeInPlace();

	if (entity_angle.y < 0.f)
		entity_angle.y += 360.f;

	entity_angle.y = DEG2RAD(entity_angle.y);
	entity_angle.y -= view_angle;

	auto position = Vector2D(wm, hm);
	position.x -= math::clamp(vLocalOrigin.DistTo(vEnemyOrigin), 100.f, hm - 100);

	rotate_point(position, Vector2D(wm, hm), false, entity_angle.y);

	return true;
}

void c_grenade_prediction::grenade_warning(projectile_t* entity)
{
	auto& predicted_nades = c_grenade_prediction::get().get_list();

	static auto last_server_tick = m_globals()->m_curtime;
	if (last_server_tick != m_globals()->m_curtime) {
		predicted_nades.clear();

		last_server_tick = m_globals()->m_curtime;
	}

	if (entity->IsDormant() || !c_config::get()->b["grenprox"])
		return;

	const auto client_class = entity->GetClientClass();
	if (!client_class
		|| client_class->m_ClassID != 114 && client_class->m_ClassID != CBaseCSGrenadeProjectile)
		return;

	if (client_class->m_ClassID == CBaseCSGrenadeProjectile) {
		const auto model = entity->GetModel();
		if (!model)
			return;

		const auto studio_model = m_modelinfo()->GetStudioModel(model);
		if (!studio_model
			|| std::string_view(studio_model->szName).find("fraggrenade") == std::string::npos)
			return;
	}

	const auto handle = entity->GetRefEHandle().ToLong();

	if (entity->m_nExplodeEffectTickBegin()) {
		predicted_nades.erase(handle);

		return;
	}

	if (predicted_nades.find(handle) == predicted_nades.end()) {
		predicted_nades.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(handle),
			std::forward_as_tuple(
				entity->m_hThrower().Get(),
				client_class->m_ClassID == 114 ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
				entity->m_vecOrigin(), reinterpret_cast<player_t*>(entity)->m_vecVelocity(),
				entity->m_flSpawnTime(), TIME_TO_TICKS(reinterpret_cast<player_t*>(entity)->m_flSimulationTime() - entity->m_flSpawnTime())
			)
		);
	}

	if (predicted_nades.at(handle).draw(entity))
		return;

	predicted_nades.erase(handle);
}