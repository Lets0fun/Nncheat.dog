#pragma once
#include "../Globalincludes.h"
#include "Framework.h"
#include <memory>

#define get_a(col) (((col)&0xff000000)>>24)
#define get_r(col) (((col)&0x00ff0000)>>16)
#define get_g(col) (((col)&0x0000ff00)>>8)
#define get_b(col) ((col)&0x000000ff)

enum circle_type { FULL, HALF, QUARTER };
enum text_alignment { LEFT, CENTER, RIGHT };

struct vertex
{
    FLOAT x, y, z, rhw;
    DWORD color;
};

struct vertex2
{
    FLOAT x, y, z, rhw;
    DWORD color;
    FLOAT u, v;
};

namespace Render
{
    using IdaLovesMe::Vec2;

    namespace Fonts
    {
       
        extern LPD3DXFONT LBIcons;
        extern LPD3DXFONT LBIcons2;
        extern LPD3DXFONT LBIcons3;
        extern LPD3DXFONT TabIcons;
        extern LPD3DXFONT TabIcons2;
        extern LPD3DXFONT TabIcons3;
        extern LPD3DXFONT TabLua125;
        extern LPD3DXFONT TabLua150;
        extern LPD3DXFONT TabIcons4;

        extern LPD3DXFONT Verdana;
        extern LPD3DXFONT Undefeated;
        extern LPD3DXFONT Tahombd;
        extern LPD3DXFONT Keybind;
        extern LPD3DXFONT Logs;
        extern LPD3DXFONT SmallFont;
        extern LPD3DXFONT Indicator;
        extern LPD3DXFONT SmallFont25;
        extern LPD3DXFONT SmallFont50;
        extern LPD3DXFONT SmallFont75;
        extern LPD3DXFONT SmallFont200;
        extern LPD3DXFONT IndicatorShadow;
        extern LPD3DXFONT Verdana25;
        extern LPD3DXFONT Verdana50;
        extern LPD3DXFONT Verdana75;
        extern LPD3DXFONT Verdana200;
        extern LPD3DXFONT Tahombd25;
        extern LPD3DXFONT Tahombd50;
        extern LPD3DXFONT Tahombd75;
        extern LPD3DXFONT Tahombd200;
        extern LPD3DXFONT Keybind75;
        extern LPD3DXFONT Keybind75;
        extern LPD3DXFONT Keybind75;
        extern LPD3DXFONT Keybind200;
    }

    class CDraw
    {
    public:
        struct sScreen
        {
            float Width;
            float Height;
            float x_center;
            float y_center;
        } Screen;

        //=============================================================================================
        void Line(Vec2 Pos, Vec2 Pos2, D3DCOLOR Color);
        void Rect(Vec2 Pos, Vec2 Size, float linewidth, D3DCOLOR Color, bool Antialias = false);
        void FilledRect(Vec2 Pos, Vec2 Size, D3DCOLOR color, bool Antialias = false);
        void Gradient(Vec2 Pos, Vec2 Size, D3DCOLOR LColor, D3DCOLOR ROtherColor, bool Vertical = false, bool Antialias = false);
        void Triangle(Vec2 Top, Vec2 Left, Vec2 Right, D3DCOLOR Color, bool antialias = false);
        void Text(const char* Text, float X, float Y, int Orientation, LPD3DXFONT Font, bool Bordered, D3DCOLOR Color, Vec2 MaxSize = Vec2(0, 0), Vec2 Clip_pos = Vec2(0, 0));
        void Text_test(const char* Text, float X, float Y, int Orientation, LPD3DXFONT Font, bool Bordered, D3DCOLOR Color, Vec2 MaxSize = Vec2(0, 0));
        typedef void Sprited(LPDIRECT3DTEXTURE9 Texture, Vec2 Pos, Vec2 Size, D3DCOLOR Color);
        void Sprite(LPDIRECT3DTEXTURE9 Texture, Vec2 Pos, Vec2 Size, D3DCOLOR Color);
        //=============================================================================================


        //=============================================================================================
        void Init(LPDIRECT3DDEVICE9 D3dDevice);
        void CreateObjects();
        void ReleaseObjects();
        void Reset();
        void GradientColor(Vec2 Pos, Vec2 Size, D3DCOLOR color);
        //=============================================================================================
        IDirect3DTexture9*  GetBgTexture();
        Vec2                GetTextSize(ID3DXFont* Font, const char* Text);
        LPDIRECT3DDEVICE9   GetD3dDevice();
        //=============================================================================================

        IDirect3DTexture9*      RageAdaptiveGlobalIcon;
        
    private:
        LPDIRECT3DDEVICE9       m_Device;
        

        IDirect3DTexture9*      m_BgTexture;
        HANDLE                  m_TabFont;
        HANDLE                  m_TabFont2;
        HANDLE                  m_TabFont3;
        HANDLE                  m_TabFont4;
        HANDLE                  m_LBFont;

        bool Initialized = false;
    };

    extern std::unique_ptr<CDraw> Draw;
}