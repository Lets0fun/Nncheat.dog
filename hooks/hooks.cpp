// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hooks.hpp"

#include <tchar.h>
#include <iostream>
#include <d3d9.h>
#include <dinput.h>

#include "..\cheats\misc\logs.h"
#include "..\cheats\misc\misc.h"
#include "..\cheats\visuals\other_esp.h"
#include "..\utils\Render.h"
#include "..\cheats\visuals\radar.h"
#include "../Menu/Menu.h"
#include "../Menu/MenuFramework/Renderer.h"
#include "../Menu/MenuFramework/Framework.h"
using namespace IdaLovesMe;

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment(lib, "freetype.lib")

#include <shlobj.h>
#include <shlwapi.h>
#include <thread>
#include "..\cheats\menu_alpha.h"
#include "../Bytesa.h"

auto _visible = true;
static auto d3d_init = false;

namespace INIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool MouseDown[5];
bool KeysDown[512];

bool IsAnyMouseDown()
{
	for (int n = 0; n < sizeof(MouseDown); n++)
		if (MouseDown[n])
			return true;
	return false;
}

namespace hooks
{
	int rage_weapon = 0;
	int legit_weapon = 0;
	bool menu_open = false;
	bool input_shouldListen = false;

	ButtonCode_t* input_receivedKeyval;

