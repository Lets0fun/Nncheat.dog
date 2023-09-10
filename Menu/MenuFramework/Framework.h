#pragma once
#include "../Globalincludes.h"

#include <vector>
#include "color.h"
#include <unordered_map>
#include "../Configuration/Config.h"
typedef int GuiFlags;
typedef int DrawListType;



namespace IdaLovesMe {

	enum GuiFlags_ {
		Flags_None = 0,
		Flags_NoMove = 1 << 1,
		Flags_NoResize = 1 << 2,
		Flags_ChildWindow = 1 << 3,
		Flags_PopUp = 1 << 4,
		Flags_ColorPicker = 1 << 5,
		Flags_ReturnKeyReleased = 1 << 6,
		Flags_ComboBox = 1 << 7,
		Flags_Listbox = 1 << 8,
		GuiFlags_IntSlider = 1 << 9,
		GuiFlags_LegitTab = 1 << 16,
		GuiFlags_FloatSlider = 1 << 10
	};

	enum DrawListType_ {
		DrawType_Text = 0,
		DrawType_FilledRect = 1,
		DrawType_Rect = 2,
		DrawType_Gradient = 3,
		DrawType_Triangle = 4,
		DrawType_Sprite = 5
	};

	struct Vec2 {
		float x, y;

		Vec2() { x = y = 0; }
		Vec2(float _x, float _y) { x = _x; y = _y; }

		Vec2& operator+=(const Vec2& other)
		{
			x += other.x;
			y += other.x;
			return *this;
		}
		Vec2& operator-=(const Vec2& other)
		{
			x -= other.x;
			y -= other.x;
			return *this;
		}
		Vec2 operator+(const Vec2& other)
		{
			return Vec2(x + other.x, y + other.y);
		}

		Vec2 operator-(const Vec2& other)
		{
			return Vec2(x - other.x, y - other.y);
		}

		bool operator==(const Vec2& other) const {
			return x == other.x && y == other.y;
		}
	};

	struct HSV {
		float h;
		float s;
		float v;
		float a;

		HSV() {
			h = 0;
			s = 0;
			v = 0;
			a = 255;
		}

		HSV(float h_, float s_, float v_, float a_) {
			h = h_;
			s = s_;
			v = v_;
			a = a_;
		}
	};

	struct Rect {
		Vec2 Min;
		Vec2 Max;

		Rect() : Min(FLT_MAX, FLT_MAX), Max(-FLT_MAX, -FLT_MAX) {}
		Rect(const Vec2& min, const Vec2& max) : Min(min), Max(max) {}
		Rect(float x1, float y1, float x2, float y2) : Min(x1, y1), Max(x2, y2) {}
	};

	struct GuiWindow {
		std::string				 Name;
		GuiFlags				 Flags;
		Vec2					 Pos, Size;
		Rect                     Defualts;
		GuiWindow* ParentWindow;
		std::vector<GuiWindow*>  ChildWindows;

		bool					 Opened;
		bool					 Dragging;
		bool					 Resizing;
		bool					 Block;
		bool					 Init;
		bool                     Disabled;

		int						 xPos;
		int						 yPos;
		int					     xSize;
		int						 ySize;
		Vec2				     CursorPos;
		Vec2                     PrevCursorPos;
		Vec2                     PrevWidgetPos;
		std::unordered_map<std::string, bool> ActiveID;
		std::unordered_map<std::string, bool> ItemActive;
		std::unordered_map<std::string, std::string> SelectedId;
		std::unordered_map<std::string, std::string> Preview;
		std::unordered_map<std::string, float> SliderValue;
		std::unordered_map<std::string, int> SliderValueInt;
		std::vector<GuiWindow*>  PopUpWindows;
		float                    ScrollRatio;
		std::string SelectedItem;
	};

	struct NextWindowInfo {
		Vec2					 Pos;
		Vec2					 Size;
		bool					 PosCond;
		bool					 SizeCond;
	};

	struct GuiContext {
		bool					 Initialized;
		std::vector<GuiWindow*>  Windows;
		std::vector<std::string> WindowsByName;
		GuiWindow* CurrentWindow;
		NextWindowInfo			 NextWindowInfo;
		int						 MenuAlpha;

		bool					 KeyState[256];
		bool                     IsClicking = false;
		bool                     DontMove;
		const char* CurrentItem;
		bool                     Blocking = false;
		bool					 PrevKeyState[256];
		Vec2					 MousePos;
		Vec2					 PrevMousePos;
		float                    niggatest;
		int                      niggatestint;
		float                    MouseWheel = 0.f;
		bool                     AwaitingInput;
		int                      LastInput;
	};



