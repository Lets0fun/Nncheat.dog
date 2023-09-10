#pragma once
#include "..\includes.hpp"

namespace render2 
{
	enum e_textflags 
	{
		none = 0,
		outline = 1 << 0,
		centered_x = 1 << 1,
		centered_y = 1 << 2,
	};

	enum e_cornerflags
	{
		default = 15,
		corner_left_top = 1 << 0,
		corner_right_top = 1 << 1,
		corner_left_bot = 1 << 2,
		corner_right_bot = 1 << 3,
	};
}


class ImGuiRendering
{
public:

	void BeginScene(IDirect3DDevice9* pDevice);
	void EndScene();

	void arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);
	void two_sided_arc(float x, float y, float radius, float scale, Color col, float thickness);
	void PolyLine(const ImVec2* points, const int points_count, Color clr, bool closed, float thickness = 1.f, Color fill = Color(0, 0, 0, 0));
	void DrawRing3D1(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float progress = 1.f, bool fill_prog = false);
	void DrawCollisionRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float prog);
	void DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, Color clr, float width);
	void DrawLine(float x1, float y1, float x2, float y2, Color clr, float thickness = 1.f);
	void DrawImage(float x1, float y1, float x2, float y2, Color clr, float thickness);
	void filled_rect_world(Vector center, Vector2D size, Color color, int angle);
	void DrawLineGradient(float x1, float y1, float x2, float y2, Color clr1, Color cl2, float thickness = 1.f);
	void Rect(float x, float y, float w, float h, Color clr, float rounding = 0.f);
	void FilledRect(float x, float y, float w, float h, Color clr, float rounding = 0.f);
	void filled_rect_gradient(float x, float y, float w, float h, Color col_upr_left, Color col_upr_right, Color col_bot_right, Color col_bot_left);
	void FilledRect2(float x, float y, float w, float h, Color clr, float rounding = 0.f);
	void Triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color clr, float thickness = 1.f);
	void TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, Color clr);
	void QuadFilled(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Color clr);

	void CircleFilled(float x1, float y1, float radius, Color col, int segments);
	void RadialGradient3D(Vector pos, float radius, Color in, Color out, bool one);
	void DrawWave(Vector loc, float radius, Color clr, float thickness = 1.f);
	void DrawString(float x, float y, Color color, int flags, ImFont* font, const char* message, ...);
	void DrawCircle(float x, float y, float r, float s, Color color);
	void build_lookup_table();
	void circle_filled_radial_gradient(IDirect3DDevice9* dev, Vector2D center, float radius, Color color1, Color color2);
	void DrawGradientCircle(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2);
	void DrawString2(float x, float y, Color color, int flags, ImFont* font, const std::string message, ...);
	void DrawRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float progress = 1.f, bool fill_prog = false);

	static constexpr auto points = 64;
	std::vector<Vector2D> lookup_table;
};

inline ImGuiRendering* g_Render;