#pragma once

#include "..\includes.hpp"
#include "singleton.h"

enum FontCenteringFlags
{
	HFONT_CENTERED_NONE = (1 << 0),
	HFONT_CENTERED_X = (1 << 1),
	HFONT_CENTERED_Y = (1 << 2)
};

enum GradientType
{
	GRADIENT_HORIZONTAL,
	GRADIENT_VERTICAL
};

enum FONT_INDEX
{
	LOGS,
	ESP,
	NAME,
	HE,
	SUBTABWEAPONS,
	KNIFES,
	GRENADES,
	HE2,
	FONT_MAX
};

extern vgui::HFont fonts[FONT_MAX];

template<typename T>
class singleton;

struct CUSTOMVERTEX2 {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};

class render : public singleton< render > {
private:
	float alpha_factor = 1.0f;
	LPDIRECT3DDEVICE9 device; //-V122
	D3DVIEWPORT9      m_viewport;
public:
	bool initalized = false;

	float get_alphafactor();
	void set_alphafactor(float factor);
	void rect(int x, int y, int w, int h, Color color);
	void rect_filled(int x, int y, int w, int h, Color color);
	void filled_rect_world(Vector center, Vector2D size, Color color, int angle);
	void setup_states() const;
	void invalidate_objects();
	void restore_objects(LPDIRECT3DDEVICE9 m_device);
	void gradient(int x, int y, int w, int h, Color first, Color second, float type);
	void rounded_box(int x, int y, int w, int h, int points, int radius, Color color);
	void grid(int x, int y, int w, int h, Color first, Color second, Color third, GradientType type);
	void circle(int x, int y, int points, int radius, Color color);
	void circle_filled(int x, int y, int points, int radius, Color color);
	void triangle(Vector2D point_one, Vector2D point_two, Vector2D point_three, Color color);
	void triangle_def(Vector2D fir, Vector2D sec, Vector2D thrd, Color color);
	void line(int x, int y, int x2, int y2, Color color);
	void text(vgui::HFont font, int x, int y, Color color, DWORD flags, const char* msg, ...);
	void wtext(vgui::HFont font, int x, int y, Color color, DWORD flags, wchar_t* msg);
	int text_height(vgui::HFont font, const char* msg, ...);
	int text_width(vgui::HFont font, const char* msg, ...);
	void DrawFilled3DBox(const Vector& origin, int width, int height, Color outline, Color filling);
	void Draw3DCircle(const Vector& origin, float radius, Color color, int line_thinkness);
	void Draw3DFilledCircle(const Vector& origin, float radius, Color color);
	void Draw3DFilledCircleTest(const Vector& origin, float radius, Color color);
	void Draw3DCircleGradient(const Vector& origin, float radius, Color color, float alpha_multiplier);
	void Draw3DRainbowCircle(const Vector& origin, float radius, int line_thinkness);
	RECT viewport();
	void arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);
	void DrawArc(const Vector2D& center, float radius, float progress, const Color& color, float thickness, int num_segments);
	void arc_poly(const Vector& center, float radius, float a_min, float a_max, float num_segments, float thiccboi, Color color);
	void CircularProgressBar(int x, int y, int radius, int start_angle, int percent, int thickness, Color color);
	void CircularProgressBar_test(int x, int y, int radius, int start_angle, int percent, int points, int thickness, Color color);
	void draw_arc2(int x, int y, int radius, int startangle, int percent, int thickness, Color color, int line_thinkness);
	void progress_circle( const Vector2D& center, int radius, const Color& col, float progress );
	void dual_circle(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device);
};