	namespace ui {
		void GetInputFromWindow(const std::string& window_name);
		bool IsInside(float x, float y, float w, float h);
		bool IsInsideIgnore(const float x, const float y, const float w, const float h);
		bool KeyPressed(const int key);
		bool KeyPressedIgnore(const int key);
		bool KeyDown(const int key);
		bool KeyDownMenu(const int key);
		bool KeyReleased(const int key);

		void HandleMoving(GuiWindow* Window, Rect Constraints = Rect{}, Vec2* v = nullptr);
		void HandleResize(GuiWindow* Window, Rect Constraints = Rect{}, Vec2* buffer = nullptr);
		bool ChildsAreStable(GuiWindow* Window);

		void					 Shutdown(GuiContext* context);
		void					 Init(GuiContext* context);
		GuiContext* CreateContext();
		void					 DeleteContext(GuiContext* ctx);

		GuiWindow* FindWindowByName(const char*& name);
		static GuiWindow* CreateNewWindow(const char*& name, Vec2 size, GuiFlags flags);
		void					 SetNextWindowPos(const Vec2& pos);
		void					 SetNextWindowSize(const Vec2& pos);
		Vec2		   			 GetWindowSize();
		Vec2					 GetWindowPos();
		GuiWindow* GetCurrentWindow();
		void					 SetCurrentWindow(GuiWindow* window);
		bool					 IsInsideWindow(GuiWindow* Window = 0);

		void					 Begin(const char* Id, GuiFlags flags);
		void					 End();

		void BeginChildRagewep(const char* Name, Rect X, GuiFlags flags);

		void					 BeginChild(const char* id, Rect X = { Vec2(0, 0), Vec2(3,10) }, GuiFlags flags = NULL);
		void					 EndChild();

		bool listboxskins(const char* label, bool activated, const Vec2& size_arg);

		void ListBox(const char* Name, Vec2 X, bool Text, GuiFlags flags, D3DCOLOR col = D3DCOLOR_RGBA(35, 35, 35, 255));

		void EndListBox();

		void EndSelectNoText();

		void InputText(const char* id, char* buffer, bool Listbox, GuiFlags flags);

		//Elements