	LRESULT Hooked_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int dbutton = 0;
		int ubutton = 0;
		switch (msg)
		{
		case WM_LBUTTONDOWN:
			dbutton = 0; break;
		case WM_RBUTTONDOWN:
			dbutton = 1; break;
		case WM_MBUTTONDOWN:
			dbutton = 2; break;
		case WM_XBUTTONDOWN:
		{
			if (LOWORD(wParam) == MK_XBUTTON1)
				dbutton = 3;
			if (LOWORD(wParam) == MK_XBUTTON2)
				dbutton = 4;

			if (!IsAnyMouseDown() && GetCapture() == NULL)
				SetCapture(hwnd);

			MouseDown[dbutton] = true;
			return true;
		}
		case WM_LBUTTONUP:
			ubutton = 0;
		case WM_RBUTTONUP:
			ubutton = 1;
		case WM_MBUTTONUP:
			ubutton = 2;
		case WM_XBUTTONUP:
		{
			if (LOWORD(wParam) == MK_XBUTTON1)
				ubutton = 3;
			if (LOWORD(wParam) == MK_XBUTTON2)
				ubutton = 4;

			MouseDown[ubutton] = false;
			if (!IsAnyMouseDown() && GetCapture() == hwnd)
				ReleaseCapture();
			return true;
		}
		case WM_MOUSEWHEEL:
			return true;
		case WM_MOUSEMOVE:
			return false;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			if (wParam < 256)
				KeysDown[wParam] = 1;

			/*if (wParam == g_cfg.misc.menu_key) {
				menu_open = !menu_open;
				CMenu::get()->set_menu_opened(!CMenu::get()->is_menu_opened());
				input_shouldListen = menu_open;

			}*/

			return false;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (wParam < 256)
				KeysDown[wParam] = 0;

			return false;
		case WM_CHAR:
			return true;
		case WM_SETCURSOR:

			if (LOWORD(lParam) == HTCLIENT)
				return 1;

			return false;
		}
		return 0;
	}

	LRESULT Hooked_WndProc2nd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int dbutton = 0;
		int ubutton = 0;
		switch (msg)
		{
		case WM_LBUTTONDOWN:
			dbutton = 0; break;
		case WM_RBUTTONDOWN:
			dbutton = 1; break;
		case WM_MBUTTONDOWN:
			dbutton = 2; break;
		case WM_XBUTTONDOWN:
		{
			return true;
		}
		case WM_LBUTTONUP:
			ubutton = 0;
		case WM_RBUTTONUP:
			ubutton = 1;
		case WM_MBUTTONUP:
			ubutton = 2;
		case WM_XBUTTONUP:
		{
			return true;
		}
		case WM_MOUSEWHEEL:
			return true;
		case WM_MOUSEMOVE:
			return false;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {

			//if (wParam == g_cfg.misc.menu_key) {
				//menu_open = !menu_open;
				//CMenu::get()->set_menu_opened(!CMenu::get()->is_menu_opened());
				//input_shouldListen = menu_open;
			//}

			return false;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
			return false;
		case WM_CHAR:
			return true;
		case WM_SETCURSOR:

			if (LOWORD(lParam) == HTCLIENT)
				return 1;

			return false;
		}
		return 0;
	}

	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		static auto original_fn = directx_hook->get_func_address <EndSceneFn>(42);
		return original_fn(pDevice);
	}

	void GUI_Init(IDirect3DDevice9* pDevice)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(INIT::Window);
		ImGui_ImplDX9_Init(pDevice);

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		style.WindowMinSize = ImVec2(10, 10);

		ImFontConfig m_config;
		m_config.OversampleH = m_config.OversampleV = 3;
		m_config.PixelSnapH = false;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x044F, // Cyrillic
			0,
		};

		char windows_directory[MAX_PATH];
		GetWindowsDirectory(windows_directory, MAX_PATH);

		auto verdana_directory = (std::string)windows_directory + "\\Fonts\\Verdana.ttf";

		// Menu fonts
		c_menu::get().futura = io.Fonts->AddFontFromMemoryTTF((void*)neverpuk, sizeof(neverpuk), 15.f, &m_config, ranges);
		c_menu::get().futura_small = io.Fonts->AddFontFromMemoryTTF((void*)neverpuk, sizeof(neverpuk), 13.f, &m_config, ranges);
		c_menu::get().futura_large = io.Fonts->AddFontFromMemoryTTF((void*)neverpuk, sizeof(neverpuk), 20.f, &m_config, ranges);

		c_menu::get().super_ico = io.Fonts->AddFontFromMemoryTTF((void*)super_ico, sizeof(super_ico), 20.f, &m_config, ranges);
		c_menu::get().gotham = io.Fonts->AddFontFromMemoryTTF((void*)GothamPro, sizeof(GothamPro), 13.f, &m_config, ranges);
		c_menu::get().interfaces = io.Fonts->AddFontFromMemoryTTF((void*)interfaces, sizeof(interfaces), 15.f * 1.25, &m_config, ranges);
		c_menu::get().font = io.Fonts->AddFontFromMemoryTTF((void*)font, sizeof(font), 13.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());

		ImGui_ImplDX9_CreateDeviceObjects();
		d3d_init = true;
	}

	long __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region)
	{
		static auto original = directx_hook->get_func_address <PresentFn>(17);
		g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

		if (!d3d_init)
			GUI_Init(device);

		IDirect3DVertexDeclaration9* vertex_dec;
		device->GetVertexDeclaration(&vertex_dec);

		IDirect3DVertexShader9* vertex_shader;
		device->GetVertexShader(&vertex_shader);
		//drawhurblovesme

		//drawhurblovesme


		static auto wanted_ret_address = _ReturnAddress();
		if (_ReturnAddress() == wanted_ret_address)
		{
			Render::Draw->Init(device);
			playeresp::get( ).paint_traverse( );
			misc::get( ).spectators_list( );
			Render::Draw->Reset();

			if ( g_ctx.globals.loaded_script )
				for ( auto current : c_lua::get( ).hooks.getHooks( "on_paint" ) )
					current.func( );

			playeresp::get().esp_device = device;

			CMenu::get()->Initialize();
			CMenu::get()->Draw();	
		}

		Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		if (ui::GetCurrentWindow()->Flags == Flags_None) {
			for (const auto obj : DrawList::Drawlist) {
				if (obj.Type == DrawType_Text)
					Render::Draw->Text(obj.Text.c_str(), obj.Pos.x, obj.Pos.y, LEFT, obj.Font, false, obj.Color, obj.TextClipSize);
				else if (obj.Type == DrawType_FilledRect)
					Render::Draw->FilledRect(obj.Pos, obj.Size, obj.Color);
				else if (obj.Type == DrawType_Rect)
					Render::Draw->Rect(obj.Pos, obj.Size, 1, obj.Color, obj.Antialias);
				else if (obj.Type == DrawType_Gradient)
					Render::Draw->Gradient(obj.Pos, obj.Size, obj.Color, obj.OtherColor, obj.Vertical, obj.Antialias);
				else if (obj.Type == DrawType_Sprite)
					Render::Draw->Sprite(obj.Texture, obj.Pos, obj.Size, obj.Color);
			}

			DrawList::Drawlist.clear();
		}

		Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		g_Render->BeginScene(device);
		g_Render->EndScene();

		g_ctx.globals.device = device;

		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		device->SetVertexShader(vertex_shader);
		device->SetVertexDeclaration(vertex_dec);

		return original(device, src_rect, dest_rect, dest_wnd_override, dirty_region);
	}

	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto ofunc = directx_hook->get_func_address<EndSceneResetFn>(16);

		if (!d3d_init)
			return ofunc(pDevice, pPresentationParameters);

		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = ofunc(pDevice, pPresentationParameters);

		if (SUCCEEDED(hr))
			ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}

	DWORD original_getforeignfallbackfontname;
	DWORD original_setupbones;
	DWORD original_doextrabonesprocessing;
	DWORD original_standardblendingrules;
	DWORD original_updateclientsideanimation;
	DWORD original_physicssimulate;
	DWORD original_oClampBonesInBBox;
	DWORD original_modifyeyeposition;
	DWORD original_calcviewmodelbob;
	DWORD original_processinterpolatedlist;
	DWORD original_clmove;

	vmthook* directx_hook;
	vmthook* client_hook;
	vmthook* clientstate_hook;
	vmthook* engine_hook;
	vmthook* player_hook;
	vmthook* clientmode_hook;
	vmthook* inputinternal_hook;
	vmthook* renderview_hook;
	vmthook* panel_hook;
	vmthook* modelcache_hook;
	vmthook* materialsys_hook;
	vmthook* modelrender_hook;
	vmthook* surface_hook;
	vmthook* bspquery_hook;
	vmthook* prediction_hook;
	vmthook* trace_hook;
	vmthook* filesystem_hook;

	C_HookedEvents hooked_events;
}

void __fastcall hooks::hooked_setkeycodestate(void* thisptr, void* edx, ButtonCode_t code, bool bDown)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetKeyCodeState_t>(91);

	if (input_shouldListen && bDown)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, bDown);
}

void __fastcall hooks::hooked_setmousecodestate(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetMouseCodeState_t>(92);

	if (input_shouldListen && state == BUTTON_PRESSED)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, state);
}