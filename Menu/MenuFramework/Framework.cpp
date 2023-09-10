#include "Framework.h"
#include "Renderer.h"
#include "../../includes.hpp"
#include "..\cheats\misc\logs.h"
#include <algorithm>
#pragma warning(disable : 4996)
#include <time.h>
#include "../Menu.h"
using namespace std;

using namespace IdaLovesMe;

GuiContext* IdaLovesMe::Gui_Ctx = NULL;

std::vector<DrawList::RenderObject> DrawList::Drawlist;

void DrawList::AddText(const char* text, int x, int y, D3DCOLOR Color, LPD3DXFONT font, bool bordered, Vec2 TextClipSize) {
	auto obj = new RenderObject(DrawType_Text, Vec2(x, y), Vec2(0, 0), Color, NULL, text, font, bordered, false, TextClipSize, false, NULL);

	DrawList::Drawlist.push_back(*obj);
}

void DrawList::AddSprite(LPDIRECT3DTEXTURE9 Texture, Vec2 Pos, Vec2 Size, D3DCOLOR Color)
{
	auto obj = new RenderObject(DrawType_Sprite, Pos, Size, Color, NULL, NULL, NULL, false, false, Vec2(0, 0), false, Texture);

	DrawList::Drawlist.push_back(*obj);
}

void DrawList::AddFilledRect(Vec2 Pos, Vec2 Size, D3DCOLOR Color) {
	auto obj = new RenderObject(DrawType_FilledRect, Pos, Size, Color, NULL, NULL, NULL, false, false, Vec2(0, 0), false, NULL);

	DrawList::Drawlist.push_back(*obj);
}

void DrawList::AddRect(Vec2 Pos, Vec2 Size, D3DCOLOR Color, bool Antialias) {
	auto obj = new RenderObject(DrawType_Rect, Pos, Size, Color, NULL, NULL, NULL, false, false, Vec2(0, 0), Antialias, NULL);

	DrawList::Drawlist.push_back(*obj);
}

void DrawList::AddGradient(Vec2 Pos, Vec2 Size, D3DCOLOR Color, D3DCOLOR OtherColor, bool Vertical, bool Antialias) {
	auto obj = new RenderObject(DrawType_Gradient, Pos, Size, Color, OtherColor, NULL, NULL, false, Vertical, Vec2(0, 0), Antialias, NULL);

	DrawList::Drawlist.push_back(*obj);
}

void ui::Shutdown(GuiContext* context) {
	GuiContext& g = *context;

	if (!g.Initialized)
		return;

	for (std::size_t i = 0; i < g.Windows.size(); i++)
		delete g.Windows[i];
	g.Windows.clear();
	g.WindowsByName.clear();

	g.Initialized = false;
}

void ui::Init(GuiContext* context) {
	GuiContext& g = *context;

	g.NextWindowInfo.PosCond = false;
	g.NextWindowInfo.SizeCond = false;

	g.Initialized = true;
}

GuiContext* ui::CreateContext() {
	GuiContext* ctx = new GuiContext;
	if (Gui_Ctx == NULL)
		Gui_Ctx = ctx;
	Init(ctx);
	return ctx;
}

void ui::DeleteContext(GuiContext* ctx) {
	if (ctx == NULL)
		ctx = Gui_Ctx;
	Shutdown(ctx);
	if (Gui_Ctx == ctx)
		Gui_Ctx = NULL;
	delete ctx;
}

GuiWindow* ui::FindWindowByName(const char*& name) {
	GuiContext& g = *Gui_Ctx;
	auto it = std::find(g.WindowsByName.begin(), g.WindowsByName.end(), name);
	int index;

	if (it != g.WindowsByName.end())
		index = it - g.WindowsByName.begin();
	else
		return NULL;

	return g.Windows.at(index);
}

static GuiWindow* ui::CreateNewWindow(const char*& name, Vec2 size, GuiFlags flags) {
	
	GuiContext& g = *Gui_Ctx;

	GuiWindow* window = new GuiWindow;

	window->Init = false;
	window->Name = name;

	window->Pos = Vec2(0, 0);
	window->Size = size;
	window->CursorPos = Vec2(0, 0);
	window->PrevCursorPos = Vec2(0, 0);
	window->ScrollRatio = 0.f;

	window->Opened = false;
	window->Resizing = false;
	window->Dragging = false;
	window->Block = false;
	window->ParentWindow = NULL;
	window->ChildWindows;

	g.Windows.push_back(window);
	g.WindowsByName.push_back(name);

	return window;
}

void ui::GetInputFromWindow(const std::string& window_name) {
	GuiContext& g = *Gui_Ctx;

	POINT p_mouse_pos;
	GetCursorPos(&p_mouse_pos);
	ScreenToClient(FindWindowA(nullptr, window_name.data()), &p_mouse_pos);

	for (int i = 0; i < 256; i++) {
		g.PrevKeyState[i] = g.KeyState[i];
		g.KeyState[i] = GetAsyncKeyState(i);
	}

	g.PrevMousePos = g.MousePos;
	g.MousePos = Vec2{ (float)p_mouse_pos.x, (float)p_mouse_pos.y };
}

void ui::SetCurrentWindow(GuiWindow* window) {
	GuiContext& g = *Gui_Ctx;
	g.CurrentWindow = window;
}

GuiWindow* ui::GetCurrentWindow() {
	GuiContext& g = *Gui_Ctx;
	if (IdaLovesMe::Gui_Ctx)
		return g.CurrentWindow;
	return NULL;
}

bool ui::ChildsAreStable(GuiWindow* Window) {
	for (std::size_t i = 0; i < Window->ChildWindows.size(); i++) {
		if (Window->ChildWindows[i]->Dragging || Window->ChildWindows[i]->Resizing)
			return false;
	}
	return true;
}

void ui::SetNextWindowPos(const Vec2& pos) {
	GuiContext& g = *Gui_Ctx;
	g.NextWindowInfo.Pos = pos;
	g.NextWindowInfo.PosCond = true;
}

void ui::SetNextWindowSize(const Vec2& size) {
	GuiContext& g = *Gui_Ctx;
	g.NextWindowInfo.Size = size;
	g.NextWindowInfo.SizeCond = true;

}

Vec2 ui::GetWindowPos() {
	return GetCurrentWindow()->Pos;
}

Vec2 ui::GetWindowSize() {
	GuiWindow* window = GetCurrentWindow();
	return window->Size;
}




bool ui::KeyPressed(const int key) {
	if (!GetActiveWindow() && !m_engine()->IsConnected() || CMenu::get()->PopUpOpen)
		return false;
	GuiContext& g = *Gui_Ctx;
	return g.KeyState[key] && !g.PrevKeyState[key];
}

bool ui::KeyPressedIgnore(const int key) {
	if (!GetActiveWindow() && !m_engine()->IsConnected())
		return false;
	GuiContext& g = *Gui_Ctx;
	return g.KeyState[key] && !g.PrevKeyState[key];
}

bool ui::KeyDown(const int key) {
	if (!GetActiveWindow() && !m_engine()->IsConnected())
		return false;
	GuiContext& g = *Gui_Ctx;
	return g.KeyState[key];
}

bool ui::KeyDownMenu(const int key) {
	if (!GetActiveWindow() && !m_engine()->IsConnected())
		return false;
	GuiContext& g = *Gui_Ctx;
	return g.KeyState[key];
}

bool ui::KeyReleased(const int key) {
	if (!GetActiveWindow() && !m_engine()->IsConnected())
		return false;
	GuiContext& g = *Gui_Ctx;
	return !g.KeyState[key] && g.PrevKeyState[key];
}

bool ui::IsInside(const float x, const float y, const float w, const float h) {
	/*if (!GetActiveWindow())
		return false;*/
	GuiContext& g = *Gui_Ctx;
	return g.MousePos.x > x && g.MousePos.y > y && g.MousePos.x < w + x && g.MousePos.y < h + y;
}

bool ui::IsInsideWindow(GuiWindow* Window) {
	if (!Gui_Ctx->Initialized)
		return false;

	if (!Window)
		Window = Gui_Ctx->Windows.at(0);

	/*if (!Window)
		return false;*/

	return IsInside(Window->Pos.x, Window->Pos.y, Window->Size.x, Window->Size.y);
}

bool ui::IsInsideIgnore(const float x, const float y, const float w, const float h) {
	if (!GetActiveWindow() && !m_engine()->IsConnected())
		return false;
	GuiContext& g = *Gui_Ctx;
	return g.MousePos.x > x && g.MousePos.y > y && g.MousePos.x < w + x && g.MousePos.y < h + y;
}

