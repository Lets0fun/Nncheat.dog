#include "Globalincludes.h"
#include "MenuFramework/Framework.h"
#include "../hooks/hooks.hpp"
#include "../Configuration/Config.h"
#include "Menu.h"

WNDPROC oWndProc;

void wnd_handler(UINT msg, WPARAM wParam) {

}

namespace Hooked
{
	static LRESULT __stdcall wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		auto g = IdaLovesMe::Gui_Ctx;

		if (msg == WM_MOUSEWHEEL) {
			if (g)
				g->MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		}

		if (msg == WM_KEYDOWN)
			if (g)
				if (wParam == c_config::get()->i["misc_menukey"] && (g->MenuAlpha == 255 || g->MenuAlpha == 0)) {
					CMenu::get()->set_menu_opened(!CMenu::get()->is_menu_opened());
				}

		if (CMenu::get()->is_menu_opened()) { //aka g.Windows.at(0)->Opened
			hooks::Hooked_WndProc(window, msg, wParam, lParam);
			//g->MenuAlpha = 255;

			if (wParam != 'W' && wParam != 'A' && wParam != 'S' && wParam != 'D' && wParam != VK_SHIFT && wParam != VK_CONTROL && wParam != VK_TAB && wParam != VK_SPACE )
				return true;
		}
		else {
			hooks::Hooked_WndProc2nd(window, msg, wParam, lParam);
		}
		

		return CallWindowProcA(oWndProc, window, msg, wParam, lParam);
	}
}