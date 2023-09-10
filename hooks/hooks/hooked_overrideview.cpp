// This is an independent project of an individual developer. Dear PVS-Studio, please chec`q it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\lagcompensation\local_animations.h"

using OverrideView_t = void(__stdcall*)(CViewSetup*);

void thirdperson(bool fakeducking);

void __stdcall hooks::hooked_overrideview(CViewSetup* viewsetup)
{
	static auto original_fn = clientmode_hook->get_func_address <OverrideView_t> (18);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!viewsetup)
		return original_fn(viewsetup);

	if (g_ctx.local())
	{
		static auto fakeducking = false;

		if (!fakeducking && g_ctx.globals.fakeducking)
			fakeducking = true;
		else if (fakeducking && !g_ctx.globals.fakeducking && (!g_ctx.local()->get_animation_state()->m_fDuckAmount || g_ctx.local()->get_animation_state()->m_fDuckAmount == 1.0f)) //-V550
			fakeducking = false;

		if (!g_ctx.local()->is_alive()) //-V807
			fakeducking = false;

		auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

		
		if (!g_ctx.local()->m_bIsScoped() && c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
			viewsetup->fov = c_config::get()->i["misc_fov_over"];
		else if ( c_config::get( )->b[ "remscope" ] && c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
		{
			if (weapon->m_zoomLevel() == 1)
				viewsetup->fov = 90.0f + (float)c_config::get()->i["misc_over_zoom_fov"];
			else
				viewsetup->fov = (float)c_config::get()->i["misc_over_zoom_fov"];
		}

		if (weapon)
		{
			auto viewmodel = (entity_t*)m_entitylist()->GetClientEntityFromHandle(g_ctx.local()->m_hViewModel());

			if (viewmodel)
			{
				auto eyeAng = viewsetup->angles;
				eyeAng.z -= (float)g_cfg.esp.viewmodel_roll;

				viewmodel->set_abs_angles(eyeAng);
			}

			if (weapon->is_grenade() && c_config::get()->b["grenadetraj"] && c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
				GrenadePrediction::get().View(viewsetup, weapon);
                viewsetup->fov = c_config::get()->i["misc_fov_over"];
		}

		if (c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]) && c_config::get()->b["ftp"] && (c_config::get()->auto_check(c_config::get()->i["ftp_key"], c_config::get()->i["ftp_key_style"])))
			thirdperson(fakeducking);
		else
		{
			g_ctx.globals.in_thirdperson = false;
			m_input()->camera_third_person = false;
		}

		original_fn(viewsetup);

		if (fakeducking)
		{
			viewsetup->origin = g_ctx.local()->GetAbsOrigin() + Vector(0.0f, 0.0f, m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f);

			if (m_input()->camera_third_person)
			{
				auto camera_angles = Vector(m_input()->vecCameraOffset.x, m_input()->vecCameraOffset.y, 0.0f); //-V807
				auto camera_forward = ZERO;

				math::angle_vectors(camera_angles, camera_forward);
				math::VectorMA(viewsetup->origin, -m_input()->vecCameraOffset.z, camera_forward, viewsetup->origin);
			}
		}
	}
	else
		return original_fn(viewsetup);
}

void Thirdperson_Init(bool fakeducking, float progress) {
    /* our current fraction. */
    static float current_fraction = 0.0f;

  //  auto distance = ((float)g_cfg.misc.thirdperson_distance) * progress;
    auto distance = (75 * progress);
    //auto distance = ((float)c_config::get()->i["thirdperdist"] * progress);
    Vector angles, inverse_angles;

    // get camera angles.
    m_engine()->GetViewAngles(angles);
    m_engine()->GetViewAngles(inverse_angles);

    // cam_idealdist convar.
    inverse_angles.z = distance;

    // set camera direction.
    Vector forward, right, up;
    math::angle_vectors(inverse_angles, &forward, &right, &up);


    // various fixes to camera when fakeducking.
    auto eye_pos = fakeducking ? g_ctx.local()->GetAbsOrigin() + m_gamemovement()->GetPlayerViewOffset(false) : g_ctx.local()->GetAbsOrigin() + g_ctx.local()->m_vecViewOffset();
    auto offset = eye_pos + forward * -distance + right + up;

    // setup trace filter and trace.
    CTraceFilterWorldOnly filter;
    trace_t tr;

    // tracing to camera angles.
    m_trace()->TraceRay(Ray_t(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f)), 131083, &filter, &tr);

    // interpolate camera speed if something behind our camera.
    if (current_fraction > tr.fraction)
        current_fraction = tr.fraction;
    else if (current_fraction > 0.9999f)
        current_fraction = 1.0f;

    // adapt distance to travel time.
    current_fraction = math::interpolate(current_fraction, tr.fraction, m_globals()->m_frametime * 10.0f);
    angles.z = distance * current_fraction;



    // override camera angles.
    m_input()->vecCameraOffset = angles;
}

void thirdperson(bool fakeducking)
{
    /* thirdperson code. */
    {
        static float progress;
        static bool in_transition;
        static auto in_thirdperson = false;

        if (!in_thirdperson && g_ctx.globals.in_thirdperson)
        {
            in_thirdperson = true;
        }
        else if (in_thirdperson && !g_ctx.globals.in_thirdperson)
            in_thirdperson = false;

        if (g_ctx.local()->is_alive() && in_thirdperson)
        {
            in_transition = false;

           

            if (!m_input()->camera_third_person)
            {
                m_input()->camera_third_person = true;
            }
        }
        else
        {
            progress -= m_globals()->m_frametime * 8.f + (progress / 100);
            progress = std::clamp(progress, 0.f, 1.f);

            if (!progress)
                m_input()->camera_third_person = false;
            else
            {
                in_transition = true;
                m_input()->camera_third_person = true;
            }
        }

        if (m_input()->camera_third_person && !in_transition)
        {
            progress += m_globals()->m_frametime * 8.f + (progress / 100);
            progress = std::clamp(progress, 0.f, 1.f);



        }

        Thirdperson_Init(fakeducking, progress);
    }

    /* thirdperson on death code. */
    {
        static auto require_reset = false;

        if (g_ctx.local()->is_alive())
        {
            require_reset = false;
            return;
        }

        if (c_config::get()->b["ftp2"])
        {
            if (require_reset)
                g_ctx.local()->m_iObserverMode() = OBS_MODE_CHASE;

            if (g_ctx.local()->m_iObserverMode() == OBS_MODE_IN_EYE)
                require_reset = true;
        }
    }
}