void ui::HandleMoving(GuiWindow* Window, Rect Boundaries, Vec2* buffer) {
	
	GuiContext& g = *Gui_Ctx;

	if (!(Window->Flags & Flags_ChildWindow)) {

		if (g.DontMove)
			return;
		if (g.Blocking)
			return;
		if (g.IsClicking)
			return;

		if (IsInside(Window->Pos.x, Window->Pos.y, Window->Size.x, Window->Size.y) && KeyDown(VK_LBUTTON) && !Window->Dragging && !IsInside(Window->Pos.x + Window->Size.x - 15, Window->Pos.y + Window->Size.y - 15, 15, 15) && !Window->Resizing && !Window->Block && ChildsAreStable(Window))
			Window->Dragging = true;

		else if (KeyDown(VK_LBUTTON) && Window->Dragging && !(Window->Flags & Flags_NoMove)) {
			const Vec2 mouse_delta{ g.MousePos.x - g.PrevMousePos.x , g.MousePos.y - g.PrevMousePos.y };
			const Vec2 new_position{ Window->Pos.x + mouse_delta.x, Window->Pos.y + mouse_delta.y };

			Window->Pos = Vec2(std::clamp(new_position.x, Boundaries.Min.x, Boundaries.Max.x), std::clamp(new_position.y, Boundaries.Min.y, Boundaries.Max.y));
		}

		else if (!KeyDown(VK_LBUTTON) && Window->Dragging)
			Window->Dragging = false;
	}
	else if ((Window->Flags & Flags_ChildWindow) && buffer) {
		static Vec2 ClickPos;


		const Vec2 MinSize = Vec2(ui::GetWindowSize().x * 0.25f - 46.f - CMenu::get()->GetDPITab(), false ? 0.1672f * ui::GetWindowSize().y - 39.f : 0.1664f * ui::GetWindowSize().y - 25.f);
		const Vec2 Step = Vec2((Boundaries.Max.x - MinSize.x - 2.f) / 9, (Boundaries.Max.y - MinSize.y - 2) / 10);

		if (IsInside(Window->Pos.x, Window->Pos.y, Window->Size.x, 17) && KeyDown(VK_LBUTTON) && !Window->Dragging && !Window->Resizing && !Window->ParentWindow->Resizing && !Window->ParentWindow->Dragging && ui::ChildsAreStable(Window->ParentWindow)) {

			ClickPos.x = g.MousePos.x - Window->Pos.x;
			Window->Dragging = true;
		}

		else if (KeyDown(VK_LBUTTON) && Window->Dragging && !(Window->Flags & Flags_NoMove) && !CMenu::get()->LockMenu) {
			Window->xPos = (int)round((g.MousePos.x - buffer->x - ClickPos.x) / Step.x);
			Window->yPos = (int)round((g.MousePos.y - buffer->y) / Step.y);

			Window->xPos = std::clamp(Window->xPos, 0, 9 - Window->xSize);
			Window->yPos = std::clamp(Window->yPos, 0, 10 - Window->ySize);

			WritePrivateProfileStringA(Window->Name.c_str(), "posx", std::to_string(Window->xPos).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(Window->Name.c_str(), "posy", std::to_string(Window->yPos).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}

		else if (!KeyDown(VK_LBUTTON) && Window->Dragging)
			Window->Dragging = false;

		buffer->x += Window->xPos * Step.x;
		buffer->y += Window->yPos * Step.y;
	}
}

void ui::HandleResize(GuiWindow* Window, Rect Boundaries, Vec2* buffer) {
	GuiContext& g = *Gui_Ctx;



	if (!(Window->Flags & Flags_ChildWindow)) {

		if (g.DontMove)
			return;
		if (g.Blocking)
			return;

		if (IsInside(Window->Pos.x + Window->Size.x - 15, Window->Pos.y + Window->Size.y - 15, 15, 15) && KeyDown(VK_LBUTTON) && !CMenu::get()->PopUpOpen && !Window->Resizing && !Window->Dragging && !CMenu::get()->LockMenu)
			Window->Resizing = true;
		else if (KeyDown(VK_LBUTTON) && Window->Resizing && !(Window->Flags & Flags_NoMove)) {
			Window->Size.x = std::clamp(g.MousePos.x - Window->Pos.x, Boundaries.Min.x, Boundaries.Max.x);
			Window->Size.y = std::clamp(g.MousePos.y - Window->Pos.y, Boundaries.Min.y, Boundaries.Max.y);

			WritePrivateProfileStringA(Window->Name.c_str(), "sizex", std::to_string(Window->xSize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(Window->Name.c_str(), "sizey", std::to_string(Window->ySize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}
		else if (!ui::KeyDown(VK_LBUTTON) && Window->Resizing)
			Window->Resizing = false;
	}
	else if ((Window->Flags & Flags_ChildWindow) && buffer) {


		const Vec2 MinSize = Vec2(ui::GetWindowSize().x * 0.25f - 46.f - CMenu::get()->GetDPITab(), false ? 0.1672f * ui::GetWindowSize().y - 39.f : 0.1664f * ui::GetWindowSize().y - 25.f);
		const Vec2 Step = Vec2((Boundaries.Max.x - MinSize.x - 2.f) / 9, (Boundaries.Max.y - MinSize.y - 2) / 10);

		if (!CMenu::get()->LockMenu && IsInside(Window->Pos.x + Window->Size.x - 15, Window->Pos.y + Window->Size.y - 15, 15, 15) && KeyDown(VK_LBUTTON) && !CMenu::get()->PopUpOpen && !Window->Resizing && !Window->Dragging && !Window->ParentWindow->Resizing && ui::ChildsAreStable(Window->ParentWindow))
			Window->Resizing = true;

		else if (KeyDown(VK_LBUTTON) && Window->Resizing && !(Window->Flags & Flags_NoMove) && !CMenu::get()->LockMenu) {
			if (g.MousePos.x >= Window->Pos.x + Window->Size.x + Step.x)
				Window->xSize = (int)round((g.MousePos.x - Window->Pos.x - MinSize.x) / Step.x);
			else if (g.MousePos.x <= Window->Pos.x + Window->Size.x - Step.x)
				Window->xSize = (int)round((g.MousePos.x - Window->Pos.x - MinSize.x) / Step.x);

			Window->xSize = std::clamp(Window->xSize, 0, 9 - Window->xPos);

			if (g.MousePos.y >= Window->Pos.y + Window->Size.y + Step.y)
				Window->ySize = (int)round((g.MousePos.y - Window->Pos.y - MinSize.y) / Step.y);
			else if (g.MousePos.y <= Window->Pos.y + Window->Size.y - Step.y)
				Window->ySize = (int)round((g.MousePos.y - Window->Pos.y - MinSize.y) / Step.y);

			Window->ySize = std::clamp(Window->ySize, 0, 10 - Window->yPos);


			WritePrivateProfileStringA(Window->Name.c_str(), "sizex", std::to_string(Window->xSize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(Window->Name.c_str(), "sizey", std::to_string(Window->ySize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}
		else if (!KeyDown(VK_LBUTTON) && Window->Resizing)
			Window->Resizing = false;

		*buffer = Vec2(MinSize.x + (Step.x * Window->xSize), MinSize.y + (Step.y * Window->ySize));
	}
}

void ui::Begin(const char* Name, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	static Vec2 size_on_first_use;
	GuiWindow* Window = FindWindowByName(Name);

	if (Window == NULL) {
		if (g.NextWindowInfo.SizeCond)
			size_on_first_use = g.NextWindowInfo.Size;
		else
			size_on_first_use = Vec2(10, 10);
		Window = CreateNewWindow(Name, size_on_first_use, flags);
	}

	if (g.NextWindowInfo.PosCond) {
		Window->Pos = g.NextWindowInfo.Pos;
		g.NextWindowInfo.PosCond = false;
	}

	if ((Window->Flags & Flags_ChildWindow) || (Window->Flags & Flags_PopUp) && g.NextWindowInfo.SizeCond) {
		Window->Size = g.NextWindowInfo.Size;
		g.NextWindowInfo.SizeCond = false;
	}

	if (!(Window->Flags & Flags_ChildWindow) && !ui::ChildsAreStable(Window))
		Window->Block = true;
	else
		Window->Block = false;

	if (flags & Flags_NoMove)
		Window->Dragging = false;

	if (flags & Flags_NoResize)
		Window->Resizing = false;

	Window->Flags = flags;


	bool lazyaf = false;


	if (flags & Flags_PopUp) {

		Vec2 label_size;

		Window->Pos = Vec2(floor(Window->Pos.x), floor(Window->Pos.y));
		Window->Size = Vec2(floor(Window->Size.x), floor(Window->Size.y));

		if (Window->Opened) {
			DrawList::AddFilledRect(Window->Pos, Window->Size, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			DrawList::AddFilledRect(Window->Pos + Vec2(1, 1), Window->Size - Vec2(2, 2), D3DCOLOR_RGBA(35, 35, 35, g.MenuAlpha));
		}
		lazyaf = true;
	}

	if (!(flags & Flags_ChildWindow) && !lazyaf) {

		RECT ClipRect = { (LONG)Window->Pos.x, (LONG)Window->Pos.y, LONG(Window->Pos.x + Window->Size.x), LONG(Window->Pos.y + Window->Size.y ) };
		Render::Draw->GetD3dDevice()->SetScissorRect(&ClipRect);
		Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		Render::Draw->Sprite(Render::Draw->GetBgTexture(), Window->Pos, Vec2(4096, 4096), D3DCOLOR_RGBA(255, 255, 255, g.MenuAlpha));
		Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		int OutlineColors[6] = { 12, 61, 43, 43, 43, 61 };
		for (int i = 0; i < 6; i++)
			Render::Draw->Rect(Window->Pos + Vec2((float)i, (float)i), Window->Size - Vec2(2 * (float)i, 2 * (float)i), 0, D3DCOLOR_RGBA(OutlineColors[i], OutlineColors[i], OutlineColors[i], g.MenuAlpha));
		Render::Draw->FilledRect(Window->Pos + Vec2(6, 6), Vec2(Window->Size.x - 12, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->Gradient(Window->Pos + Vec2(7, 7), Vec2(Window->Size.x / 2 - 7, 2), D3DCOLOR_RGBA(55, 177, 218, g.MenuAlpha), D3DCOLOR_RGBA(204, 70, 205, g.MenuAlpha), false);
		Render::Draw->Gradient(Window->Pos + Vec2(7 + (Window->Size.x / 2 - 7), 7), Vec2(Window->Size.x / 2 - 7, 2), D3DCOLOR_RGBA(204, 70, 205, g.MenuAlpha), D3DCOLOR_RGBA(204, 227, 53, g.MenuAlpha), false);
		Render::Draw->FilledRect(Window->Pos + Vec2(7, 8), Vec2(Window->Size.x - 14, 1), D3DCOLOR_RGBA(0, 0, 0, std::clamp(119, 0, g.MenuAlpha)));
		Render::Draw->Line(Window->Pos + Vec2(7, 9), Window->Pos + Vec2(Window->Size.x - 7, 9), D3DCOLOR_RGBA(6, 6, 6, g.MenuAlpha));



		Render::Draw->FilledRect(Window->Pos + Vec2(6, 10), Vec2(74 + (20 * CMenu::get()->DPI()), 11), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha)); // 12 12 12
		Render::Draw->Line(Window->Pos + Vec2(80 + (20 * CMenu::get()->DPI()), 10), Window->Pos + Vec2(80 + (20 * CMenu::get()->DPI()), 20), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha)); //gray
		Render::Draw->Line(Window->Pos + Vec2(79 + (20 * CMenu::get()->DPI()), 10), Window->Pos + Vec2(79 + (20 * CMenu::get()->DPI()), 19), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));

		//ind

	}
	else if (flags & Flags_ChildWindow && !lazyaf) {
		Vec2 label_size;

		if (!Window->Name.empty())
			label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), Window->Name.c_str());

		Window->Pos = Vec2(floor(Window->Pos.x), floor(Window->Pos.y));
		Window->Size = Vec2(floor(Window->Size.x), floor(Window->Size.y));

		D3DCOLOR border_color = Window->Resizing == true ? CMenu::get()->MenuColor() : D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha);

		Render::Draw->FilledRect(Window->Pos + Vec2(2, 2), Window->Size - Vec2(3, 3), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha));

		Render::Draw->Line(Window->Pos, Window->Pos + Vec2(0, Window->Size.y), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->Line(Window->Pos + Vec2(0, Window->Size.y - 1), Window->Pos + Window->Size - Vec2(0, 1), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->Line(Window->Pos + Window->Size - Vec2(1, 1), Window->Pos + Vec2(Window->Size.x - 1, 0), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->Line(Window->Pos, Window->Pos + Vec2(10, 0), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->Line(Window->Pos + Vec2(16 + label_size.x, 0), Window->Pos + Vec2(Window->Size.x, 0), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

		Render::Draw->Line(Window->Pos + Vec2(1, 1), Window->Pos + Vec2(1, Window->Size.y - 1), border_color);
		Render::Draw->Line(Window->Pos + Vec2(1, Window->Size.y - 2), Window->Pos + Window->Size - Vec2(1, 2), border_color);
		Render::Draw->Line(Window->Pos + Window->Size - Vec2(2, 2), Window->Pos + Vec2(Window->Size.x - 2, 1), border_color);
		Render::Draw->Line(Window->Pos + Vec2(1, 1), Window->Pos + Vec2(10, 1), border_color);
		Render::Draw->Line(Window->Pos + Vec2(16 + label_size.x, 1), Window->Pos + Vec2(Window->Size.x - 1, 1), border_color);

		//

		if (!(flags & Flags_NoResize))
		{
			Render::Draw->Triangle(Window->Pos + Window->Size - Vec2(2, 2), Window->Pos + Window->Size - Vec2(2, 8), Window->Pos + Window->Size - Vec2(8, 2), border_color, true);
		}
	}

	Window->CursorPos = Window->Pos + Vec2(0, 23);

	int offset = (int)Window->PrevCursorPos.y - (int)((int)Window->Pos.y + (int)Window->Size.y);

	Window->CursorPos.y -= !(Window->PrevCursorPos.y > Window->Pos.y + Window->Size.y) ? 0 : (int)(offset * Window->ScrollRatio);

	SetCurrentWindow(Window);
}

void ui::End() {
	GuiContext& g = *Gui_Ctx;
	SetCurrentWindow(g.Windows.at(0));
	GuiWindow* Window = GetCurrentWindow();
	Vec2 MenuSize = Vec2(660, 560);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		MenuSize = Vec2(660, 560);
		break;
	}
	case 1:
	{
		MenuSize = Vec2(825, 750);
		break;
	}
	case 2:
	{
		MenuSize = Vec2(990, 880);
		break;
	}
	case 3:
	{
		MenuSize = Vec2(1155, 1020);
		break;
	}
	case 4:
	{
		MenuSize = Vec2(1320, 1030);
		break;
	}
	}

	static int last_dpi = CMenu::get()->DPI();
	int dpi = CMenu::get()->DPI();

	if (last_dpi != dpi)
	{
		Window->Size = MenuSize;
		last_dpi = dpi;
	}

	g.MouseWheel = 0.f;

	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	if (!Window->Block && !(Window->Flags & Flags_ChildWindow)) {
		HandleMoving(Window, Rect{ Vec2(0 - Window->Size.x / 2, 0 - Window->Size.y / 2), Vec2(1920, 1080) });
		HandleResize(Window, Rect{ MenuSize, Vec2(1920, 1080) });
	}
}


void ui::BeginChildRagewep(const char* Name, Rect X, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* parent_window = g.Windows.at(0);
	flags |= Flags_ChildWindow;


	if (!parent_window->ChildWindows.empty()) {

		int idx = 0;
		auto iterration = std::find(parent_window->ChildWindows.begin(), parent_window->ChildWindows.end(), FindWindowByName(Name));

		if (iterration != parent_window->ChildWindows.end())
			idx = std::distance(parent_window->ChildWindows.begin(), iterration);

		if (parent_window->ChildWindows.at(idx)) {
			Rect Boundaries = { (parent_window->Pos.x + 99) * CMenu::get()->GetDPINum(), parent_window->Pos.y + 29, parent_window->Size.x - (99 + 23) * CMenu::get()->GetDPINum(), parent_window->Size.y - (29 + 23) };

			Vec2 ChildPos = ui::GetWindowPos() + Vec2(100 * CMenu::get()->GetDPINum(), 30);
			Vec2 ChildSize;

			if (!parent_window->ChildWindows.at(idx)->Block) {
				HandleMoving(parent_window->ChildWindows.at(idx), Boundaries, &ChildPos);
				HandleResize(parent_window->ChildWindows.at(idx), Boundaries, &ChildSize);
			}

			if (!g.NextWindowInfo.PosCond)
				SetNextWindowPos(ChildPos);
			if (!g.NextWindowInfo.SizeCond)
				SetNextWindowSize(ChildSize);
		}
	}

	Begin(Name, flags);

	GuiWindow* child_window = GetCurrentWindow();
	child_window->ParentWindow = parent_window;


	RECT ClipRect = { (LONG)child_window->Pos.x, (LONG)child_window->Pos.y + 4, LONG(child_window->Pos.x + child_window->Size.x - 15), LONG(child_window->Pos.y + child_window->Size.y - 3) };
	Render::Draw->GetD3dDevice()->SetScissorRect(&ClipRect);
	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	if (CMenu::get()->ResetMenu) {
		remove("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		for (auto s : parent_window->ChildWindows)
			s->Init = false;
		CMenu::get()->ResetMenu = false;
	}

	if (!child_window->Init) {
		parent_window->ChildWindows.push_back(FindWindowByName(Name));

		child_window->xPos = (int)X.Min.x;
		child_window->yPos = (int)X.Min.y;

		child_window->xSize = (int)X.Max.x;
		child_window->ySize = (int)X.Max.y;
		char nigga[20];

		char nigga1[20];
		char nigga2[20];
		char nigga3[20];
		char nigga4[20];
		if (!GetPrivateProfileStringA(child_window->Name.c_str(), "posx", "", nigga, 20, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini")) {

			WritePrivateProfileStringA(child_window->Name.c_str(), "posx", std::to_string(child_window->xPos).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(child_window->Name.c_str(), "posy", std::to_string(child_window->yPos).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(child_window->Name.c_str(), "sizex", std::to_string(child_window->xSize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(child_window->Name.c_str(), "sizey", std::to_string(child_window->ySize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}
		else {
			child_window->xPos = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "posx", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			child_window->yPos = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "posy", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");

			child_window->xSize = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "sizex", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			child_window->ySize = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "sizey", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}
		child_window->Init = true;
	}


}
void ui::BeginChild(const char* Name, Rect X, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* parent_window = g.Windows.at(0);
	flags |= Flags_ChildWindow;


	if (!parent_window->ChildWindows.empty()) {

		int idx = 0;
		auto iterration = std::find(parent_window->ChildWindows.begin(), parent_window->ChildWindows.end(), FindWindowByName(Name));

		if (iterration != parent_window->ChildWindows.end())
			idx = std::distance(parent_window->ChildWindows.begin(), iterration);

		if (parent_window->ChildWindows.at(idx)) {
			Rect Boundaries = { (parent_window->Pos.x + 99) * CMenu::get()->GetDPINum(), parent_window->Pos.y + 29, parent_window->Size.x - (99 + 23) * CMenu::get()->GetDPINum(), parent_window->Size.y - (29 + 23) };

			Vec2 ChildPos = ui::GetWindowPos() + Vec2(100 * CMenu::get()->GetDPINum(), 30);
			Vec2 ChildSize;

			if (!parent_window->ChildWindows.at(idx)->Block) {
				HandleMoving(parent_window->ChildWindows.at(idx), Boundaries, &ChildPos);
				HandleResize(parent_window->ChildWindows.at(idx), Boundaries, &ChildSize);
			}

			SetNextWindowPos(ChildPos);
			SetNextWindowSize(ChildSize);
		}
	}

	Begin(Name, flags);

	GuiWindow* child_window = GetCurrentWindow();
	child_window->ParentWindow = parent_window;

	RECT ClipRect = { (LONG)child_window->Pos.x, (LONG)child_window->Pos.y + 4, LONG(child_window->Pos.x + child_window->Size.x - 15), LONG(child_window->Pos.y + child_window->Size.y - 3) };
	
	if (strcmp(Name, "Weapon type##rage") == 0)
	{
		ClipRect.right = LONG(child_window->Pos.x + child_window->Size.x);
	}

	Render::Draw->GetD3dDevice()->SetScissorRect(&ClipRect);
	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	if (CMenu::get()->ResetMenu) {
		remove("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		for (auto s : parent_window->ChildWindows)
			s->Init = false;
		CMenu::get()->ResetMenu = false;

	}

	if (!child_window->Init) {
		parent_window->ChildWindows.push_back(FindWindowByName(Name));

		child_window->xPos = (int)X.Min.x;
		child_window->yPos = (int)X.Min.y;

		child_window->xSize = (int)X.Max.x;
		child_window->ySize = (int)X.Max.y;
		char nigga[20];

		char nigga1[20];
		char nigga2[20];
		char nigga3[20];
		char nigga4[20];
		if (!GetPrivateProfileStringA(child_window->Name.c_str(), "posx", "", nigga, 20, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini")) {

			WritePrivateProfileStringA(child_window->Name.c_str(), "posx", std::to_string(child_window->xPos).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(child_window->Name.c_str(), "posy", std::to_string(child_window->yPos).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(child_window->Name.c_str(), "sizex", std::to_string(child_window->xSize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			WritePrivateProfileStringA(child_window->Name.c_str(), "sizey", std::to_string(child_window->ySize).c_str(), "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}
		else {


			child_window->xPos = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "posx", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			child_window->yPos = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "posy", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");

			child_window->xSize = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "sizex", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
			child_window->ySize = (int)GetPrivateProfileIntA(child_window->Name.c_str(), "sizey", 0, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\luas\\menu.ini");
		}
		child_window->Init = true;
	}


}

void ui::EndChild() {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();

	//window->CursorPos.y += 6;
	window->CursorPos.y += 4;

	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	if (window->PrevCursorPos.y > window->Pos.y + window->Size.y && (window->ScrollRatio >= 0.f || (!window->ParentWindow->Dragging || !ChildsAreStable(window->ParentWindow)))) {
		if (window->ScrollRatio == -0.2f )
			window->ScrollRatio = 0.f;

		int scroll_height = window->Size.y / ((window->PrevCursorPos.y - window->Pos.y) / (window->Size.y));
		int offset = (window->Size.y - scroll_height) * window->ScrollRatio;

		Render::Draw->FilledRect(Vec2(window->Pos.x + window->Size.x - 7, window->Pos.y + 1), Vec2(7, window->Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(window->Pos.x + window->Size.x - 6, window->Pos.y + 3 + offset), Vec2(6, scroll_height - 6), D3DCOLOR_RGBA(65, 65, 65, g.MenuAlpha));

		if (IsInside(window->Pos.x, window->Pos.y, window->Size.x, window->Size.y))
			window->ScrollRatio = std::clamp(window->ScrollRatio - g.MouseWheel * (20.f / ((int)window->PrevCursorPos.y - (int)((int)window->Pos.y + (int)window->Size.y))), 0.f, 1.f);

		if (window->ScrollRatio > 0.0f) {
			Render::Draw->Gradient(window->Pos + Vec2(2, 2), Vec2(window->Size.x - 9, 22), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha), D3DCOLOR_RGBA(23, 23, 23, 0), true);
			Render::Draw->Triangle(Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 15, 8), Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 18, 12), Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 12, 12), D3DCOLOR_RGBA(157, 157, 157, g.MenuAlpha));
		}
		if (window->ScrollRatio < 0.2f ) {
			Render::Draw->Gradient(window->Pos + Vec2(2, window->Size.y - 21), Vec2(window->Size.x - 9, 19), D3DCOLOR_RGBA(23, 23, 23, 0), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha), true);
			Render::Draw->Triangle(Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 17, -11), Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 15, -8), Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 12, -11), D3DCOLOR_RGBA(157, 157, 157, g.MenuAlpha));
		}
	}
	else {
		window->ScrollRatio = -0.2f;
	}

	int offset = (int)window->PrevCursorPos.y - (int)((int)window->Pos.y + (int)window->Size.y);
	window->CursorPos.y += !(window->PrevCursorPos.y > (int)window->Pos.y + (int)window->Size.y) ? 0 : (int)(offset * window->ScrollRatio);
	window->PrevCursorPos = window->CursorPos;

	Vec2 text_size;
	std::string ChildName = window->Name;

	if (window->Name.find("#") != std::string::npos)
		ChildName = ChildName.substr(0, window->Name.find("#"));

	if (!ChildName.empty())
		text_size = Render::Draw->GetTextSize(Render::Fonts::Tahombd, ChildName.c_str());

	Render::Draw->Text(ChildName.c_str(), window->Pos.x + 14, window->Pos.y + (text_size.y / -2) + 1, LEFT, CMenu::get()->GetFontBold(), false, D3DCOLOR_RGBA(15, 15, 15, g.MenuAlpha));
	Render::Draw->Text(ChildName.c_str(), window->Pos.x + 13, window->Pos.y + (text_size.y / -2), LEFT, CMenu::get()->GetFontBold(), false, window->Dragging ? CMenu::get()->MenuColor() : D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha));

	SetCurrentWindow(g.Windows.at(0));
}
// this is listbox begin

bool ui::listboxskins(const char* label, bool activated, const Vec2& size_arg)
{
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Rect Framebb{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x : size_arg.x, size_arg.y == 0 ? 16 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	Rect Framebb2{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 17 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	bool hovered = IsInside(Framebb.Min.x, Framebb.Min.y, Framebb.Max.x, Framebb.Max.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;

	if (pressed)
		Window->SelectedItem = label;

	LPD3DXFONT TextFont = (activated) ? CMenu::get()->GetFontBold() : CMenu::get()->GetFontDPI();
	Vec2 label_size = Render::Draw->GetTextSize(TextFont, label);
	D3DCOLOR TextColor;
	if (activated) {
		TextColor = CMenu::get()->MenuColor();
	}
	else if (activated) {
		TextColor = CMenu::get()->MenuColor();
	}
	else {
		TextColor = D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha);
	}

	AddItemToWindow(Window, Framebb2, Window->Flags);

	if (activated) {
		Render::Draw->FilledRect(Framebb.Min, Framebb.Max - Vec2(2, 0), D3DCOLOR_RGBA(25, 25, 25, g.MenuAlpha));
	}

	Render::Draw->Text(label, Framebb.Min.x + 9, Framebb.Min.y + (Framebb.Max.y / 2) - label_size.y / 2, LEFT, TextFont, false, TextColor, Framebb.Min + Framebb.Max - Vec2(15, 0));

	return pressed;
}

void ui::ListBox(const char* Name, Vec2 X, bool Text, GuiFlags flags, D3DCOLOR col) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	flags |= Flags_PopUp | Flags_ColorPicker;



	Rect Fullbb = { Window->CursorPos.x + 42 + (1 * CMenu::get()->DPI()), Window->CursorPos.y - 3, X.x, X.y + (25 * CMenu::get()->DPI()) };
	Rect Fullbb2 = { Window->CursorPos.x + 42 + (1 * CMenu::get()->DPI()), Window->CursorPos.y - 3, X.x, X.y + 2 + (25 * CMenu::get()->DPI()) };


	AddItemToWindow(Window, Fullbb2, NULL);

	SetNextWindowPos(Fullbb.Min);
	SetNextWindowSize(Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), X.y + (25 * CMenu::get()->DPI())));

	Begin(Name, Flags_PopUp);
	GuiWindow* PickerWindow = GetCurrentWindow();

	PickerWindow->ParentWindow = Window;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	PickerWindow->CursorPos.x += 1;
	PickerWindow->CursorPos.y -= 22;

	if (!Text) {
		Render::Draw->FilledRect(PickerWindow->Pos, PickerWindow->Size, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(PickerWindow->Pos + Vec2(1, 1), PickerWindow->Size - Vec2(2, 2), col);
	}
	else {
		Render::Draw->FilledRect(PickerWindow->Pos + Vec2(0, 1), PickerWindow->Size - Vec2(0, 1), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(PickerWindow->Pos + Vec2(1, 1), PickerWindow->Size - Vec2(2, 2), col);
	}


	RECT ClipRect2 = { (LONG)Window->Pos.x, (LONG)Window->Pos.y + 4, LONG(Window->Pos.x + Window->Size.x - 15), LONG(Window->Pos.y + Window->Size.y - 3) };
	Render::Draw->GetD3dDevice()->SetScissorRect(&ClipRect2);
	//SetCurrentWindow(PickerWindow->ParentWindow);
}

// end
void ui::EndListBox() {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();

	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	// scrolling
	window->CursorPos.y += 10;

	if (window->PrevCursorPos.y > window->Pos.y + window->Size.y && (window->ScrollRatio >= 0.f || (!window->ParentWindow->Dragging || !ChildsAreStable(window->ParentWindow)))) {
		if (window->ScrollRatio == -0.2f)
			window->ScrollRatio = 0.f;

		int scroll_height = window->Size.y / ((window->PrevCursorPos.y - window->Pos.y) / (window->Size.y));
		int offset = (window->Size.y - scroll_height) * window->ScrollRatio;

		Render::Draw->FilledRect(Vec2(window->Pos.x + window->Size.x - 7, window->Pos.y + 1), Vec2(7, window->Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(window->Pos.x + window->Size.x - 6, window->Pos.y + 3 + offset), Vec2(6, scroll_height - 6), D3DCOLOR_RGBA(65, 65, 65, g.MenuAlpha));

		if (IsInside(window->Pos.x, window->Pos.y, window->Size.x, window->Size.y))
			window->ScrollRatio = std::clamp(window->ScrollRatio - g.MouseWheel * (20.f / ((int)window->PrevCursorPos.y - (int)((int)window->Pos.y + (int)window->Size.y))), 0.f, 1.f);

		if (window->ScrollRatio > 0.0f) {
			Render::Draw->Gradient(window->Pos + Vec2(2, 2), Vec2(window->Size.x - 9, 22), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha), D3DCOLOR_RGBA(23, 23, 23, 0), true);
			Render::Draw->Triangle(Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 15, 8), Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 18, 12), Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 12, 12), D3DCOLOR_RGBA(157, 157, 157, g.MenuAlpha));
		}
		if (window->ScrollRatio < 0.2f) {
			Render::Draw->Gradient(window->Pos + Vec2(2, window->Size.y - 21), Vec2(window->Size.x - 9, 19), D3DCOLOR_RGBA(23, 23, 23, 0), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha), true);
			Render::Draw->Triangle(Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 17, -11), Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 15, -8), Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 12, -11), D3DCOLOR_RGBA(157, 157, 157, g.MenuAlpha));
		}
	}
	else {
		window->ScrollRatio = -0.2f;
	}

	int offset = (int)window->PrevCursorPos.y - (int)((int)window->Pos.y + (int)window->Size.y);
	window->CursorPos.y += !(window->PrevCursorPos.y > (int)window->Pos.y + (int)window->Size.y) ? 0 : (int)(offset * window->ScrollRatio);
	window->PrevCursorPos = window->CursorPos;

	RECT ClipRect2 = { (LONG)window->ParentWindow->Pos.x, (LONG)window->ParentWindow->Pos.y + 4, LONG(window->ParentWindow->Pos.x + window->ParentWindow->Size.x - 15), LONG(window->ParentWindow->Pos.y + window->ParentWindow->Size.y - 3) };
	Render::Draw->GetD3dDevice()->SetScissorRect(&ClipRect2);
	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	SetCurrentWindow(window);
	SetCurrentWindow(window->ParentWindow);
}

void ui::EndSelectNoText() {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();

	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	// scrolling
	window->CursorPos.y += 9;

	if (window->PrevCursorPos.y > window->Pos.y + window->Size.y && (window->ScrollRatio >= 0.f || (!window->ParentWindow->Dragging || !ChildsAreStable(window->ParentWindow)))) {
		if (window->ScrollRatio == -0.2f)
			window->ScrollRatio = 0.f;

		int scroll_height = window->Size.y / ((window->PrevCursorPos.y - window->Pos.y) / (window->Size.y));
		int offset = (window->Size.y - scroll_height) * window->ScrollRatio;

		Render::Draw->FilledRect(Vec2(window->Pos.x + window->Size.x - 7, window->Pos.y + 1), Vec2(7, window->Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(window->Pos.x + window->Size.x - 6, window->Pos.y + 3 + offset), Vec2(6, scroll_height - 6), D3DCOLOR_RGBA(65, 65, 65, g.MenuAlpha));

		if (IsInside(window->Pos.x, window->Pos.y, window->Size.x, window->Size.y))
			window->ScrollRatio = std::clamp(window->ScrollRatio - g.MouseWheel * (20.f / ((int)window->PrevCursorPos.y - (int)((int)window->Pos.y + (int)window->Size.y))), 0.f, 1.f);

		if (window->ScrollRatio > 0.0f) {
			Render::Draw->Gradient(window->Pos + Vec2(2, 2), Vec2(window->Size.x - 9, 22), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha), D3DCOLOR_RGBA(23, 23, 23, 0), true);
			Render::Draw->Triangle(Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 15, 8), Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 18, 12), Vec2((int)window->Pos.x, (int)window->Pos.y) + Vec2((int)window->Size.x - 12, 12), D3DCOLOR_RGBA(157, 157, 157, g.MenuAlpha));
		}
		if (window->ScrollRatio < 0.2f) {
			Render::Draw->Gradient(window->Pos + Vec2(2, window->Size.y - 21), Vec2(window->Size.x - 9, 19), D3DCOLOR_RGBA(23, 23, 23, 0), D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha), true);
			Render::Draw->Triangle(Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 17, -11), Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 15, -8), Vec2((int)window->Pos.x, (int)window->Pos.y + (int)window->Size.y) + Vec2((int)window->Size.x - 12, -11), D3DCOLOR_RGBA(157, 157, 157, g.MenuAlpha));
		}
	}
	else {
		window->ScrollRatio = -0.2f;
	}

	int offset = (int)window->PrevCursorPos.y - (int)((int)window->Pos.y + (int)window->Size.y);
	window->CursorPos.y += !(window->PrevCursorPos.y > (int)window->Pos.y + (int)window->Size.y) ? 0 : (int)(offset * window->ScrollRatio);
	window->PrevCursorPos = window->CursorPos;

	RECT ClipRect2 = { (LONG)window->ParentWindow->Pos.x, (LONG)window->ParentWindow->Pos.y + 4, LONG(window->ParentWindow->Pos.x + window->ParentWindow->Size.x - 15), LONG(window->ParentWindow->Pos.y + window->ParentWindow->Size.y - 3) };
	Render::Draw->GetD3dDevice()->SetScissorRect(&ClipRect2);
	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	SetCurrentWindow(window);
	SetCurrentWindow(window->ParentWindow);
}

void ui::InputText(const char* id, char* buffer, bool Listbox, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	int RemoveSpacing = 0;

	if (Listbox)
		RemoveSpacing = 8;

	Rect Framebb = { Window->CursorPos + Vec2(42 + (1 * CMenu::get()->DPI()), 0), Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 20 + (5 * CMenu::get()->DPI())) };
	Rect Framebb2 = { Window->CursorPos + Vec2(42 + (1 * CMenu::get()->DPI()), 0), Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 29 - RemoveSpacing + (5 * CMenu::get()->DPI())) };

	AddItemToWindow(Window, Framebb2, flags);

	bool hovered = IsInside(Framebb.Min.x, Framebb.Min.y, Framebb.Max.x, Framebb.Max.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;

	if (pressed) {
		Window->SelectedId[id] = std::string(id);
		g.AwaitingInput = true;
	}
	else if (!hovered && Window->SelectedId[id] != "" && KeyPressed(VK_LBUTTON)) {
		Window->SelectedId[id] = "";
		g.AwaitingInput = false;
		g.LastInput = 0;
	}

	bool is_selected = Window->SelectedId[id] == id;

	if (is_selected) {
		for (int i = 0x20; i <= 0x5A; i++) {
			if ((i == VK_SPACE || i >= 0x30) && KeyPressed(i)) {

				char c[2] = { char(MapVirtualKeyA(i, 2)), 0 };

				if (i > 0x3A)
					c[0] += (KeyDown(VK_SHIFT) ? 0 : 32);

				if (strlen(buffer) < 63)
					strcat_s(buffer, 64, (char*)c);

				if (g.LastInput == 0)
					g.LastInput = time(0);

				break;
			}
		}

		if (KeyPressed(VK_BACK) && strlen(buffer) > 0)
			buffer[strlen(buffer) - 1] = '\0';
	}

	const int colors[2] = { 50, 16 };

	D3DCOLOR text_col = is_selected ? CMenu::get()->MenuColor() : D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha);

	Render::Draw->Rect(Framebb.Min + Vec2(0, 0), Framebb.Max - Vec2(0 * 2, 0 * 2), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Rect(Framebb.Min + Vec2(1, 1), Framebb.Max - Vec2(1 * 2, 1 * 2), 1, D3DCOLOR_RGBA(50, 50, 50, g.MenuAlpha));
	Render::Draw->Rect(Framebb.Min + Vec2(2, 2), Framebb.Max - Vec2(2 * 2, 2 * 2), 1, D3DCOLOR_RGBA(16, 16, 16, g.MenuAlpha));

	std::string out_str = std::string(buffer) + '_';

	Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), out_str.c_str());
	Render::Draw->Text(out_str.c_str(), Framebb.Min.x + 5, Framebb.Min.y + ((Framebb.Max.y / 2) - label_size.y / 2), LEFT, CMenu::get()->GetFontDPI(), false, text_col, Framebb.Min + Framebb.Max - Vec2(4, 0));
}

bool ui::TabButton(const char* label, int* selected, int num, int total) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();


	Vec2 DrawPos = Vec2(window->Pos.x + 6, window->Pos.y + 19) + Vec2(0, float((461 + total) / total * num));
	const Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::TabIcons, label);

	Vec2 Size = Vec2(74, 70);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Size = Vec2(74, 70);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		Size = Vec2(74 + 20, 70 + 14);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		Size = Vec2(74 + (20 * 2), 70 + (14 * 2));
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		Size = Vec2(74 + (20 * 3), 70 + (14 * 3));
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		Size = Vec2(74 + (20 * 4), 70 + (14 * 4));
		Offset = Vec2(4, 4);
		break;
	}
	}

	if (num != 0)
		DrawPos = DrawPos + Vec2(0, (14 * CMenu::get()->DPI()) * num);


	if (num == 7 && ui::GetWindowSize().y < 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 2), Vec2(1, window->Size.y - 555), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 2), Vec2(1, window->Size.y - 555), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}
	if (*selected != num && num == 8 && ui::GetWindowSize().y >= 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555 - 66 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 3), Vec2(1, window->Size.y - 65 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 3), Vec2(1, window->Size.y - 65 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}
	else if (*selected == num && num == 8 && ui::GetWindowSize().y >= 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555 - 66 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 2), Vec2(1, window->Size.y - 66 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 2), Vec2(1, window->Size.y - 66 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}



	if (num == 8 && ui::GetWindowSize().y < 623.f + (57 * CMenu::get()->DPI()))
		goto jmp;

	if (*selected != num) {
		if (*selected == num - 1) {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 4), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y + 2), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y + 3), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
		else if (num == 7) {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 2), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
		else {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 2), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 3), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 3), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
	}

	else {
		Render::Draw->Line(DrawPos, DrawPos + Vec2(Size.x, 0), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->Line(DrawPos + Vec2(0, 1), DrawPos + Vec2(Size.x + 1, 1), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));

		Render::Draw->Line(DrawPos + Vec2(0, Size.y - 2), DrawPos + Vec2(Size.x + 1, Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
		Render::Draw->Line(DrawPos + Vec2(0, Size.y - 1), DrawPos + Vec2(Size.x, Size.y - 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
	}


jmp:
	bool hovered = IsInside(DrawPos.x, DrawPos.y, Size.x, Size.y);

	if (num == 8 && c_lua::get().luatabactive) {
		if (KeyPressed(VK_LBUTTON) && hovered)
			*selected = num;
	}
	else {
		if (KeyPressed(VK_LBUTTON) && hovered)
			*selected = num;
	}

	D3DCOLOR textColor = D3DCOLOR_RGBA(90, 90, 90, g.MenuAlpha);
	if (hovered)
		textColor = D3DCOLOR_RGBA(140, 140, 140, g.MenuAlpha);
	if (*selected == num)
		textColor = D3DCOLOR_RGBA(210, 210, 210, g.MenuAlpha);


	if (num == 6)
		Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons3, false, textColor);
	else if (num == 3)
		Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons4, false, textColor);
	else
		Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons, false, textColor);

	return true;
}