		bool TabButton(const char* label, int* selected, int num, int total);
		bool TabButtonLua125(const char* label, int* selected, int num, int total);
		bool TabButtonLua150(const char* label, int* selected, int num, int total);
		//bool LegitButton(const char* label, int* selected, int num, int total);
		//bool Selectable(const char* label, bool activated, const Vec2& size_arg);
		bool ListBoxSelectable(const char* label, bool activated, bool loaded,  const Vec2& size_arg);
		bool RageButton(const char* label, int* selected, int num, int total, GuiFlags flags);
		bool ListBoxSelectableLua(const char* label, bool selected, bool loaded, const Vec2& size_arg);
		bool ListBoxSelectableElement(const char* label, bool selected, bool loaded, const Vec2& size_arg);
		bool ListBoxSelectableConfig(const char* label, bool activated, const Vec2& size_arg);
		//bool ListBoxSelectableCheck(const char* label, bool* p_selected, Vec2 size_arg);
		bool Button(const char* label);
		bool ButtonDisabled(const char* label);
		bool Checkbox(const char* label, bool* selected);
		bool Checkboxinvisible(const char* label, bool* selected);
		bool CheckboxDisabled(const char* label, bool* selected);
		bool CheckboxColor(const char* label);
		bool CheckboxUntrusted(const char* label, bool* selected);
		bool Label(const char* label);
		bool LabelUntrusted(const char* label);
		template<typename T>
		bool SliderBehaviorIDA(const char* item_id, Rect bb, T value, T min_value, T max_value, GuiFlags flags);
		template<typename T>
		void Slider(const char* label, T* v, T v_min, T v_max, const char* format = NULL, GuiFlags flags = NULL, float scale = 1.f, int remove = 0.f);
		template<typename T>
		void Slider2(const char* label, T* v, T v_min, T v_max, const char* format = NULL, GuiFlags flags = NULL, float scale = 1.f, int remove = 0.f);
		template<typename T>
		void SliderDMG(const char* label, T* v, T v_min, T v_max, const char* format = NULL, GuiFlags flags = NULL, float scale = 1.f, int remove = 0.f);
		template<typename T>
		void SliderNoText(const char* label, T* v, T v_min, T v_max, const char* format, GuiFlags flags, float scale, int remove);
		void SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = NULL, int remove = 0);
		void SliderIntDMG(const char* label, int* v, int v_min, int v_max, const char* format = NULL, int remove = 0);
		void SliderIntNoText(const char* label, int* v, int v_min, int v_max, const char* format, int remove);
		bool ButtonBehavior(GuiWindow* window, const char* label, Rect v, bool& hovered, bool& held, GuiFlags flags);
		void SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float scale);
		bool SetNextOffset(int size);
		bool LegitButton(const char* label, int* selected, int num, int total, GuiFlags flags);
		void AddItemToWindow(GuiWindow* Window, Rect size, GuiFlags flags);
		bool Selectable(const char* label, bool selected, Vec2 size_arg, Vec2 pos);
		bool PopupColor(const char* id, int* current_item, std::vector<const char*> items, Vec2 size, Vec2 pos);
		bool Popup(const char* id, int* current_item, std::vector<const char*> items, Vec2 size, Vec2 pos);
		bool PopupKeybind(const char* id, int* current_item, std::vector<const char*> items, Vec2 size, Vec2 pos);
		bool Keybind(const char* label, int* current_key, int* keystyle);
		HSV ColorPickerBehavior(GuiWindow* PickerWindow, Rect& RcColor, Rect& RcAlpha, Rect& RcHue, int col[4], bool reset);
		bool ColorPicker(const char* label, int col[4], GuiFlags flags);
		bool ColorPickerC(const char* label, CColor* col, GuiFlags flags);
		bool SelectableCheck(const char* label, bool* p_selected, Vec2 size_arg, Vec2 pos_arg);
		bool MultiSelect(const char* label, std::vector<int>& data, std::vector<const char*> items);
		bool PopUpsAreClosed(GuiWindow* Window);
		bool BeginCombo(const char* label, const char* preview_value, int items, GuiFlags flags);
		void EndCombo();
		bool ComboSelectable(const char* label, bool selected, GuiFlags flags, const Vec2& size_arg);
		bool MultiSelectOld(const char* label, std::unordered_map<int, bool>* data, std::vector<const char*> items);
		bool SingelSelect(const char* label, int* current_item, std::vector<const char*> items);

		bool SingelSelectDisabled(const char* label, int* current_item, std::vector<const char*> items);

		bool SingelSelectNoTextagent(const char* label, int* current_item, std::vector<const char*> items);

		bool SingelSelectNoText(const char* label, int* current_item, std::vector<const char*> items);

		bool SingleSelectIcon(const char* label, int* current_item, std::unordered_map<int, bool>* data, std::vector<const char*> items, LPD3DXFONT Font);

		bool SelectableIcon(const char* label, bool* selected, Vec2 size_arg, std::string icon);

		bool SingleSelectIcon(const char* label, int* current_item, std::vector<const char*> items, LPD3DXFONT Font);
		bool SelectableIcon(const char* label, bool selected, Vec2 size_arg, std::string icon);
	}

	class DrawList {
	public:
		struct RenderObject {
			DrawListType Type;
			Vec2         Pos;
			Vec2         Size;
			D3DCOLOR     Color;
			D3DCOLOR     OtherColor;
			std::string  Text;
			LPD3DXFONT     Font;
			bool         Bordered;
			bool         Vertical;
			Vec2         TextClipSize;
			bool         Antialias;
			LPDIRECT3DTEXTURE9 Texture;

			RenderObject(DrawListType Type, Vec2 Pos, Vec2 Size, D3DCOLOR color, D3DCOLOR OtherColor, const char* text, LPD3DXFONT font, bool Bordered, bool Vertical, Vec2 TextClipSize, bool Antialias, LPDIRECT3DTEXTURE9 Texture) {
				this->Type = Type;
				this->Pos = Pos;
				this->Size = Size;
				this->Color = color;
				this->OtherColor = OtherColor;
				this->Text = text ? std::string(text) : std::string();
				this->Font = font;
				this->Bordered = Bordered;
				this->Vertical = Vertical;
				this->TextClipSize = TextClipSize;
				this->Antialias = Antialias;
				this->Texture = Texture;
			}
		};
		static std::vector<RenderObject> Drawlist;
		static void AddText(const char* text, int x, int y, D3DCOLOR Color, LPD3DXFONT font, bool Bordered = false, Vec2 TextClipSize = Vec2(0, 0));
		static void AddSprite(LPDIRECT3DTEXTURE9 Texture, Vec2 Pos, Vec2 Size, D3DCOLOR Color);
		static void AddFilledRect(Vec2 Pos, Vec2 Size, D3DCOLOR Color);
		static void AddRect(Vec2 Pos, Vec2 Size, D3DCOLOR Color, bool Antialias = false);
		static void AddGradient(Vec2 Pos, Vec2 Size, D3DCOLOR Color, D3DCOLOR OtherColor, bool Vertical = false, bool Antialias = false);
	};

	extern GuiContext* Gui_Ctx;
}
