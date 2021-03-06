#include "WindowsMessageMap.h"

namespace clm {
	WindowsMessages::WindowsMessages() noexcept
	{
		umMessages = {
			REGISTER_MESSAGE(WM_ACTIVATE),
			REGISTER_MESSAGE(WM_ACTIVATEAPP),
			REGISTER_MESSAGE(WM_CANCELMODE),
			REGISTER_MESSAGE(WM_CAPTURECHANGED),
			REGISTER_MESSAGE(WM_CHAR),
			REGISTER_MESSAGE(WM_CHARTOITEM),
			REGISTER_MESSAGE(WM_CHILDACTIVATE),
			REGISTER_MESSAGE(WM_CLOSE),
			REGISTER_MESSAGE(WM_COMPAREITEM),
			REGISTER_MESSAGE(WM_CONTEXTMENU),
			REGISTER_MESSAGE(WM_CREATE),
			REGISTER_MESSAGE(WM_DELETEITEM),
			REGISTER_MESSAGE(WM_DESTROY),
			REGISTER_MESSAGE(WM_DEVMODECHANGE),
			REGISTER_MESSAGE(WM_DPICHANGED),
			REGISTER_MESSAGE(WM_DRAWITEM),
			REGISTER_MESSAGE(WM_DWMNCRENDERINGCHANGED),
			REGISTER_MESSAGE(WM_ENABLE),
			REGISTER_MESSAGE(WM_ENDSESSION),
			REGISTER_MESSAGE(WM_ENTERIDLE),
			REGISTER_MESSAGE(WM_ENTERMENULOOP),
			REGISTER_MESSAGE(WM_ENTERSIZEMOVE),
			REGISTER_MESSAGE(WM_ERASEBKGND),
			REGISTER_MESSAGE(WM_EXITMENULOOP),
			REGISTER_MESSAGE(WM_EXITSIZEMOVE),
			REGISTER_MESSAGE(WM_FONTCHANGE),
			REGISTER_MESSAGE(WM_GETFONT),
			REGISTER_MESSAGE(WM_GETHOTKEY),
			REGISTER_MESSAGE(WM_GETICON),
			REGISTER_MESSAGE(WM_GETMINMAXINFO),
			REGISTER_MESSAGE(WM_GETOBJECT),
			REGISTER_MESSAGE(WM_GETTEXT),
			REGISTER_MESSAGE(WM_GETTEXTLENGTH),
			REGISTER_MESSAGE(WM_ICONERASEBKGND),
			REGISTER_MESSAGE(WM_IME_SETCONTEXT),
			REGISTER_MESSAGE(WM_IME_NOTIFY),
			REGISTER_MESSAGE(WM_INITMENU),
			REGISTER_MESSAGE(WM_INPUTLANGCHANGE),
			REGISTER_MESSAGE(WM_INPUTLANGCHANGEREQUEST),
			REGISTER_MESSAGE(WM_KEYFIRST),
			REGISTER_MESSAGE(WM_KEYUP),
			REGISTER_MESSAGE(WM_LBUTTONDOWN),
			REGISTER_MESSAGE(WM_LBUTTONUP),
			REGISTER_MESSAGE(WM_KILLFOCUS),
			REGISTER_MESSAGE(WM_MEASUREITEM),
			REGISTER_MESSAGE(WM_MENUCHAR),
			REGISTER_MESSAGE(WM_MENUSELECT),
			REGISTER_MESSAGE(WM_MOUSEACTIVATE),
			REGISTER_MESSAGE(WM_MOUSEMOVE),
			REGISTER_MESSAGE(WM_MOVE),
			REGISTER_MESSAGE(WM_MOVING),
			REGISTER_MESSAGE(WM_NCACTIVATE),
			REGISTER_MESSAGE(WM_NCCALCSIZE),
			REGISTER_MESSAGE(WM_NCCREATE),
			REGISTER_MESSAGE(WM_NCDESTROY),
			REGISTER_MESSAGE(WM_NCHITTEST),
			REGISTER_MESSAGE(WM_NCLBUTTONDOWN),
			REGISTER_MESSAGE(WM_NCMOUSELEAVE),
			REGISTER_MESSAGE(WM_NCMOUSEMOVE),
			REGISTER_MESSAGE(WM_NCPAINT),
			REGISTER_MESSAGE(WM_NEXTDLGCTL),
			REGISTER_MESSAGE(WM_NULL),
			REGISTER_MESSAGE(WM_PAINT),
			REGISTER_MESSAGE(WM_PAINTICON),
			REGISTER_MESSAGE(WM_POWERBROADCAST),
			REGISTER_MESSAGE(WM_QUERYDRAGICON),
			REGISTER_MESSAGE(WM_QUERYOPEN),
			REGISTER_MESSAGE(WM_QUEUESYNC),
			REGISTER_MESSAGE(WM_QUIT),
			REGISTER_MESSAGE(WM_RBUTTONDOWN),
			REGISTER_MESSAGE(WM_RBUTTONUP),
			REGISTER_MESSAGE(WM_SETCURSOR),
			REGISTER_MESSAGE(WM_SETFOCUS),
			REGISTER_MESSAGE(WM_SETFONT),
			REGISTER_MESSAGE(WM_SETHOTKEY),
			REGISTER_MESSAGE(WM_SETICON),
			REGISTER_MESSAGE(WM_SETREDRAW),
			REGISTER_MESSAGE(WM_SETTEXT),
			REGISTER_MESSAGE(WM_SHOWWINDOW),
			REGISTER_MESSAGE(WM_SIZE),
			REGISTER_MESSAGE(WM_SIZING),
			REGISTER_MESSAGE(WM_SPOOLERSTATUS),
			REGISTER_MESSAGE(WM_STYLECHANGED),
			REGISTER_MESSAGE(WM_STYLECHANGING),
			REGISTER_MESSAGE(WM_SYNCPAINT),
			REGISTER_MESSAGE(WM_SYSCHAR),
			REGISTER_MESSAGE(WM_SYSCOLORCHANGE),
			REGISTER_MESSAGE(WM_SYSCOMMAND),
			REGISTER_MESSAGE(WM_SYSKEYDOWN),
			REGISTER_MESSAGE(WM_SYSKEYUP),
			REGISTER_MESSAGE(WM_THEMECHANGED),
			REGISTER_MESSAGE(WM_TIMECHANGE),
			REGISTER_MESSAGE(WM_TIMER),
			REGISTER_MESSAGE(WM_USERCHANGED),
			REGISTER_MESSAGE(WM_VKEYTOITEM),
			REGISTER_MESSAGE(WM_WINDOWPOSCHANGED),
			REGISTER_MESSAGE(WM_WINDOWPOSCHANGING),
			REGISTER_MESSAGE(WM_WININICHANGE)
		};
	}

	WindowsMessages::~WindowsMessages() noexcept
	{
		umMessages.clear();
	}

	std::wstring WindowsMessages::operator()(UINT dwMsg, WPARAM wParam, LPARAM lParam) const noexcept
	{
		std::wostringstream oss;
		constexpr int nColWidth = 25;
		const auto tmp = umMessages.find(dwMsg);

		if (tmp != umMessages.end())
		{
			oss << std::left << std::setw(nColWidth) << tmp->second << std::right;
		}
		else
		{
			std::wostringstream ossPad;
			ossPad << L"Unknown message: 0x" << std::hex << std::setfill(L'0') << dwMsg;
			oss << std::left << std::setw(nColWidth) << ossPad.str() << std::right;
		}
		oss << L"\tlParam: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << lParam
			<< L"\twParam: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << wParam
			<< std::endl;
		return oss.str();
	}
}