bool ui::TabButtonLua125(const char* label, int* selected, int num, int total) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();


	Vec2 DrawPos = Vec2(window->Pos.x + 6, window->Pos.y + 19) + Vec2(0, float((461 + total) / total * num));
	const Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::TabIcons, label);

	Vec2 Size = Vec2(74, 70);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Size = Vec2(74, 70); //Size = Vec2(74, 70);
		Offset = Vec2(0, 0); //Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		Size = Vec2(74 + 20, 70 + 14);//Size = Vec2(74 + 20, 70 + 14);
		Offset = Vec2(1, 1);//Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		Size = Vec2(74 + (20 * 2), 70 + (14 * 2));
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		Size = Vec2(74 + (20 * 3), 70 + (14 * 3));
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		Size = Vec2(74 + (20 * 4), 70 + (14 * 4));
		Offset = Vec2(4, 4);
		break;
	}
	}

	if (num != 0)
		DrawPos = DrawPos + Vec2(0, (14 * CMenu::get()->DPI()) * num);


	if (num == 7 && ui::GetWindowSize().y < 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 2), Vec2(1, window->Size.y - 555), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 2), Vec2(1, window->Size.y - 555), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}
	if (*selected != num && num == 8 && ui::GetWindowSize().y >= 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555 - 66 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 3), Vec2(1, window->Size.y - 65 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 3), Vec2(1, window->Size.y - 65 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}
	else if (*selected == num && num == 8 && ui::GetWindowSize().y >= 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555 - 66 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 2), Vec2(1, window->Size.y - 66 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 2), Vec2(1, window->Size.y - 66 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}



	if (num == 8 && ui::GetWindowSize().y < 623.f + (57 * CMenu::get()->DPI()))
		goto jmp;

	if (*selected != num) {
		if (*selected == num - 1) {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 4), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y + 2), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y + 3), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
		else if (num == 7) {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 2), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
		else {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 2), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 3), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 3), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
	}

	else {
		Render::Draw->Line(DrawPos, DrawPos + Vec2(Size.x, 0), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->Line(DrawPos + Vec2(0, 1), DrawPos + Vec2(Size.x + 1, 1), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));

		Render::Draw->Line(DrawPos + Vec2(0, Size.y - 2), DrawPos + Vec2(Size.x + 1, Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
		Render::Draw->Line(DrawPos + Vec2(0, Size.y - 1), DrawPos + Vec2(Size.x, Size.y - 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
	}


jmp:
	bool hovered = IsInside(DrawPos.x, DrawPos.y, Size.x, Size.y);

	if (num == 8 && c_lua::get().luatabactive) {
		if (KeyPressed(VK_LBUTTON) && hovered)
			*selected = num;
	}
	else {
		if (KeyPressed(VK_LBUTTON) && hovered)
			*selected = num;
	}

	D3DCOLOR textColor = D3DCOLOR_RGBA(90, 90, 90, g.MenuAlpha);
	if (hovered)
		textColor = D3DCOLOR_RGBA(140, 140, 140, g.MenuAlpha);
	if (*selected == num)
		textColor = D3DCOLOR_RGBA(210, 210, 210, g.MenuAlpha);

	if (num == 8 && GetWindowSize().y < 596.f + (57 * CMenu::get()->DPI()) || num == 8 && !c_lua::get().luatabactive)
		return false;

	if (num == 6) //Size.x / 2    //2.9

		Render::Draw->Text(label, DrawPos.x + Size.x / 2.8 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabLua125, false, textColor);
	else
		Render::Draw->Text(label, DrawPos.x + Size.x / 2.8 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabLua125, false, textColor);
	//	Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons3, false, textColor);
	//else if (num == 3)
		//Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons4, false, textColor);
//	else
	//	Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons, false, textColor);

	return true;
}


bool ui::TabButtonLua150(const char* label, int* selected, int num, int total) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();


	Vec2 DrawPos = Vec2(window->Pos.x + 6, window->Pos.y + 19) + Vec2(0, float((461 + total) / total * num));
	const Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::TabIcons, label);

	Vec2 Size = Vec2(74, 70);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Size = Vec2(74, 70); //Size = Vec2(74, 70);
		Offset = Vec2(0, 0); //Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		Size = Vec2(74 + 20, 70 + 14);//Size = Vec2(74 + 20, 70 + 14);
		Offset = Vec2(1, 1);//Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		Size = Vec2(74 + (20 * 2), 70 + (14 * 2));
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		Size = Vec2(74 + (20 * 3), 70 + (14 * 3));
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		Size = Vec2(74 + (20 * 4), 70 + (14 * 4));
		Offset = Vec2(4, 4);
		break;
	}
	}

	if (num != 0)
		DrawPos = DrawPos + Vec2(0, (14 * CMenu::get()->DPI()) * num);


	if (num == 7 && ui::GetWindowSize().y < 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 2), Vec2(1, window->Size.y - 555), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 2), Vec2(1, window->Size.y - 555), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}
	if (*selected != num && num == 8 && ui::GetWindowSize().y >= 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555 - 66 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 3), Vec2(1, window->Size.y - 65 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 3), Vec2(1, window->Size.y - 65 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}
	else if (*selected == num && num == 8 && ui::GetWindowSize().y >= 623.f + (57 * CMenu::get()->DPI())) {
		Render::Draw->FilledRect(DrawPos + Vec2(0, Size.y - 2), Vec2(Size.x, window->Size.y - 555 - 66 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x - 1, Size.y - 2), Vec2(1, window->Size.y - 66 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(DrawPos + Vec2(Size.x, Size.y - 2), Vec2(1, window->Size.y - 66 - 555 - (118 * CMenu::get()->DPI()) - CMenu::get()->GetDPITab()), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
	}



	if (num == 8 && ui::GetWindowSize().y < 623.f + (57 * CMenu::get()->DPI()))
		goto jmp;

	if (*selected != num) {
		if (*selected == num - 1) {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 4), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y + 2), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y + 3), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
		else if (num == 7) {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 2), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 2), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
		else {
			Render::Draw->FilledRect(DrawPos + Vec2(0, 2), Size - Vec2(1, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x, DrawPos.y), Vec2(DrawPos.x + Size.x, DrawPos.y + Size.y - 3), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
			Render::Draw->Line(Vec2(DrawPos.x + Size.x - 1, DrawPos.y), Vec2(DrawPos.x + Size.x - 1, DrawPos.y + Size.y - 3), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		}
	}

	else {
		Render::Draw->Line(DrawPos, DrawPos + Vec2(Size.x, 0), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->Line(DrawPos + Vec2(0, 1), DrawPos + Vec2(Size.x + 1, 1), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));

		Render::Draw->Line(DrawPos + Vec2(0, Size.y - 2), DrawPos + Vec2(Size.x + 1, Size.y - 2), D3DCOLOR_RGBA(40, 40, 40, g.MenuAlpha));
		Render::Draw->Line(DrawPos + Vec2(0, Size.y - 1), DrawPos + Vec2(Size.x, Size.y - 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
	}


jmp:
	bool hovered = IsInside(DrawPos.x, DrawPos.y, Size.x, Size.y);

	if (num == 8 && c_lua::get().luatabactive) {
		if (KeyPressed(VK_LBUTTON) && hovered)
			*selected = num;
	}
	else {
		if (KeyPressed(VK_LBUTTON) && hovered)
			*selected = num;
	}

	D3DCOLOR textColor = D3DCOLOR_RGBA(90, 90, 90, g.MenuAlpha);
	if (hovered)
		textColor = D3DCOLOR_RGBA(140, 140, 140, g.MenuAlpha);
	if (*selected == num)
		textColor = D3DCOLOR_RGBA(210, 210, 210, g.MenuAlpha);

	if (num == 8 && GetWindowSize().y < 630.f + (57 * CMenu::get()->DPI()) || num == 8 && !c_lua::get().luatabactive)
		return false;

	if (num == 6) //Size.x / 2                       //2.9
		Render::Draw->Text(label, DrawPos.x + Size.x / 2.7 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabLua150, false, textColor);
	else
		Render::Draw->Text(label, DrawPos.x + Size.x / 2.7 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabLua150, false, textColor);
	//	Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons3, false, textColor);
	//else if (num == 3)
		//Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons4, false, textColor);
//	else
	//	Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons, false, textColor);

	return true;
}



bool ui::LegitButton(const char* label, int* selected, int num, int total, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();

	Vec2 DrawPos = Vec2(window->Pos.x + 24, window->Pos.y + 19) + Vec2(float((461 + total) / total * num), 0);
	const Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::LBIcons, label);
	Vec2 Size = Vec2(55 + 20, 40);
	Vec2 Offset = Vec2(1, 1);

	const int sizes[] = { 69, 60, 82, 57, 70, 88 };
	const Vec2 positions[] = { Vec2(15, 17), Vec2(97, 12),  Vec2(170, 19), Vec2(265, 17),  Vec2(335, 15), Vec2(419, 16) };

	if (flags & GuiFlags_LegitTab) {
		Size = Vec2(sizes[num], 68);
		DrawPos = window->Pos + positions[num];
	}

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Size = Vec2(55 + 20, 40);
		Offset = Vec2(1, 1); //Vec2(0, 0);
		break;
	}
	case 1:
	{
		Size = Vec2(55 + 20, 40 + 14);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		Size = Vec2(60 + (20 * 2), 40 + (14 * 2));
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		Size = Vec2(60 + (20 * 3), 40 + (14 * 3));
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		Size = Vec2(40 + (20 * 4), 40 + (14 * 4));
		Offset = Vec2(4, 4);
		break;
	}
	}

	if (num != 0)
		DrawPos = DrawPos + Vec2((18 * CMenu::get()->DPI()) * num, 0);
jmp:
	bool hovered = IsInside(DrawPos.x, DrawPos.y, Size.x, Size.y);

	if (KeyPressed(VK_LBUTTON) && hovered)
		*selected = num;

	D3DCOLOR textColor = D3DCOLOR_RGBA(90, 90, 90, g.MenuAlpha);
	if (hovered)
		textColor = D3DCOLOR_RGBA(140, 140, 140, g.MenuAlpha);
	if (*selected == num)
		textColor = D3DCOLOR_RGBA(210, 210, 210, g.MenuAlpha);

	//Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 5, DrawPos.y + Size.y / 2 - label_size.y / 2 - 4, LEFT, Render::Fonts::LBIcons, false, textColor);

	if (flags & GuiFlags_LegitTab)
		Render::Draw->Text(label, DrawPos.x, DrawPos.y, LEFT, Render::Fonts::LBIcons, false, textColor);
	else
		Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons, false, textColor);

	return true;
}

void ui::AddItemToWindow(GuiWindow* Window, Rect size, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	Window->PrevWidgetPos = Window->CursorPos;
	if (!(flags & Flags_ColorPicker))
		Window->CursorPos = Window->CursorPos + Vec2(0, size.Max.y + ((flags & Flags_ComboBox) || (flags & Flags_PopUp) || (flags & Flags_Listbox)));
}

//bool ui::ButtonBehavior()

bool ui::ListBoxSelectable(const char* label, bool activated, bool loaded, const Vec2& size_arg) {

	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Rect Framebb{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 16 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	Rect Framebb2{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 17 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	bool hovered = IsInside(Framebb.Min.x, Framebb.Min.y, Framebb.Max.x, Framebb.Max.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;
	if (!Window->Disabled) {
		if (pressed) {
			Window->SelectedItem = label;
			activated = true;
		}
	}
	LPD3DXFONT TextFont = (activated) ? CMenu::get()->GetFontBold() : CMenu::get()->GetFontDPI();
	Vec2 label_size = Render::Draw->GetTextSize(TextFont, label);
	D3DCOLOR TextColor;
	if (loaded) {
		TextColor = CMenu::get()->MenuColor();
	}
	else if (Window->Disabled) {
		TextColor = D3DCOLOR_RGBA(60, 60, 60, g.MenuAlpha);
	}
	else {
		TextColor = D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha);
	}

	AddItemToWindow(Window, Framebb2, Window->Flags);

	if (activated) {
		Render::Draw->FilledRect(Framebb.Min, Framebb.Max, D3DCOLOR_RGBA(25, 25, 25, g.MenuAlpha));
	}

	Render::Draw->Text(label, Framebb.Min.x + 9, Framebb.Min.y + (Framebb.Max.y / 2) - label_size.y / 2, LEFT, TextFont, false, TextColor, Framebb.Min + Framebb.Max - Vec2(15, 0));

	return pressed;

}


bool ui::RageButton(const char* label, int* selected, int num, int total, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* window = GetCurrentWindow();

	Vec2 DrawPos = Vec2(window->Pos.x + -4, window->Pos.y + 9) + Vec2(float((461 + total) / total * num), 0);
	const Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::LBIcons, label);
	Vec2 Size = Vec2(55 + 20, 40);
	Vec2 Offset = Vec2(-5, 1);

	const int sizes[] = { 69, 60, 82, 57, 70, 88 };
	//const Vec2 positions[] = { Vec2(15, 17), Vec2(97, 12),  Vec2(170, 19), Vec2(265, 17),  Vec2(335, 15), Vec2(419, 16) };
	auto nig = Vec2(245, 32); // 125

	if (flags & GuiFlags_LegitTab) {
		Size = Vec2(sizes[num], 68);
		DrawPos = window->Pos + nig;
	}

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Size = Vec2(55 + 20, 40);
		Offset = Vec2(1, 1); //Vec2(0, 0);
		break;
	}
	case 1:
	{
		Size = Vec2(55 + 20, 40 + 14);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		Size = Vec2(60 + (20 * 2), 40 + (14 * 2));
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		Size = Vec2(60 + (20 * 3), 40 + (14 * 3));
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		Size = Vec2(40 + (20 * 4), 40 + (14 * 4));
		Offset = Vec2(4, 4);
		break;
	}
	}

	if (num != 0)
		DrawPos = DrawPos + Vec2((-15 * CMenu::get()->DPI()) * num, 0); //DrawPos = DrawPos + Vec2((18 * CMenu::get()->DPI()) * num, 0);
jmp:
	bool hovered = IsInside(DrawPos.x, DrawPos.y, Size.x, Size.y);

	if (KeyPressed(VK_LBUTTON) && hovered)
		*selected = num;

	D3DCOLOR textColor = D3DCOLOR_RGBA(90, 90, 90, g.MenuAlpha);
	if (hovered)
		textColor = D3DCOLOR_RGBA(140, 140, 140, g.MenuAlpha);
	if (*selected == num)
		textColor = D3DCOLOR_RGBA(210, 210, 210, g.MenuAlpha);

	//Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2 + 5, DrawPos.y + Size.y / 2 - label_size.y / 2 - 4, LEFT, Render::Fonts::LBIcons, false, textColor);

	if (flags & GuiFlags_LegitTab)
		Render::Draw->Text(label, DrawPos.x, DrawPos.y, LEFT, Render::Fonts::LBIcons2, false, textColor);
	//Render::Draw->Text(label, DrawPos.x, DrawPos.y, LEFT, Render::Fonts::LBIcons, false, textColor);
	else
		Render::Draw->Text(label, DrawPos.x + Size.x / 2 - label_size.x / 2, DrawPos.y + Size.y / 2 - label_size.y / 2 - 1, LEFT, Render::Fonts::TabIcons, false, textColor);

	return true;
}

bool ui::ListBoxSelectableLua(const char* label, bool selected, bool loaded, const Vec2& size_arg) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Rect Framebb{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 16 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	Rect Framebb2{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 17 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	bool hovered = IsInside(Framebb.Min.x, Framebb.Min.y, Framebb.Max.x, Framebb.Max.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;
	if (!Window->Disabled) {
		if (pressed) {
			Window->SelectedItem = label;
			selected = true;
		}
	}
	LPD3DXFONT TextFont = (selected) ? CMenu::get()->GetFontBold() : CMenu::get()->GetFontDPI();
	Vec2 label_size = Render::Draw->GetTextSize(TextFont, label);
	D3DCOLOR TextColor;
	if (loaded) {
		TextColor = CMenu::get()->MenuColor();
	}
	else if (Window->Disabled) {
		TextColor = D3DCOLOR_RGBA(60, 60, 60, g.MenuAlpha);
	}
	else {
		TextColor = D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha);
	}

	AddItemToWindow(Window, Framebb2, Window->Flags);

	if (selected) {
		Render::Draw->FilledRect(Framebb.Min, Framebb.Max, D3DCOLOR_RGBA(25, 25, 25, g.MenuAlpha));
	}

	Render::Draw->Text(label, Framebb.Min.x + 9, Framebb.Min.y + (Framebb.Max.y / 2) - label_size.y / 2, LEFT, TextFont, false, TextColor, Vec2(Window->Size.x, Window->Size.y), Window->Pos);

	return pressed;
}

bool ui::ListBoxSelectableElement(const char* label, bool selected, bool loaded, const Vec2& size_arg) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Rect Framebb{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 16 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	Rect Framebb2{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 17 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	bool hovered = IsInside(Framebb.Min.x, Framebb.Min.y, Framebb.Max.x, Framebb.Max.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;
	if (pressed) {
		Window->SelectedItem = label;
		selected = true;
	}
	LPD3DXFONT TextFont = (selected) ? CMenu::get()->GetFontBold() : CMenu::get()->GetFontDPI();
	Vec2 label_size = Render::Draw->GetTextSize(TextFont, label);
	D3DCOLOR TextColor;
	if (loaded) {
		TextColor = CMenu::get()->MenuColor();
	}
	else {
		TextColor = D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha);
	}

	AddItemToWindow(Window, Framebb2, Window->Flags);

	if (selected) {
		Render::Draw->FilledRect(Framebb.Min, Framebb.Max, D3DCOLOR_RGBA(25, 25, 25, g.MenuAlpha));
	}

	Render::Draw->Text(label, Framebb.Min.x + 9, Framebb.Min.y + (Framebb.Max.y / 2) - label_size.y / 2, LEFT, TextFont, false, TextColor, Framebb.Min + Framebb.Max - Vec2(15, 0));

	return pressed;
}

bool ui::ListBoxSelectableConfig(const char* label, bool activated, const Vec2& size_arg) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Rect Framebb{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x : size_arg.x, size_arg.y == 0 ? 16 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	Rect Framebb2{ Window->CursorPos - Vec2(0, 0), Vec2(size_arg.x == 0 ? Window->Size.x - 2 : size_arg.x, size_arg.y == 0 ? 17 + (5 * CMenu::get()->DPI()) : size_arg.y) };
	bool hovered = IsInside(Framebb.Min.x, Framebb.Min.y, Framebb.Max.x, Framebb.Max.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;

	if (pressed)
		Window->SelectedItem = label;

	LPD3DXFONT TextFont = (activated) ? CMenu::get()->GetFontBold() : CMenu::get()->GetFontDPI();
	Vec2 label_size = Render::Draw->GetTextSize(TextFont, label);
	D3DCOLOR TextColor;
	if (activated) {
		TextColor = CMenu::get()->MenuColor();
	}
	else if (activated) {
		TextColor = CMenu::get()->MenuColor();
	}
	else {
		TextColor = D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha);
	}

	AddItemToWindow(Window, Framebb2, Window->Flags);

	if (activated) {
		Render::Draw->FilledRect(Framebb.Min, Framebb.Max - Vec2(2, 0), D3DCOLOR_RGBA(25, 25, 25, g.MenuAlpha));
	}

	Render::Draw->Text(label, Framebb.Min.x + 9, Framebb.Min.y + (Framebb.Max.y / 2) - label_size.y / 2, LEFT, TextFont, false, TextColor, Vec2(Window->Size.x, Window->Size.y), Window->Pos);

	return pressed;
}

//bool ui::ListBoxSelectableCheck(const char* label, bool* p_selected, Vec2 size_arg)
//{
//	if (ListBoxSelectable(label, *p_selected, size_arg))
//	{
//		*p_selected = !*p_selected;
//		return true;
//	}
//	return false;
//}



bool ui::Button(const char* label) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();


	Vec2 size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 25);

	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 25);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 32);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 39);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 46);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 53);
		Offset = Vec2(4, 4);
		break;
	}
	}

	Vec2 DrawPos = Window->CursorPos + Vec2(42 + Offset.x, -3);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,8) }, NULL);

	//bool hovered, held;
	//bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	bool pressed = KeyDown(VK_LBUTTON) && hovered;
	bool to_return = false;

	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);

	if (pressed && !Window->Dragging && g.CurrentItem == "") {
		Window->ItemActive[label] = true;
		g.IsClicking = true;
		g.CurrentItem = label;

	}

	if (hovered && pressed) {
		Render::Draw->Gradient(DrawPos + Vec2(2, 2), size - Vec2(3, 3), D3DCOLOR_RGBA(27, 27, 27, g.MenuAlpha), D3DCOLOR_RGBA(33, 33, 33, g.MenuAlpha), true);
		//eventlogs::get().add("pressed, not hovered", false);
		//debug shit dont mind it
	}
	else if (hovered) {
		Render::Draw->Gradient(DrawPos + Vec2(2, 2), size - Vec2(3, 3), D3DCOLOR_RGBA(39, 39, 39, g.MenuAlpha), D3DCOLOR_RGBA(33, 33, 33, g.MenuAlpha), true);
		//eventlogs::get().add("hovered", false);
		//debug shit dont mind it
	}
	else {
		Render::Draw->Gradient(DrawPos + Vec2(2, 2), size - Vec2(3, 3), D3DCOLOR_RGBA(34, 34, 34, g.MenuAlpha), D3DCOLOR_RGBA(30, 30, 30, g.MenuAlpha), true);
	}

	if (KeyReleased(VK_LBUTTON) && !hovered) {
		Window->ItemActive[label] = false;
		g.IsClicking = false;
		to_return = false;
		g.CurrentItem = "";
	}
	else if (KeyReleased(VK_LBUTTON) && hovered) {
		Window->ItemActive[label] = false;
		g.IsClicking = false;
		to_return = true;
		g.CurrentItem = "";
	}
	Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Rect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), 1, D3DCOLOR_RGBA(50, 50, 50, g.MenuAlpha));
	Vec2 Niggashit = DrawPos + Vec2((size.x - DrawPos.x) / 2 - label_size.x / 2, (size.y - DrawPos.y) / 2 - label_size.y / 2);
	Render::Draw->Text(label, (DrawPos.x + size.x / 2) - label_size.x / 2 + 1, (DrawPos.y + size.y / 2) - label_size.y / 2 + 1, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(15, 15, 15, g.MenuAlpha));
	Render::Draw->Text(label, (DrawPos.x + size.x / 2) - label_size.x / 2, (DrawPos.y + size.y / 2) - label_size.y / 2, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));

	return to_return;
}

bool ui::ButtonDisabled(const char* label) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();



	Vec2 size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 25);

	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 25);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 32);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 39);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 46);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI()), 53);
		Offset = Vec2(4, 4);
		break;
	}


	}

	Vec2 DrawPos = Window->CursorPos + Vec2(42 + Offset.x, -3);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,8) }, NULL);



	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	bool pressed = KeyDown(VK_LBUTTON) && hovered;
	bool to_return = false;

	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);
	Render::Draw->Gradient(DrawPos + Vec2(2, 2), size - Vec2(3, 3), D3DCOLOR_RGBA(34, 34, 34, g.MenuAlpha), D3DCOLOR_RGBA(30, 30, 30, g.MenuAlpha), true);

	Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Rect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), 1, D3DCOLOR_RGBA(50, 50, 50, g.MenuAlpha));
	Vec2 Niggashit = DrawPos + Vec2((size.x - DrawPos.x) / 2 - label_size.x / 2, (size.y - DrawPos.y) / 2 - label_size.y / 2);
	Render::Draw->Text(label, (DrawPos.x + size.x / 2) - label_size.x / 2 + 1, (DrawPos.y + size.y / 2) - label_size.y / 2 + 1, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(21, 21, 21, g.MenuAlpha));
	Render::Draw->Text(label, (DrawPos.x + size.x / 2) - label_size.x / 2, (DrawPos.y + size.y / 2) - label_size.y / 2, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(60, 60, 60, g.MenuAlpha));

	return to_return;
}

bool ui::Checkbox(const char* label, bool* selected) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}


	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);
	bool hovered2 = IsInside(DrawPos.x + 23, DrawPos.y, label_size.x, size.y);
	bool pressed = KeyPressed(VK_LBUTTON) && (hovered || hovered2);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10) + Offset }, NULL);

	if (pressed && !(CMenu::get()->PopUpOpen)) {
		*selected = !(*selected);

	}
	if (*selected) {
		Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), CMenu::get()->MenuColor());
	}
	else {
		if (hovered || hovered2) {
			Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(75, 75, 75, g.MenuAlpha));
		}
		else {
			Render::Draw->Gradient(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(77, 77, 77, g.MenuAlpha), D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), true);
		}
	}
	Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	Render::Draw->Text(label, DrawPos.x + 23 + Offset.x, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));
	return pressed;
}

bool ui::Checkboxinvisible(const char* label, bool* selected) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}


	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	//	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);
		//bool hovered2 = IsInside(DrawPos.x + 23, DrawPos.y, label_size.x, size.y);
	bool pressed = KeyPressed(VK_LBUTTON) && (hovered);

	//AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10) + Offset }, NULL);

	if (pressed && !(CMenu::get()->PopUpOpen)) {
		*selected = !(*selected);

	}
	if (*selected) {
		//	Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), CMenu::get()->MenuColor());
	}
	else {
		//	if (hovered || hovered2) {
				//Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(75, 75, 75, g.MenuAlpha));
			//}
			//else {
				//Render::Draw->Gradient(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(77, 77, 77, g.MenuAlpha), D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), true);
			//}
	}
	//Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	//Render::Draw->Text(label, DrawPos.x + 23 + Offset.x, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));
	return pressed;

}

bool ui::CheckboxDisabled(const char* label, bool* selected) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}


	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);
	bool hovered2 = IsInside(DrawPos.x + 23, DrawPos.y, label_size.x, size.y);
	bool pressed = KeyPressed(VK_LBUTTON) && (hovered || hovered2);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10) + Offset }, NULL);


	Render::Draw->Gradient(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(43, 43, 43, g.MenuAlpha), D3DCOLOR_RGBA(34, 34, 34, g.MenuAlpha), true);

	Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	Render::Draw->Text(label, DrawPos.x + 23, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(83, 83, 83, g.MenuAlpha));

	return pressed;
}

bool ui::CheckboxColor(const char* label) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}


	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);
	bool hovered2 = IsInside(DrawPos.x + 23, DrawPos.y, label_size.x, size.y);
	bool pressed = KeyPressed(VK_LBUTTON) && (hovered || hovered2);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10) + Offset }, NULL);


	Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), CMenu::get()->MenuColor());

	Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	Render::Draw->Text(label, DrawPos.x + 23, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(83, 83, 83, g.MenuAlpha));

	return pressed;
}

bool ui::CheckboxUntrusted(const char* label, bool* selected) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}


	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x, size.y);

	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontDPI(), label);
	bool hovered2 = IsInside(DrawPos.x + 23, DrawPos.y, label_size.x, size.y);
	bool pressed = KeyPressed(VK_LBUTTON) && (hovered || hovered2);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10) + Offset }, NULL);

	if (pressed && !(CMenu::get()->PopUpOpen)) {
		*selected = !(*selected);

	}
	if (*selected) {
		Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), CMenu::get()->MenuColor());
	}
	else {
		if (hovered || hovered2) {
			Render::Draw->FilledRect(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(75, 75, 75, g.MenuAlpha));
		}
		else {
			Render::Draw->Gradient(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(77, 77, 77, g.MenuAlpha), D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), true);
		}
	}
	Render::Draw->Rect(DrawPos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	Render::Draw->Text(label, DrawPos.x + 23 + Offset.x, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(169, 169, 95, g.MenuAlpha));

	return pressed;
}

bool ui::Label(const char* label) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}
	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10 + Offset.y) }, NULL);

	Render::Draw->Text(label, DrawPos.x + 23 + Offset.x, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));

	return true;
}

bool ui::LabelUntrusted(const char* label) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);
	Vec2 Offset = Vec2(0, 0);

	switch (CMenu::get()->DPI()) {
	case 0:
	{
		size = Vec2(8, 8);
		Offset = Vec2(0, 0);
		break;
	}
	case 1:
	{
		size = Vec2(10, 10);
		Offset = Vec2(1, 1);
		break;
	}
	case 2:
	{
		size = Vec2(12, 12);
		Offset = Vec2(2, 2);
		break;
	}
	case 3:
	{
		size = Vec2(14, 14);
		Offset = Vec2(3, 3);
		break;
	}
	case 4:
	{
		size = Vec2(16, 16);
		Offset = Vec2(4, 4);
		break;
	}


	}
	Vec2 DrawPos = Window->CursorPos + Vec2(20, 0);

	AddItemToWindow(Window, Rect{ DrawPos, size + Vec2(0,10 + Offset.y) }, NULL);

	Render::Draw->Text(label, DrawPos.x + 23 + Offset.x, DrawPos.y - 3, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(169, 169, 95, g.MenuAlpha));

	return true;
}

bool ui::Popup(const char* id, int* current_item, std::vector<const char*> items, Vec2 size, Vec2 pos) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	Render::Draw->FilledRect(pos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(35, 35, 35, g.MenuAlpha));
	Render::Draw->Rect(pos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	for (int i = 0; i < items.size(); i++)


		return true;
}

bool ui::Selectable(const char* label, bool selected, Vec2 size_arg, Vec2 pos) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	bool nigga = true;

	bool hovered = IsInsideIgnore(pos.x, pos.y, size_arg.x, size_arg.y);

	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered;

	D3DCOLOR TextColor = selected ? CMenu::get()->MenuColor() : (hovered ? D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha) : D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));

	if (hovered) {
		nigga = false;
		DrawList::AddFilledRect(pos, size_arg, D3DCOLOR_RGBA(26, 26, 26, g.MenuAlpha));
		DrawList::AddText(label, pos.x + 9, pos.y + 3, TextColor, CMenu::get()->GetFontBold(), false, Vec2(Window->Size.x - 15, size_arg.y));

	}
	else if (selected && hovered) {
		nigga = false;
		DrawList::AddFilledRect(pos, size_arg, D3DCOLOR_RGBA(26, 26, 26, g.MenuAlpha));
		DrawList::AddText(label, pos.x + 9, pos.y + 3, TextColor, CMenu::get()->GetFontBold(), false, Vec2(Window->Size.x - 15, size_arg.y));
	}
	else if (selected) {
		nigga = false;
		DrawList::AddText(label, pos.x + 9, pos.y + 3, TextColor, CMenu::get()->GetFontDPI(), false, Vec2(Window->Size.x - 15, size_arg.y));
	}
	else {
		nigga = false;
		DrawList::AddText(label, pos.x + 9, pos.y + 3, TextColor, CMenu::get()->GetFontDPI(), false, Vec2(Window->Size.x - 15, size_arg.y));
	}
	return pressed;

}

bool ui::PopupKeybind(const char* id, int* current_item, std::vector<const char*> items, Vec2 size, Vec2 pos) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	bool gotem = false;

	//Render::Draw->FilledRect(pos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(35, 35, 35, g.MenuAlpha));
	//Render::Draw->Rect(pos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	if (Selectable("Always On", *current_item == 0, Vec2(98, 20), pos + Vec2(1, 1))) {
		*current_item = 0;
		gotem = true;
	}

	if (Selectable("On Hotkey", *current_item == 1, Vec2(98, 20), pos + Vec2(1, 21))) {
		*current_item = 1;
		gotem = true;
	}

	if (Selectable("Toggle", *current_item == 2, Vec2(98, 20), pos + Vec2(1, 41))) {
		*current_item = 2;
		gotem = true;
	}

	if (Selectable("Off Hotkey", *current_item == 3, Vec2(98, 20), pos + Vec2(1, 61))) {
		*current_item = 3;
		gotem = true;
	}

	return gotem;
}

bool ui::PopupColor(const char* id, int* current_item, std::vector<const char*> items, Vec2 size, Vec2 pos) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	bool gotem = false;

	//Render::Draw->FilledRect(pos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(35, 35, 35, g.MenuAlpha));
	//Render::Draw->Rect(pos, size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	if (Selectable(id, *current_item == 135135, Vec2(98, 20), pos + Vec2(1, 1))) {
		*current_item = 0;
		gotem = true;
	}

	return gotem;
}

const char* keys[] = {
	"[-]",
	"[M1]",
	"[M2]",
	"[CN]",
	"[M3]",
	"[M4]",
	"[M5]",
	"[-]",
	"[BAC]",
	"[TAB]",
	"[-]",
	"[-]",
	"[CLR]",
	"[RET]",
	"[-]",
	"[-]",
	"[SHI]",
	"[CTL]",
	"[MEN]",
	"[PAU]",
	"[CAP]",
	"[KAN]",
	"[-]",
	"[JUN]",
	"[FIN]",
	"[KAN]",
	"[-]",
	"[ESC]",
	"[CON]",
	"[NCO]",
	"[ACC]",
	"[MAD]",
	"[SPA]",
	"[PGU]",
	"[PGD]",
	"[END]",
	"[HOM]",
	"[LEF]",
	"[UP]",
	"[RIG]",
	"[DOW]",
	"[SEL]",
	"[PRI]",
	"[EXE]",
	"[PRI]",
	"[INS]",
	"[DEL]",
	"[HEL]",
	"[0]",
	"[1]",
	"[2]",
	"[3]",
	"[4]",
	"[5]",
	"[6]",
	"[7]",
	"[8]",
	"[9]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[A]",
	"[B]",
	"[C]",
	"[D]",
	"[E]",
	"[F]",
	"[G]",
	"[H]",
	"[I]",
	"[J]",
	"[K]",
	"[L]",
	"[M]",
	"[N]",
	"[O]",
	"[P]",
	"[Q]",
	"[R]",
	"[S]",
	"[T]",
	"[U]",
	"[V]",
	"[W]",
	"[X]",
	"[Y]",
	"[Z]",
	"[WIN]",
	"[WIN]",
	"[APP]",
	"[-]",
	"[SLE]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[NUM]",
	"[MUL]",
	"[ADD]",
	"[SEP]",
	"[MIN]",
	"[DEC]",
	"[DIV]",
	"[F1]",
	"[F2]",
	"[F3]",
	"[F4]",
	"[F5]",
	"[F6]",
	"[F7]",
	"[F8]",
	"[F9]",
	"[F10]",
	"[F11]",
	"[F12]",
	"[F13]",
	"[F14]",
	"[F15]",
	"[F16]",
	"[F17]",
	"[F18]",
	"[F19]",
	"[F20]",
	"[F21]",
	"[F22]",
	"[F23]",
	"[F24]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[NUM]",
	"[SCR]",
	"[EQU]",
	"[MAS]",
	"[TOY]",
	"[OYA]",
	"[OYA]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[-]",
	"[SHI]",
	"[SHI]",
	"[CTR]",
	"[CTR]",
	"[ALT]",
	"[ALT]"
};

#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_MBUTTON        0x04
#define VK_XBUTTON1       0x05
#define VK_XBUTTON2       0x06
#define VK_BACK           0x08
#define VK_RMENU          0xA5

template<typename T>
bool ui::SliderBehaviorIDA(const char* item_id, Rect bb, T value, T min_value, T max_value, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* current_window = GetCurrentWindow();
	bool hovered = IsInside(bb.Min.x, bb.Min.y, bb.Max.x, bb.Max.y);

	if (KeyPressed(VK_LBUTTON) && hovered && !current_window->ActiveID[item_id]) {
		current_window->ActiveID[item_id] = true;
		
	}

	if (current_window->ActiveID[item_id]) {
		g.Blocking = true;
		if (flags & GuiFlags_FloatSlider)
			*value = float(std::clamp(g.MousePos.x - bb.Min.x, 0.0f, (bb.Max.x - 1)) / (bb.Max.x - 1) * (*max_value - *min_value) + *min_value);
		else if (flags & GuiFlags_IntSlider)
			*value = int(std::clamp(g.MousePos.x - bb.Min.x, 0.0f, (bb.Max.x - 1)) / (bb.Max.x - 1) * (*max_value - *min_value) + *min_value);
	}

	if (KeyReleased(VK_LBUTTON) && current_window->ActiveID[item_id]) {
		current_window->ActiveID[item_id] = false;
		g.Blocking = false;
	}

	return hovered;
}
bool ui::ButtonBehavior(GuiWindow* window, const char* label, Rect v, bool& hovered, bool& held, GuiFlags flags)
{
	GuiContext& g = *Gui_Ctx;

	hovered = IsInside(v.Min.x, v.Min.y, v.Max.x, v.Max.y);
	held = hovered && KeyDown(VK_LBUTTON);

	return hovered && KeyPressed(VK_LBUTTON);

}


#include <string>
#include <sstream>
#include <iostream>

const char* to_cstr(std::string&& s)
{
	static thread_local std::string sloc;
	sloc = std::move(s);
	return sloc.c_str();
}
template<typename T>
void ui::SliderDMG(const char* label, T* v, T v_min, T v_max, const char* format, GuiFlags flags, float scale, int remove) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), label);
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-42, 3), Vec2(w + (49 * CMenu::get()->DPI()), label_size.y + 12 + (2 * CMenu::get()->DPI())) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, label_size.y + 4), Vec2(frame_bb.Max.x, 6 + (2 * CMenu::get()->DPI())) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1, v_min, v_max);


	//const float rect_width = static_cast<float>(*v) / (v_max - v_min) * w - 1 * (CMenu::get()->DPI() / 2);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w + 1 * CMenu::get()->DPI(); // a bit fixed
	// 
	//const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1 + (49 * CMenu::get()->DPI());

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), formatted_string);
	Render::Draw->Text(label, frame_bb.Min.x, frame_bb.Min.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha));

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	if (showincrements) {
		Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, -1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	}
	std::stringstream ss;
	ss << "HP+" << to_string(*v - 100);

	if (*v == 0)
		Render::Draw->Text("Auto", slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));
	else if (*v > 100)
		Render::Draw->Text(to_cstr(std::move(ss).str()), slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));
	else
		Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));
}

template<typename T>
void ui::Slider2(const char* label, T* v, T v_min, T v_max, const char* format, GuiFlags flags, float scale, int remove) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), label);
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-42, 3), Vec2(w + (49 * CMenu::get()->DPI()), label_size.y + 12 + (2 * CMenu::get()->DPI())) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, label_size.y + 4), Vec2(frame_bb.Max.x, 6 + (2 * CMenu::get()->DPI())) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1.f, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1.f, v_min, v_max);


	//const float rect_width = static_cast<float>(*v) / (v_max - v_min) * w - 1 * (CMenu::get()->DPI() / 2);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w + 1 * CMenu::get()->DPI(); // a bit fixed
	// 
	//const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1 + (49 * CMenu::get()->DPI());

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), formatted_string);
	Render::Draw->Text(label, frame_bb.Min.x, frame_bb.Min.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha));

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	if (showincrements) {
		Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, -1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	}

	Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));
}

template<typename T>
void ui::Slider(const char* label, T* v, T v_min, T v_max, const char* format, GuiFlags flags, float scale, int remove) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), label);
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 102 + 8 + (49 * CMenu::get()->DPI()), 63.f, 200.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-42, 3), Vec2(w , label_size.y + 12 + (2 * CMenu::get()->DPI())) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, label_size.y + 4), Vec2(frame_bb.Max.x, 6 + (2 * CMenu::get()->DPI())) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1, v_min, v_max);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1; // a bit fixed

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), formatted_string);
	Render::Draw->Text(label, frame_bb.Min.x, frame_bb.Min.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha));

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	if (showincrements) {
		Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, -1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	}

	Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));
}

template<typename T>
void ui::SliderNoText(const char* label, T* v, T v_min, T v_max, const char* format, GuiFlags flags, float scale, int remove) {

	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-42, 3), Vec2(w + (49 * CMenu::get()->DPI()),  12 + (2 * CMenu::get()->DPI())) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, 4), Vec2(frame_bb.Max.x, 6 + (2 * CMenu::get()->DPI())) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1, v_min, v_max);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1 + (49 * CMenu::get()->DPI());

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), formatted_string);
	//Render::Draw->Text(label, frame_bb.Min.x, frame_bb.Min.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha));

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	//if (showincrements) {
	Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));

	Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	//}

	Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));/*
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::Tahombd, label);
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 90 - 6, 54.f, 198.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-43 , 3), Vec2(w, label_size.y + 12) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, label_size.y + 4), Vec2(frame_bb.Max.x, 6) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1, v_min, v_max);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1;

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(Render::Fonts::Tahombd, formatted_string);

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	if (showincrements) {
		Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));

		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	}

	Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, Render::Fonts::Tahombd, true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));*/
}

/*
void ui::SliderNoText(const char* label, T* v, T v_min, T v_max, const char* format, GuiFlags flags, float scale, int remove) {

	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-42, 3), Vec2(w + (49 * CMenu::get()->DPI()),  12 + (2 * CMenu::get()->DPI())) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, 4), Vec2(frame_bb.Max.x, 6 + (2 * CMenu::get()->DPI())) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1, v_min, v_max);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1 + (49 * CMenu::get()->DPI());

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), formatted_string);
	//Render::Draw->Text(label, frame_bb.Min.x, frame_bb.Min.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(205, 205, 205, g.MenuAlpha));

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	//if (showincrements) {
		Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));

		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	//}

	Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, CMenu::get()->GetFontBold(), true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));/*
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 label_size = Render::Draw->GetTextSize(Render::Fonts::Tahombd, label);
	char formatted_string[100];

	const float w = std::clamp(Window->Size.x - 90 - 6, 54.f, 198.f);
	Rect frame_bb = { Window->CursorPos - Vec2(-43 , 3), Vec2(w, label_size.y + 12) };
	Rect slider_bb = { frame_bb.Min + Vec2(0, label_size.y + 4), Vec2(frame_bb.Max.x, 6) };

	AddItemToWindow(Window, Rect(frame_bb.Min, frame_bb.Max + Vec2(0, 6)), Flags_None);

	std::vector<D3DCOLOR> bg_color = { D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha) };
	bool hovered, held, pressed;

	bool slider_hovered = false;

	bool hovereded = IsInside(slider_bb.Min.x, slider_bb.Min.y, slider_bb.Max.x, slider_bb.Max.y);

	bool showincrements = false;

	if (slider_bb.Max.x < (int)v_max)
		showincrements = true;

	if (slider_hovered = SliderBehaviorIDA(label, slider_bb, v, &v_min, &v_max, flags))
		bg_color = { D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), D3DCOLOR_RGBA(92, 92, 92, g.MenuAlpha) };

	bool hoveredinc = IsInside(slider_bb.Min.x + slider_bb.Max.x, slider_bb.Min.y, 10, 7);
	bool hoveredinc2 = IsInside(slider_bb.Min.x - 10, slider_bb.Min.y, 10, 7);

	if (hoveredinc && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v + 1, v_min, v_max);
	if (hoveredinc2 && KeyPressed(VK_LBUTTON))
		*v = std::clamp(*v = *v - 1, v_min, v_max);

	const float rect_width = (static_cast<float>(*v) - v_min) / (v_max - v_min) * w - 1;

	if (!format && flags & GuiFlags_IntSlider)
		sprintf_s(formatted_string, "%d", (int)*v - remove);
	else if (!format && GuiFlags_FloatSlider)
		sprintf_s(formatted_string, "%f", (float)*v);
	else
		sprintf_s(formatted_string, format, T(*v) - remove);

	Vec2 value_width = Render::Draw->GetTextSize(Render::Fonts::Tahombd, formatted_string);

	Render::Draw->Gradient(slider_bb.Min + Vec2(0, 1), slider_bb.Max - Vec2(0, 1), bg_color[0], bg_color[1], true);

	Render::Draw->FilledRect(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), CMenu::get()->MenuColor());
	Render::Draw->Rect(slider_bb.Min, slider_bb.Max + Vec2(0, 1), 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Gradient(slider_bb.Min + Vec2(1, 1), Vec2(rect_width, slider_bb.Max.y), D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(0, 0, 0, std::clamp(120, 0, g.MenuAlpha)), true);

	if (showincrements) {
		Render::Draw->FilledRect(slider_bb.Min + Vec2(-7, 3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));

		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(3, -3), Vec2(3, 1), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
		Render::Draw->FilledRect(slider_bb.Min + slider_bb.Max + Vec2(4, -4), Vec2(1, 3), D3DCOLOR_RGBA(100, 100, 100, g.MenuAlpha));
	}

	Render::Draw->Text(formatted_string, slider_bb.Min.x + rect_width - value_width.x / 2, slider_bb.Min.y, LEFT, Render::Fonts::Tahombd, true, D3DCOLOR_RGBA(255, 255, 255, min(200, g.MenuAlpha)));
}
*/



void ui::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float scale) {
	return Slider2(label, v, v_min, v_max, format, GuiFlags_FloatSlider, scale);
}

void ui::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, int remove) {
	return Slider(label, v, v_min, v_max, format, GuiFlags_IntSlider, 1.f, remove);
}

void ui::SliderIntDMG(const char* label, int* v, int v_min, int v_max, const char* format, int remove) {
	return SliderDMG(label, v, v_min, v_max, format, GuiFlags_IntSlider, 1.f, remove);
}

void ui::SliderIntNoText(const char* label, int* v, int v_min, int v_max, const char* format, int remove) {
	return SliderNoText(label, v, v_min, v_max, format, GuiFlags_IntSlider, 1.f, remove);
}

bool ui::Keybind(const char* label, int* current_key, int* keystyle) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();
	Vec2 size = Vec2(8, 8);

	std::string niggae = std::string(label);
	char buf_display[64] = "[-]";
	strcpy_s(buf_display, keys[*current_key]);
	const Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetKeybindDPI(), buf_display);

	bool edit_requested = Window->ActiveID[label];

	static bool nigga = false;
	int niggggg;

	Vec2 DrawPos = Window->PrevWidgetPos + Vec2(Window->Size.x - label_size.x - 19 - 6, 0);
	bool hovered = IsInside(DrawPos.x, DrawPos.y, size.x + 2, size.y);

	bool pressed = KeyPressed(VK_LBUTTON) && hovered;

	bool pressed2 = KeyPressed(VK_RBUTTON) && hovered;

	bool notfocused = KeyPressed(VK_LBUTTON || VK_RBUTTON) && !hovered;

	D3DCOLOR TextColor = edit_requested && !nigga ? D3DCOLOR_RGBA(255, 0, 0, g.MenuAlpha) : (hovered ? D3DCOLOR_RGBA(141, 141, 141, g.MenuAlpha) : D3DCOLOR_RGBA(114, 114, 114, g.MenuAlpha));

	Render::Draw->Text(buf_display, DrawPos.x, DrawPos.y - 3, LEFT, CMenu::get()->GetKeybindDPI(), true, TextColor);

	bool hoveredpopup = IsInside(DrawPos.x - 109, DrawPos.y - 6, 100, 82);


	if (pressed) {
		Window->ActiveID[label] = true;
	}

	if (edit_requested) {
		for (int i = 0; i < 166; i++) {
			if (g.KeyState[i]) {
				*current_key = i == VK_LBUTTON ? 0 : i;
			}

			if (KeyPressed(i))
				Window->ActiveID[label] = false;
		}
	}

	SetNextWindowPos(DrawPos - Vec2(9 + 100, 6));
	SetNextWindowSize({ 100,100 - 18 });

	Begin(label, Flags_PopUp);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Opened = false;
		PickerWindow->Init = true;
	}

	if (pressed2) {
		PickerWindow->Opened = true;
	}


	if (KeyPressed(VK_LBUTTON) && IsInside(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInside(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInside(DrawPos.x, DrawPos.y, size.x, size.y)) {
		Window->ActiveID[label] = false;
		PickerWindow->Opened = false;
		g.DontMove = false;
	}

	if (PickerWindow->Opened) {
		g.DontMove = true;
		if (PopupKeybind(label, keystyle, { "Default", "Solid", "Shaded", "Metallic" }, Vec2(100, 82), DrawPos - Vec2(9 + 100, 6))) {
			Window->ActiveID[label] = false;
			PickerWindow->Opened = false;
			g.DontMove = false;
		}
	}

	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	SetCurrentWindow(Window);



	return c_config::get()->auto_check(*current_key, *keystyle);
}



HSV ui::ColorPickerBehavior(GuiWindow* PickerWindow, Rect& RcColor, Rect& RcAlpha, Rect& RcHue, int col[4], bool reset) {
	GuiContext& g = *Gui_Ctx;

	static int ActiveBar = -1;

	CColor Ccol = CColor(col[0], col[1], col[2], col[3]);

	static HSV hsv = HSV(Ccol.Hue(), Ccol.Saturation(), Ccol.Brightness(), Ccol.a());

	bool h = false;
	bool sv = false;
	bool a = false;

	if (reset)
		hsv = HSV(Ccol.Hue(), Ccol.Saturation(), Ccol.Brightness(), Ccol.a());

	//bool hovered, held;
	//ButtonBehavior(PickerWindow->ParentWindow, PickerWindow->Name.c_str(), { PickerWindow->Pos, PickerWindow->Size }, hovered, held);

	if (KeyDown(VK_LBUTTON)) {
		if ((IsInsideIgnore(RcColor.Min.x, RcColor.Min.y, RcColor.Max.x, RcColor.Max.y) || ActiveBar == 0) && ActiveBar != 1 && ActiveBar != 2) {
			ActiveBar = 0;
			sv = true;
			a = false;
			h = false;
		}
		else if ((IsInsideIgnore(RcHue.Min.x, RcHue.Min.y, RcHue.Max.x, RcHue.Max.y) || ActiveBar == 1) && ActiveBar != 0 && ActiveBar != 2) {
			ActiveBar = 1;
			h = true;
			sv = false;
			a = false;
		}
		else if ((IsInsideIgnore(RcAlpha.Min.x, RcAlpha.Min.y, RcAlpha.Max.x, RcAlpha.Max.y) || ActiveBar == 2) && ActiveBar != 0 && ActiveBar != 1) {
			ActiveBar = 2;
			a = true;
			sv = false;
			h = false;
		}
	}
	else
		ActiveBar = -1;

	switch (ActiveBar) {
	case 0:
		if (sv) {
			hsv.s = (g.MousePos.x - RcColor.Min.x) / RcColor.Max.x;
			hsv.v = 1.f - (g.MousePos.y - RcColor.Min.y) / RcColor.Max.y;
		} break;
	case 1:
		if (h)
			hsv.h = 1.f - (g.MousePos.y - RcHue.Min.y) / RcHue.Max.y;
		break;
	case 2:
		if (a)
			hsv.a = std::clamp(((g.MousePos.x - RcAlpha.Min.x) / RcAlpha.Max.x) * 255, 0.f, 255.f);
		break;
	}

	return hsv;
}
#include <iostream>
#include <sstream>
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

std::string rgb2hex(int r, int g, int b, int a)
{
	std::stringstream ss;
	ss << std::hex << (r << 24 | g << 16 | b << 8 | a);

	ss = std::stringstream(ReplaceAll(ss.str(), std::string("a"), std::string("A")));
	ss = std::stringstream(ReplaceAll(ss.str(), std::string("b"), std::string("B")));
	ss = std::stringstream(ReplaceAll(ss.str(), std::string("c"), std::string("C")));
	ss = std::stringstream(ReplaceAll(ss.str(), std::string("d"), std::string("D")));
	ss = std::stringstream(ReplaceAll(ss.str(), std::string("e"), std::string("E")));
	ss = std::stringstream(ReplaceAll(ss.str(), std::string("f"), std::string("F")));

	return ss.str();
}

#include "../clip.h"

bool ui::ColorPicker(const char* label, int col[4], GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	flags |= Flags_PopUp | Flags_ColorPicker;

	Rect Fullbb = { Window->PrevWidgetPos.x + Window->Size.x - 19 - 17 - 5, Window->PrevWidgetPos.y, 17, 9 };

	Vec2 DrawPos = Vec2(Fullbb.Min);

	bool hovered, held;
	hovered = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered;
	static bool init = false;

	//AddItemToWindow(Window, Fullbb, Flags_ColorPicker);

	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->FilledRect(Fullbb.Min + Vec2(1, 1), Fullbb.Max - Vec2(2, 2), D3DCOLOR_RGBA(col[0], col[1], col[2], g.MenuAlpha));
	Render::Draw->Gradient(Fullbb.Min + Vec2(1, 1), Fullbb.Max - Vec2(2, 2), D3DCOLOR_RGBA(255, 255, 255, 20), D3DCOLOR_RGBA(0, 0, 0, 20), true);

	SetNextWindowPos(Fullbb.Min + Vec2(-1, Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(180, 175));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	Rect ColorRect = { PickerWindow->Pos + Vec2(5, 5), Vec2(150, 150) };
	Rect AlphaRect = { PickerWindow->Pos + Vec2(5, 160), Vec2(150, 10) };
	Rect HueRect = { PickerWindow->Pos + Vec2(160, 5), Vec2(15, 150) };

	if (pressed)
		PickerWindow->Opened = init = !PickerWindow->Opened;


	if (KeyPressedIgnore(VK_LBUTTON) && IsInsideIgnore(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInsideIgnore(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		CMenu::get()->PopUpOpen = false;
		g.Blocking = false;
	}

	if (PickerWindow->Opened) {
		CMenu::get()->PopUpOpen = true;
		g.Blocking = true;
		HSV ColHSV = ColorPickerBehavior(PickerWindow, ColorRect, AlphaRect, HueRect, col, init);
		CColor Topr = CColor::FromHSB(ColHSV.h, 1.f, 1.f);

		Vec2 ColorPos = { std::clamp(ColorRect.Min.x + int(std::roundf(ColHSV.s * ColorRect.Max.x)), ColorRect.Min.x + 2, ColorRect.Min.x + ColorRect.Max.x - 3), std::clamp(ColorRect.Min.y + int((1.f - ColHSV.v) * ColorRect.Max.y), ColorRect.Min.y + 2, ColorRect.Min.y + ColorRect.Max.y - 3) };
		Vec2 HuePos = { HueRect.Min.x, std::clamp(HueRect.Min.y + int((1.f - ColHSV.h) * HueRect.Max.y), HueRect.Min.y, HueRect.Min.y + HueRect.Max.y) };
		Vec2 AlphaPos = { std::clamp(AlphaRect.Min.x + int(AlphaRect.Max.x * (ColHSV.a / 255.f)), AlphaRect.Min.x, AlphaRect.Min.x + AlphaRect.Max.x), AlphaRect.Min.y };

		// Color Picker
		DrawList::AddGradient(ColorRect.Min, ColorRect.Max, D3DCOLOR_RGBA(255, 255, 255, g.MenuAlpha), D3DCOLOR_RGBA(Topr.r(), Topr.g(), Topr.b(), 255), false, false);
		DrawList::AddGradient(ColorRect.Min, ColorRect.Max, D3DCOLOR_RGBA(0, 0, 0, std::clamp(0, 0, g.MenuAlpha)), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha), true, false);
		DrawList::AddRect(ColorRect.Min - Vec2(1, 1), Vec2(152, 152), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

		//Alpha Bar
		DrawList::AddRect(AlphaRect.Min - Vec2(1, 1), Vec2(152, 12), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		DrawList::AddFilledRect(AlphaRect.Min, AlphaRect.Max, D3DCOLOR_RGBA(col[0], col[1], col[2], col[3]));

		//Hue Bar
		DrawList::AddRect(HueRect.Min - Vec2(1, 1), Vec2(17, 152), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		const D3DCOLOR hue_colors[6 + 1] = { D3DCOLOR_RGBA(220, 30, 34, 255), D3DCOLOR_RGBA(220, 30, 216, 255), D3DCOLOR_RGBA(30, 34, 220, 255), D3DCOLOR_RGBA(30,220,216,255), D3DCOLOR_RGBA(34,219,30,255), D3DCOLOR_RGBA(220,187,30,255), D3DCOLOR_RGBA(220,33,30,255) };
		for (int i = 0; i < 6; ++i)
			DrawList::AddGradient(PickerWindow->Pos + Vec2(160, 5 + (25 * i)), Vec2(15, 25), hue_colors[i], hue_colors[i + 1], true);

		//Picker Dot
		DrawList::AddRect(ColorPos - Vec2(2, 2), Vec2(4, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		DrawList::AddRect(ColorPos - Vec2(1, 1), Vec2(2, 2), D3DCOLOR_RGBA(255, 255, 255, std::clamp(127, 0, g.MenuAlpha)));

		//Alpha Bar
		DrawList::AddRect(AlphaPos + Vec2(0, 0), Vec2(4, 10), D3DCOLOR_RGBA(12, 12, 12, std::clamp(255, 0, g.MenuAlpha)));
		DrawList::AddRect(AlphaPos + Vec2(1, 1), Vec2(2, 8), D3DCOLOR_RGBA(255, 255, 255, std::clamp(140, 0, g.MenuAlpha)));

		//Hue Indicator
		DrawList::AddRect(HuePos - Vec2(0, 2), Vec2(15, 4), D3DCOLOR_RGBA(12, 12, 12, std::clamp(180, 0, g.MenuAlpha)));
		DrawList::AddRect(HuePos - Vec2(-1, 1), Vec2(13, 2), D3DCOLOR_RGBA(255, 255, 255, std::clamp(140, 0, g.MenuAlpha)));

		CColor outcolor = CColor::FromHSB(ColHSV.h, ColHSV.s, ColHSV.v, ColHSV.a);
		col[0] = outcolor.r();
		col[1] = outcolor.g();
		col[2] = outcolor.b();
		col[3] = outcolor.a();
	}
	std::string idn = std::string(label) + "pop";
	if (KeyPressedIgnore(VK_RBUTTON) && hovered) {
		Window->SelectedId[label] = label;
	}

	if (Window->SelectedId[label] == label) {
		CMenu::get()->PopUpOpen = true;
		SetNextWindowPos(DrawPos - Vec2(Fullbb.Max.x, 0) + Vec2(-64, 13));
		SetNextWindowSize({ 100,43 });
		Begin(idn.c_str(), Flags_PopUp);
		GuiWindow* PickerWindow = GetCurrentWindow();
		PickerWindow->ParentWindow = Window;
		Vec2 poser = DrawPos - Vec2(Fullbb.Max.x, 0) + Vec2(-64, 13);
		if (KeyPressedIgnore(VK_LBUTTON) || KeyPressedIgnore(VK_RBUTTON) && !IsInsideIgnore(poser.x, poser.y, 100, 43) && !hovered)
			Window->SelectedId[label] = "";

		if (!PickerWindow->Init) {
			Window->PopUpWindows.push_back(PickerWindow);
			PickerWindow->Init = true;
		}
		PickerWindow->Opened = true;

		if (KeyPressed(VK_LBUTTON) || KeyPressedIgnore(VK_RBUTTON) && IsInsideIgnore(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
			&& !IsInsideIgnore(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
			&& !IsInsideIgnore(DrawPos.x, DrawPos.y, Fullbb.Max.x, Fullbb.Max.y)) {
			//	Window->SelectedId[label] = "";
			//	PickerWindow->Opened = false;
			g.DontMove = false;
		}

		static int asd;
		if (PickerWindow->Opened) {
			g.DontMove = true;
			if (PopupColor("Copy", &asd, { "124", "1513" }, Vec2(22, 22), DrawPos - Vec2(Fullbb.Max.x, 0) + Vec2(-64, 13))) {
				clip::set_text(rgb2hex(col[0], col[1], col[2], col[3]));
				Window->SelectedId[label] = "";
			}
			if (PopupColor("Paste", &asd, { "124", "1513" }, Vec2(22, 22), DrawPos - Vec2(Fullbb.Max.x, 0) + Vec2(-64, 13 + 21))) {

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

				std::string colt = "255,255,255,255";

				if (clip::get_text(colt)) {
					int r, g, b, a;
					sscanf(colt.c_str(), "%02x%02x%02x%02x", &r, &g, &b, &a);
					colt = std::string(std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + "," + std::to_string(a));
					std::vector<std::string> cols = split(colt, ",");
					if (cols.size() == 4) {
						col[0] = std::stoi(cols.at(0));
						col[1] = std::stoi(cols.at(1));
						col[2] = std::stoi(cols.at(2));
						col[3] = std::stoi(cols.at(3));
					}
				}
				Window->SelectedId[label] = "";
			}
		}

		//End();
		SetCurrentWindow(PickerWindow->ParentWindow);

	}
	else {
		g.DontMove = false;
		CMenu::get()->PopUpOpen = false;
	}
	SetCurrentWindow(Window);

	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

bool ui::ColorPickerC(const char* label, CColor* col, GuiFlags flags) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	flags |= Flags_PopUp | Flags_ColorPicker;

	Rect Fullbb = { Window->PrevWidgetPos.x + Window->Size.x - 19 - 17, Window->PrevWidgetPos.y, 17, 9 };

	Vec2 DrawPos = Vec2(Fullbb.Min);

	bool hovered, held;
	hovered = IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressed(VK_LBUTTON) && hovered;
	static bool init = false;

	D3DCOLOR ColorOfPicker = D3DCOLOR_RGBA((int)col->r(), (int)col->g(), (int)col->b(), g.MenuAlpha);
	D3DCOLOR ColorOfPickerWithAlpha = D3DCOLOR_RGBA((int)col->r(), (int)col->g(), (int)col->b(), (int)col->a());

	//AddItemToWindow(Window, Fullbb, Flags_ColorPicker);

	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->FilledRect(Fullbb.Min + Vec2(1, 1), Fullbb.Max - Vec2(2, 2), ColorOfPicker);
	Render::Draw->Gradient(Fullbb.Min + Vec2(1, 1), Fullbb.Max - Vec2(2, 2), D3DCOLOR_RGBA(255, 255, 255, 20), D3DCOLOR_RGBA(0, 0, 0, 20), true);

	SetNextWindowPos(Fullbb.Min + Vec2(-1, Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(180, 175));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	Rect ColorRect = { PickerWindow->Pos + Vec2(5, 5), Vec2(150, 150) };
	Rect AlphaRect = { PickerWindow->Pos + Vec2(5, 160), Vec2(150, 10) };
	Rect HueRect = { PickerWindow->Pos + Vec2(160, 5), Vec2(15, 150) };

	if (pressed)
		PickerWindow->Opened = init = !PickerWindow->Opened;


	if (KeyPressed(VK_LBUTTON) && IsInside(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInside(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		CMenu::get()->PopUpOpen = false;
		g.DontMove = false;
	}

	int niggacol[4] = { (int)col->r(), (int)col->g(), (int)col->b(), (int)col->a() };

	if (PickerWindow->Opened) {
		CMenu::get()->PopUpOpen = true;
		g.DontMove = true;
		HSV ColHSV = ColorPickerBehavior(PickerWindow, ColorRect, AlphaRect, HueRect, niggacol, init);
		CColor Topr = CColor::FromHSB(ColHSV.h, 1.f, 1.f);

		Vec2 ColorPos = { std::clamp(ColorRect.Min.x + int(std::roundf(ColHSV.s * ColorRect.Max.x)), ColorRect.Min.x, ColorRect.Min.x + ColorRect.Max.x), std::clamp(ColorRect.Min.y + int((1.f - ColHSV.v) * ColorRect.Max.y), ColorRect.Min.y, ColorRect.Min.y + ColorRect.Max.y) };
		Vec2 HuePos = { HueRect.Min.x, std::clamp(HueRect.Min.y + int((1.f - ColHSV.h) * HueRect.Max.y), HueRect.Min.y, HueRect.Min.y + HueRect.Max.y) };
		Vec2 AlphaPos = { std::clamp(AlphaRect.Min.x + int(AlphaRect.Max.x * (ColHSV.a / 255.f)), AlphaRect.Min.x, AlphaRect.Min.x + AlphaRect.Max.x), AlphaRect.Min.y };

		// Color Picker
		DrawList::AddGradient(ColorRect.Min, ColorRect.Max, D3DCOLOR_RGBA(255, 255, 255, g.MenuAlpha), D3DCOLOR_RGBA(Topr.r(), Topr.g(), Topr.b(), 255), false, false);
		DrawList::AddGradient(ColorRect.Min, ColorRect.Max, D3DCOLOR_RGBA(0, 0, 0, std::clamp(0, 0, g.MenuAlpha)), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha), true, false);
		DrawList::AddRect(ColorRect.Min - Vec2(1, 1), Vec2(152, 152), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

		//Alpha Bar
		DrawList::AddRect(AlphaRect.Min - Vec2(1, 1), Vec2(152, 12), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		DrawList::AddFilledRect(AlphaRect.Min, AlphaRect.Max, ColorOfPickerWithAlpha);

		//Hue Bar
		DrawList::AddRect(HueRect.Min - Vec2(1, 1), Vec2(17, 152), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		const D3DCOLOR hue_colors[6 + 1] = { D3DCOLOR_RGBA(220, 30, 34, 255), D3DCOLOR_RGBA(220, 30, 216, 255), D3DCOLOR_RGBA(30, 34, 220, 255), D3DCOLOR_RGBA(30,220,216,255), D3DCOLOR_RGBA(34,219,30,255), D3DCOLOR_RGBA(220,187,30,255), D3DCOLOR_RGBA(220,33,30,255) };
		for (int i = 0; i < 6; ++i)
			DrawList::AddGradient(PickerWindow->Pos + Vec2(160, 5 + (25 * i)), Vec2(15, 25), hue_colors[i], hue_colors[i + 1], true);

		//Picker Dot
		DrawList::AddRect(ColorPos - Vec2(2, 2), Vec2(4, 4), D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
		DrawList::AddRect(ColorPos - Vec2(1, 1), Vec2(2, 2), D3DCOLOR_RGBA(255, 255, 255, std::clamp(127, 0, g.MenuAlpha)));

		//Alpha Bar
		DrawList::AddRect(AlphaPos + Vec2(0, 0), Vec2(4, 10), D3DCOLOR_RGBA(12, 12, 12, std::clamp(255, 0, g.MenuAlpha)));
		DrawList::AddRect(AlphaPos + Vec2(1, 1), Vec2(2, 8), D3DCOLOR_RGBA(255, 255, 255, std::clamp(140, 0, g.MenuAlpha)));

		//Hue Indicator
		DrawList::AddRect(HuePos - Vec2(0, 2), Vec2(15, 4), D3DCOLOR_RGBA(12, 12, 12, std::clamp(180, 0, g.MenuAlpha)));
		DrawList::AddRect(HuePos - Vec2(-1, 1), Vec2(13, 2), D3DCOLOR_RGBA(255, 255, 255, std::clamp(140, 0, g.MenuAlpha)));

		*col = CColor::FromHSB(ColHSV.h, ColHSV.s, ColHSV.v, ColHSV.a);
	}

	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

auto whatsel = [](std::unordered_map<int, bool> e, std::vector<const char*> items)-> std::string {
	auto items1 = items;
	std::string s = "";
	for (int i = 0; i < e.size(); i++)
		if (e[i])
			s = std::string(s + std::string(items1.at(i)) + ", ");

	return s.substr(0, s.size() - 2);
};

bool ui::SelectableCheck(const char* label, bool* p_selected, Vec2 size_arg, Vec2 pos_arg)
{
	if (Selectable(label, *p_selected, size_arg, pos_arg))
	{
		*p_selected = !*p_selected;
		return true;
	}
	return false;
}

template<typename T>
bool allEqual(std::vector<T> const& v) {
	if (v.size() == 0) {
		return false;
	}

	return std::all_of(v.begin(), v.end(), [&](T const& e) {
		return e == 0;
		});
}



bool ui::MultiSelect(const char* label, std::vector<int>& data, std::vector<const char*> items) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Vec2 xer = Window->CursorPos + Vec2(42, 0);
	Rect Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
	Rect nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
	Vec2 Offset = Vec2(0, 0);
	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Offset = Vec2(0, 0);
		Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
		nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 + Offset.y };
		break;
	}
	case 1:
	{
		Offset = Vec2(1, 1);
		Fullbb = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 25 };
		nigga = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 45 + Offset.y };
		break;
	}
	case 2:
	{
		Offset = Vec2(2, 2);
		Fullbb = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 30 };
		nigga = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 50 + Offset.y };
		break;
	}
	case 3:
	{
		Offset = Vec2(3, 3);
		Fullbb = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 35 };
		nigga = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 55 + Offset.y };
		break;
	}
	case 4:
	{
		Offset = Vec2(4, 4);
		Fullbb = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
		nigga = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 60 + Offset.y };
		break;
	}


	}
	AddItemToWindow(Window, nigga, NULL);


	Vec2 DrawPos = Vec2(Fullbb.Min + Vec2(0, Offset.y));

	bool canopen = true;

	bool hovered, held;
	hovered = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered;
	static bool init = false;

	Render::Draw->Text(label, DrawPos.x + 1, DrawPos.y - 14 - (Offset.y * 2), LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));


	SetNextWindowPos(Vec2(Fullbb.Min.x, Fullbb.Min.y + Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(Fullbb.Max.x, (20 + (5 * CMenu::get()->DPI())) * items.size()));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	int Combocurpos = 0;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	canopen = true;

	if (pressed && canopen && g.DontMove == false)
		PickerWindow->Opened = init = !PickerWindow->Opened;

	if (KeyPressedIgnore(VK_LBUTTON) && IsInsideIgnore(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInsideIgnore(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		CMenu::get()->PopUpOpen = false;
		g.DontMove == false;
	}

	static auto howmuchsel = [](std::unordered_map<int, bool> e) -> int {
		int s = 0;
		for (int i = 0; i < e.size(); i++)
			if (e[i])
				s++;

		return s;
	};



	for (int i = 0, j = 0; i < items.size(); i++)
	{
		if (data[i])
		{
			if (j)
				Window->Preview[label] += std::string(", ") + (std::string)items[i];
			else
				Window->Preview[label] = items[i];

			j++;
		}
	}

	if (PickerWindow->Opened) {
		g.DontMove = true;

		if (KeyPressedIgnore(VK_RBUTTON))
			PickerWindow->Opened = false;

		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);
		CMenu::get()->Clicked == "Moving";
		CMenu::get()->PopUpOpen = true;
		CMenu::get()->popupname = label;
		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 8), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 7), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));

		//std::unordered_map<int, bool> old_data = *data;




		for (auto i = 0; i < items.size(); i++) {

			ui::SelectableCheck(items.at(i), (bool*)&data[i], Vec2(float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) - 2, 18 + (5 * CMenu::get()->DPI())), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2 + Combocurpos));
			Combocurpos += 20 + (5 * CMenu::get()->DPI());
		}


		//Selectable("100%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2));
		//Selectable("125%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 22));
		//Selectable("150%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 42));
		//Selectable("175%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 62));
	}
	else if (hovered) {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	else {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(31, 31, 31, g.MenuAlpha), D3DCOLOR_RGBA(36, 36, 36, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	const char* buf_display = "";

	buf_display = Window->Preview[label].c_str();

	if (allEqual(data))
		buf_display = "-";


	Render::Draw->Text(buf_display, DrawPos.x + 10, DrawPos.y + 4 + Offset.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(142, 142, 142, g.MenuAlpha));
	Render::Draw->FilledRect(Fullbb.Min + Vec2(Fullbb.Max.x - 1, 0), Fullbb.Max, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->FilledRect(Fullbb.Min + Vec2(Fullbb.Max.x, 0), Fullbb.Max, D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha));
	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);
	return true;
}

bool ui::MultiSelectOld(const char* label, std::unordered_map<int, bool>* data, std::vector<const char*> items) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Vec2 xer = Window->CursorPos + Vec2(42, 0);

	Rect Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
	Rect nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
	Vec2 Offset = Vec2(0, 0);
	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Offset = Vec2(0, 0);
		Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
		nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 + Offset.y };
		break;
	}
	case 1:
	{
		Offset = Vec2(1, 1);
		Fullbb = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 25 };
		nigga = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 45 + Offset.y };
		break;
	}
	case 2:
	{
		Offset = Vec2(2, 2);
		Fullbb = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 30 };
		nigga = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 50 + Offset.y };
		break;
	}
	case 3:
	{
		Offset = Vec2(3, 3);
		Fullbb = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 35 };
		nigga = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 55 + Offset.y };
		break;
	}
	case 4:
	{
		Offset = Vec2(4, 4);
		Fullbb = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
		nigga = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 60 + Offset.y };
		break;
	}


	}
	AddItemToWindow(Window, nigga, NULL);


	Vec2 DrawPos = Vec2(Fullbb.Min + Vec2(0, Offset.y));

	bool canopen = true;

	bool hovered, held;
	hovered = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered;
	static bool init = false;

	Render::Draw->Text(label, DrawPos.x + 1, DrawPos.y - 14 - (Offset.y * 2), LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));


	SetNextWindowPos(Vec2(Fullbb.Min.x, Fullbb.Min.y + Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(Fullbb.Max.x, (20 + (5 * CMenu::get()->DPI())) * items.size()));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	int Combocurpos = 0;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	canopen = true;

	if (pressed && canopen && g.DontMove == false)
		PickerWindow->Opened = init = !PickerWindow->Opened;

	if (KeyPressedIgnore(VK_LBUTTON) && IsInsideIgnore(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInsideIgnore(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		CMenu::get()->PopUpOpen = false;
		g.DontMove == false;
	}

	static auto howmuchsel = [](std::unordered_map<int, bool> e) -> int {
		int s = 0;
		for (int i = 0; i < e.size(); i++)
			if (e[i])
				s++;

		return s;
	};

	for (int i = 0, j = 0; i < items.size(); i++)
	{
		if ((*data)[i])
		{
			if (j)
				Window->Preview[label] += std::string(", ") + (std::string)items[i];
			else
				Window->Preview[label] = items[i];

			j++;
		}
	}

	if (PickerWindow->Opened) {
		g.DontMove = true;

		if (KeyPressedIgnore(VK_RBUTTON))
			PickerWindow->Opened = false;

		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);
		CMenu::get()->Clicked == "Moving";
		CMenu::get()->PopUpOpen = true;
		CMenu::get()->popupname = label;
		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 8), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 7), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));

		//std::unordered_map<int, bool> old_data = *data;




		for (int i = 0; i < items.size(); i++) {
			ui::SelectableCheck(items.at(i), &(*data)[i], Vec2(float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) - 2, 18 + (5 * CMenu::get()->DPI())), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2 + Combocurpos));
			Combocurpos += 20 + (5 * CMenu::get()->DPI());
		}


		//Selectable("100%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2));
		//Selectable("125%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 22));
		//Selectable("150%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 42));
		//Selectable("175%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 62));
	}
	else if (hovered) {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	else {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(31, 31, 31, g.MenuAlpha), D3DCOLOR_RGBA(36, 36, 36, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	const char* buf_display = "-";

	buf_display = Window->Preview[label].c_str();

	if (howmuchsel(*data) == 0)
		buf_display = "-";

	Render::Draw->Text(buf_display, DrawPos.x + 10, DrawPos.y + 4 + Offset.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(142, 142, 142, g.MenuAlpha), Vec2(Fullbb.Max.x - 30, 20));
	Render::Draw->FilledRect(Fullbb.Min + Vec2(Fullbb.Max.x - 1, 0), Fullbb.Max, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->FilledRect(Fullbb.Min + Vec2(Fullbb.Max.x, 0), Fullbb.Max, D3DCOLOR_RGBA(23, 23, 23, g.MenuAlpha));
	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

bool ui::SingelSelect(const char* label, int* current_item, std::vector<const char*> items) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Vec2 xer = Window->CursorPos + Vec2(42, 0);

	Rect Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
	Rect nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
	Vec2 Offset = Vec2(0, 0);
	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Offset = Vec2(0, 0);
		Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
		nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 + Offset.y };
		break;
	}
	case 1:
	{
		Offset = Vec2(1, 1);
		Fullbb = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 25 };
		nigga = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 45 + Offset.y };
		break;
	}
	case 2:
	{
		Offset = Vec2(2, 2);
		Fullbb = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 30 };
		nigga = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 50 + Offset.y };
		break;
	}
	case 3:
	{
		Offset = Vec2(3, 3);
		Fullbb = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 35 };
		nigga = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 55 + Offset.y };
		break;
	}
	case 4:
	{
		Offset = Vec2(4, 4);
		Fullbb = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
		nigga = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 60 + Offset.y };
		break;
	}


	}
	AddItemToWindow(Window, nigga, NULL);


	Vec2 DrawPos = Vec2(Fullbb.Min + Vec2(0, Offset.y));

	bool canopen = true;

	bool hovered, held;
	hovered = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered;
	static bool init = false;

	Render::Draw->Text(label, DrawPos.x + 1, DrawPos.y - 14 - (Offset.y * 2), LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));

	SetNextWindowPos(Vec2(Fullbb.Min.x, Fullbb.Min.y + Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(Fullbb.Max.x, (20 + (5 * CMenu::get()->DPI())) * items.size()));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	int Combocurpos = 0;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	canopen = true;

	if (pressed && canopen && g.DontMove == false)
		PickerWindow->Opened = init = !PickerWindow->Opened;

	if (KeyPressedIgnore(VK_LBUTTON) && IsInsideIgnore(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInsideIgnore(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		g.DontMove = false;
		CMenu::get()->PopUpOpen = false;
		CMenu::get()->Clicked == "";
		CMenu::get()->popupname = "";
	}

	if (PickerWindow->Opened) {

		g.DontMove = true;
		CMenu::get()->PopUpOpen = true;

		if (KeyPressedIgnore(VK_RBUTTON))
			PickerWindow->Opened = false;

		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);
		//	CMenu::get()->Clicked == "Moving";
		//	CMenu::get()->popupname = label;
		//	CMenu::get()->PopUpOpen = true;
			//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 8), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 7), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));

		for (int i = 0; i < items.size(); i++) {
			if (ui::Selectable(items.at(i), *current_item == i, Vec2(float(std::clamp(Window->Size.x - 102 + 6, 63.f, 200.f) + (49 * CMenu::get()->DPI())) - 2, 18 + (5 * CMenu::get()->DPI())), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2 + Combocurpos))) {
				*current_item = i;
				PickerWindow->Opened = false;
				CMenu::get()->PopUpOpen = false;
				CMenu::get()->Clicked == "";
				CMenu::get()->popupname = "";
			}
			Combocurpos += 20 + (5 * CMenu::get()->DPI());
		}
	}
	else if (hovered) {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	else {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(31, 31, 31, g.MenuAlpha), D3DCOLOR_RGBA(36, 36, 36, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	const char* buf_display = "";

	buf_display = items.at(*current_item);


	Render::Draw->Text(buf_display, DrawPos.x + 10, DrawPos.y + 4 + Offset.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(142, 142, 142, g.MenuAlpha), Vec2(Fullbb.Max.x - 30, 20));
	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

bool ui::SingelSelectDisabled(const char* label, int* current_item, std::vector<const char*> items) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Vec2 xer = Window->CursorPos + Vec2(42, 0);

	Rect Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
	Rect nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
	Vec2 Offset = Vec2(0, 0);
	switch (CMenu::get()->DPI()) {
	case 0:
	{
		Offset = Vec2(0, 0);
		Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
		nigga = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 + Offset.y };
		break;
	}
	case 1:
	{
		Offset = Vec2(1, 1);
		Fullbb = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 25 };
		nigga = { Window->CursorPos.x + 43, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 45 + Offset.y };
		break;
	}
	case 2:
	{
		Offset = Vec2(2, 2);
		Fullbb = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 30 };
		nigga = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 50 + Offset.y };
		break;
	}
	case 3:
	{
		Offset = Vec2(3, 3);
		Fullbb = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 35 };
		nigga = { Window->CursorPos.x + 45, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 55 + Offset.y };
		break;
	}
	case 4:
	{
		Offset = Vec2(4, 4);
		Fullbb = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 + Offset.y, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 40 };
		nigga = { Window->CursorPos.x + 46, Window->CursorPos.y + 11 , float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 60 + Offset.y };
		break;
	}


	}

	AddItemToWindow(Window, nigga, NULL);


	Vec2 DrawPos = Vec2(Fullbb.Min);

	bool canopen = true;

	bool hovered, held;
	held = false;
	hovered = false;
	bool pressed = false;
	static bool init = false;

	Render::Draw->Text(label, DrawPos.x + 1, DrawPos.y - 14, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(83, 83, 83, g.MenuAlpha));
	//Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(34, 34, 34, g.MenuAlpha), D3DCOLOR_RGBA(30, 30, 30, g.MenuAlpha), true);

	SetNextWindowPos(Vec2(Fullbb.Min.x, Fullbb.Min.y + Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(Fullbb.Max.x, (20 + (5 * CMenu::get()->DPI())) * items.size()));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	int Combocurpos = 0;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	canopen = true;

	if (pressed && canopen && g.DontMove == false)
		PickerWindow->Opened = init = !PickerWindow->Opened;

	if (KeyPressed(VK_LBUTTON) && IsInside(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInside(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		g.DontMove = false;
		CMenu::get()->PopUpOpen = false;
		CMenu::get()->Clicked == "";
		CMenu::get()->popupname = "";
	}

	if (PickerWindow->Opened) {

		g.DontMove = true;

		if (KeyPressed(VK_RBUTTON))
			PickerWindow->Opened = false;

		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);
		//	CMenu::get()->Clicked == "Moving";
		//	CMenu::get()->popupname = label;
		//	CMenu::get()->PopUpOpen = true;
			//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 8), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 7), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));



		for (int i = 0; i < items.size(); i++) {
			if (ui::Selectable(items.at(i), *current_item == i, Vec2(float(std::clamp(Window->Size.x - 102 + 6, 63.f, 200.f) + (49 * CMenu::get()->DPI())) - 2, 18 + (5 * CMenu::get()->DPI())), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2 + Combocurpos))) {
				*current_item = i;
				PickerWindow->Opened = false;
				CMenu::get()->PopUpOpen = false;
				CMenu::get()->Clicked == "";
				CMenu::get()->popupname = "";
			}
			Combocurpos += 20 + (5 * CMenu::get()->DPI());
		}



		//Selectable("100%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2));
		//Selectable("125%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 22));
		//Selectable("150%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 42));
		//Selectable("175%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 62));
	}
	else if (hovered) {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	else {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(26, 26, 26, g.MenuAlpha), D3DCOLOR_RGBA(27, 27, 27, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(71, 71, 71, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(71, 71, 71, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(71, 71, 71, g.MenuAlpha));
	}
	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	const char* buf_display = "";

	buf_display = items.at(*current_item);


	Render::Draw->Text(buf_display, DrawPos.x + 10, DrawPos.y + 4 + Offset.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(72, 72, 72, g.MenuAlpha), Vec2(Fullbb.Max.x - 30, 20));
	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

bool ui::SingelSelectNoTextagent(const char* label, int* current_item, std::vector<const char*> items) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Vec2 xer = Window->CursorPos + Vec2(42, 0);

	Rect Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 - 14, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 10 };
	Rect nigga = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 - 14, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 26 };
	Vec2 Offset = Vec2(0, 0);

	AddItemToWindow(Window, nigga, NULL);

	Vec2 DrawPos = Fullbb.Min;

	bool canopen = true;

	bool hovered, held;
	hovered = IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressed(VK_LBUTTON) && hovered;
	static bool init = false;

	//Render::Draw->Text(label, DrawPos.x + 1, DrawPos.y - 14, LEFT, Render::Fonts::Verdana, false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));
	//Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(34, 34, 34, g.MenuAlpha), D3DCOLOR_RGBA(30, 30, 30, g.MenuAlpha), true);

	SetNextWindowPos(Vec2(Fullbb.Min.x, Fullbb.Min.y + Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(Fullbb.Max.x, (10 + (5 * CMenu::get()->DPI())) * items.size()));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	int Combocurpos = 0;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	canopen = true;

	if (pressed && canopen && g.Blocking == false)
		PickerWindow->Opened = init = !PickerWindow->Opened;

	if (KeyPressed(VK_LBUTTON) && IsInside(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInside(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		g.Blocking = false;
		CMenu::get()->PopUpOpen = false;
		CMenu::get()->Clicked == "";
		CMenu::get()->popupname = "";
	}

	if (PickerWindow->Opened) {

		g.Blocking = true;

		if (KeyPressed(VK_RBUTTON))
			PickerWindow->Opened = false;


		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);
		//	CMenu::get()->Clicked == "Moving";
		//	CMenu::get()->popupname = label;
		//	CMenu::get()->PopUpOpen = true;
			//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 8), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 7), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));



		for (int i = 0; i < items.size(); i++) {
			if (ui::Selectable(items.at(i), *current_item == i, Vec2(float(std::clamp(Window->Size.x - 102 + 6, 63.f, 200.f) + (49 * CMenu::get()->DPI())) - 2, 18 + (5 * CMenu::get()->DPI())), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2 + Combocurpos))) {
				*current_item = i;
				PickerWindow->Opened = false;
				CMenu::get()->PopUpOpen = false;
				CMenu::get()->Clicked == "";
				CMenu::get()->popupname = "";
			}
			Combocurpos += 20 + (5 * CMenu::get()->DPI());
		}

		//Selectable("100%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2));
		//Selectable("125%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 22));
		//Selectable("150%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 42));
		//Selectable("175%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 62));
	}
	else if (hovered) {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	else {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(31, 31, 31, g.MenuAlpha), D3DCOLOR_RGBA(36, 36, 36, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	const char* buf_display = "";

	buf_display = items.at(*current_item);

	Render::Draw->Text(buf_display, DrawPos.x + 5, DrawPos.y + 4 + Offset.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(142, 142, 142, g.MenuAlpha), Vec2(Fullbb.Max.x - 30, 20));

	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

bool ui::SingelSelectNoText(const char* label, int* current_item, std::vector<const char*> items) {
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Vec2 xer = Window->CursorPos + Vec2(42, 0);

	Rect Fullbb = { Window->CursorPos.x + 42, Window->CursorPos.y + 11 - 14, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 20 };
	Rect nigga = { Window->CursorPos.x + 44, Window->CursorPos.y + 11 - 14, float(std::clamp(Window->Size.x - 102 + 8, 63.f, 200.f) + (49 * CMenu::get()->DPI())) , 26 };
	Vec2 Offset = Vec2(0, 0);

	AddItemToWindow(Window, nigga, NULL);

	Vec2 DrawPos = Fullbb.Min;

	bool canopen = true;

	bool hovered, held;
	hovered = IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressed(VK_LBUTTON) && hovered;
	static bool init = false;

	//Render::Draw->Text(label, DrawPos.x + 1, DrawPos.y - 14, LEFT, Render::Fonts::Verdana, false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));
	//Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(34, 34, 34, g.MenuAlpha), D3DCOLOR_RGBA(30, 30, 30, g.MenuAlpha), true);

	SetNextWindowPos(Vec2(Fullbb.Min.x, Fullbb.Min.y + Fullbb.Max.y + 1));
	SetNextWindowSize(Vec2(Fullbb.Max.x, 160));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;

	int Combocurpos = 0;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	canopen = true;

	if (pressed && canopen && g.Blocking == false)
		PickerWindow->Opened = init = !PickerWindow->Opened;

	if (KeyPressed(VK_LBUTTON) && IsInside(Window->ParentWindow->Pos.x, Window->ParentWindow->Pos.y, Window->ParentWindow->Size.x, Window->ParentWindow->Size.y)
		&& !IsInside(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y)
		&& !IsInside(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y)) {
		PickerWindow->Opened = false;
		g.Blocking = false;
		CMenu::get()->PopUpOpen = false;
		CMenu::get()->Clicked == "";
		CMenu::get()->popupname = "";
		
	}

	if (PickerWindow->Opened) {

		g.Blocking = true;

		if (KeyPressed(VK_RBUTTON))
			PickerWindow->Opened = false;


		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);
		//	CMenu::get()->Clicked == "Moving";
		//	CMenu::get()->popupname = label;
		//	CMenu::get()->PopUpOpen = true;
			//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 8), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 7), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));

		PickerWindow->CursorPos.x += 1;
		PickerWindow->CursorPos.y -= 22;

		for (int i = 0; i < items.size(); i++) {
			if (ui::Selectable(items.at(i), *current_item == i, Vec2(float(std::clamp(Window->Size.x - 102 + 6, 63.f, 200.f) + (49 * CMenu::get()->DPI())) - 2, 18 + (5 * CMenu::get()->DPI())), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2 + Combocurpos))) {
				*current_item = i;
				PickerWindow->Opened = false;
				CMenu::get()->PopUpOpen = false;
				CMenu::get()->Clicked == "";
				
				CMenu::get()->popupname = "";
				ui::EndSelectNoText();

			}
			Combocurpos += 20 + (5 * CMenu::get()->DPI());
		}

		//Selectable("100%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 2));
		//Selectable("125%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 22));
		//Selectable("150%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 42));
		//Selectable("175%", data, Vec2(161, 18), Vec2(Fullbb.Min.x + 1, Fullbb.Min.y + Fullbb.Max.y + 62));
	}
	else if (hovered) {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(41, 41, 41, g.MenuAlpha), D3DCOLOR_RGBA(45, 45, 45, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	else {
		Render::Draw->Gradient(Fullbb.Min, Fullbb.Max, D3DCOLOR_RGBA(31, 31, 31, g.MenuAlpha), D3DCOLOR_RGBA(36, 36, 36, g.MenuAlpha), true);

		//Arrow shit
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 7), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 11, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 10, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 9, Fullbb.Min.y + 10), Vec2(1, 1), D3DCOLOR_RGBA(151, 151, 151, g.MenuAlpha));
	}
	Render::Draw->Rect(Fullbb.Min, Fullbb.Max, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

	const char* buf_display = "";

	buf_display = items.at(*current_item);

	Render::Draw->Text(buf_display, DrawPos.x + 10, DrawPos.y + 4 + Offset.y, LEFT, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(142, 142, 142, g.MenuAlpha), Vec2(Fullbb.Max.x - 30, 20));

	//End();
	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}

bool ui::SingleSelectIcon(const char* label, int* current_item, std::unordered_map<int, bool>* data, std::vector<const char*> items, LPD3DXFONT Font)
{
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	GuiFlags flags = Flags_PopUp;

	Rect Fullbb = { Window->CursorPos.x + 20, Window->CursorPos.y , max(Window->Size.x - 28, 63.f) , 20 };
	Vec2 Offset = Vec2(0, 0);

	//AddItemToWindow(Window, Fullbb, NULL);

	bool hovered, held;
	hovered = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, Fullbb.Max.x, Fullbb.Max.y);
	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered;
	static bool init = false;

	SetNextWindowPos(Window->Pos + Vec2(0, Window->Size.y));
	SetNextWindowSize(Vec2(Window->Size.x, (items.size() * 30) + 2));

	Begin(label, flags);
	GuiWindow* PickerWindow = GetCurrentWindow();
	PickerWindow->ParentWindow = Window;
	PickerWindow->CursorPos = PickerWindow->Pos;

	if (!PickerWindow->Init) {
		Window->PopUpWindows.push_back(PickerWindow);
		PickerWindow->Init = true;
	}

	bool skip_frame = false;

	if (pressed) {
		PickerWindow->Opened = !PickerWindow->Opened;
		skip_frame = true;
	}

	std::string icons[] = { "", "Y", "a", "Z", "J", "A", "G", "h" };

	const char* buf_display = items.at(*current_item);
	Vec2 display_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), buf_display);
	Vec2 size = Vec2(8, 8);

	//::Draw->Gradient(DrawPos + Vec2(1, 1), size - Vec2(2, 2), D3DCOLOR_RGBA(77, 77, 77, g.MenuAlpha), D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), true);

	Render::Draw->FilledRect(Fullbb.Min + Vec2(1, 1 + 6), size - Vec2(2, 2), CMenu::get()->MenuColor());
	Render::Draw->Rect(Fullbb.Min + Vec2(0, 6), size, 1, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));
	Render::Draw->Text(buf_display, Fullbb.Min.x + 20, Fullbb.Min.y + Fullbb.Max.y / 2.f - display_size.y / 2.f, LEFT, CMenu::get()->GetFontBold(), false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));

	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	if (PickerWindow->Opened) {//arrow fix fix arrow
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 5, Fullbb.Min.y + 11), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 5, Fullbb.Min.y + 10), Vec2(5, 1), D3DCOLOR_RGBA(148, 148, 148, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 4, Fullbb.Min.y + 9), Vec2(3, 1), D3DCOLOR_RGBA(148, 148, 148, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 3, Fullbb.Min.y + 8), Vec2(1, 1), D3DCOLOR_RGBA(148, 148, 148, g.MenuAlpha));
	}
	else
	{
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 5, Fullbb.Min.y + 8), Vec2(5, 1), D3DCOLOR_RGBA(0, 0, 0, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 5, Fullbb.Min.y + 9), Vec2(5, 1), D3DCOLOR_RGBA(148, 148, 148, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 4, Fullbb.Min.y + 10), Vec2(3, 1), D3DCOLOR_RGBA(148, 148, 148, g.MenuAlpha));
		Render::Draw->FilledRect(Vec2(Fullbb.Min.x + Fullbb.Max.x - 3, Fullbb.Min.y + 11), Vec2(1, 1), D3DCOLOR_RGBA(148, 148, 148, g.MenuAlpha));
	}

	if (PickerWindow->Opened) {

		CMenu::get()->PopUpOpen = true;

		if (KeyPressed(VK_LBUTTON) && IsInsideIgnore(PickerWindow->Pos.x, PickerWindow->Pos.y, PickerWindow->Size.x, PickerWindow->Size.y) && !skip_frame)
		{
			PickerWindow->Opened = false;
		}

		for (int i = 0; i < items.size(); i++) {
			if (ui::SelectableIcon(items.at(i), &(*data)[i], Vec2(0, 30), icons[i])) {
				*current_item = i;
				PickerWindow->Opened = false;
				CMenu::get()->PopUpOpen = false;
				CMenu::get()->Clicked == "";
				CMenu::get()->popupname = "";
			}
		}
	}





	Render::Draw->GetD3dDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	std::string selected_icon = icons[*current_item];

	if (strcmp(selected_icon.c_str(), "") == 0)
	{
		Vec2 icon_size = { 49, 37 };
		Render::Draw->Sprite(Render::Draw->RageAdaptiveGlobalIcon, Vec2(Fullbb.Min.x + Fullbb.Max.x - 10 - icon_size.x, Fullbb.Min.y - 10), icon_size, D3DCOLOR_RGBA(255, 255, 255, g.MenuAlpha));
	}
	else
	{
		Vec2 icon_size = Render::Draw->GetTextSize(Font, selected_icon.c_str());
		Render::Draw->Text(selected_icon.c_str(), Fullbb.Min.x + Fullbb.Max.x - 20 - icon_size.x, Fullbb.Min.y, LEFT, Font, false, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha));
	}

	SetCurrentWindow(PickerWindow->ParentWindow);

	return true;
}


bool ui::SelectableIcon(const char* label, bool* selected, Vec2 size_arg, std::string icon)
{
	GuiContext& g = *Gui_Ctx;
	GuiWindow* Window = GetCurrentWindow();

	if (size_arg.x < 1)
		size_arg.x = Window->Size.x - 2;

	Rect Fullbb = Rect{ Window->CursorPos, size_arg };

	AddItemToWindow(Window, Fullbb, NULL);

	bool hovered = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, size_arg.x, size_arg.y);

	bool hovered2 = IsInsideIgnore(Fullbb.Min.x + 50, Fullbb.Min.y, size_arg.x - 50, size_arg.y);
	bool pressed = KeyPressedIgnore(VK_LBUTTON) && hovered2;

	bool hovered3 = IsInsideIgnore(Fullbb.Min.x, Fullbb.Min.y, 49, size_arg.y);
	bool pressed2 = KeyPressedIgnore(VK_LBUTTON) && hovered3;

	if (pressed2 && !(strcmp(label, "Global") == 0))
		*selected = !*selected;

	Vec2 size = Vec2(8, 8);

	if (hovered)
		DrawList::AddFilledRect(Fullbb.Min + Vec2(2, 0), size_arg - Vec2(2, 0), D3DCOLOR_RGBA(25, 25, 25, g.MenuAlpha));

	D3DCOLOR IconColor = hovered || *selected ? D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha) : D3DCOLOR_RGBA(104, 104, 104, g.MenuAlpha);

	if (*selected)
	{
		DrawList::AddFilledRect(Fullbb.Min + Vec2(21, 1 + 12), size - Vec2(2, 2), CMenu::get()->MenuColor());
		DrawList::AddRect(Fullbb.Min + Vec2(20, 12), size, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

		Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), label);
		DrawList::AddText(label, Fullbb.Min.x + 40, Fullbb.Min.y + Fullbb.Max.y / 2.f - label_size.y / 2.f, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha), CMenu::get()->GetFontBold());

		if (strcmp(icon.c_str(), "") == 0)
		{
			Vec2 icon_size = { 49, 37 };
			DrawList::AddSprite(Render::Draw->RageAdaptiveGlobalIcon, Vec2(Fullbb.Min.x + Fullbb.Max.x - 20 - icon_size.x, Fullbb.Min.y - 5), icon_size, D3DCOLOR_RGBA(255, 255, 255, g.MenuAlpha));
		}
		else
		{
			Vec2 icon_size = Render::Draw->GetTextSize(Render::Fonts::LBIcons3, icon.c_str());
			DrawList::AddText(icon.c_str(), Fullbb.Min.x + Fullbb.Max.x - 20 - icon_size.x, Fullbb.Min.y + 6, IconColor, Render::Fonts::LBIcons3, false, Vec2(Window->Size.x - 2, size_arg.y));
		}
	}
	else
	{
		DrawList::AddGradient(Fullbb.Min + Vec2(21, 1 + 12), size - Vec2(2, 2), D3DCOLOR_RGBA(77, 77, 77, g.MenuAlpha), D3DCOLOR_RGBA(52, 52, 52, g.MenuAlpha), true);
		DrawList::AddRect(Fullbb.Min + Vec2(20, 12), size, D3DCOLOR_RGBA(12, 12, 12, g.MenuAlpha));

		Vec2 label_size = Render::Draw->GetTextSize(CMenu::get()->GetFontBold(), label);
		DrawList::AddText(label, Fullbb.Min.x + 40, Fullbb.Min.y + Fullbb.Max.y / 2.f - label_size.y / 2.f, D3DCOLOR_RGBA(192, 192, 192, g.MenuAlpha), hovered ? CMenu::get()->GetFontBold() : CMenu::get()->GetFontDPI());

		if (strcmp(icon.c_str(), "") == 0)
		{
			Vec2 icon_size = { 49, 37 };
			DrawList::AddSprite(Render::Draw->RageAdaptiveGlobalIcon, Vec2(Fullbb.Min.x + Fullbb.Max.x - 20 - icon_size.x, Fullbb.Min.y - 5), icon_size, D3DCOLOR_RGBA(150, 150, 150, g.MenuAlpha));
		}
		else
		{
			Vec2 icon_size = Render::Draw->GetTextSize(Render::Fonts::LBIcons3, icon.c_str());
			DrawList::AddText(icon.c_str(), Fullbb.Min.x + Fullbb.Max.x - 20 - icon_size.x, Fullbb.Min.y + 6, IconColor, Render::Fonts::LBIcons3, false, Vec2(Window->Size.x - 2, size_arg.y));
		}
	}

	return pressed;
}