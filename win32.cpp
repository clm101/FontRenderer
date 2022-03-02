#include "win32.h"

#include <clmUtil/clm_err.h>

namespace clm {
	Windows::Windows(const std::wstring& windowName, std::uint32_t width, std::uint32_t height, std::shared_ptr<EventSystem> eventSystem)
		:
		m_eventSystem(eventSystem)
	{
		err::check_ret_val<err::WindowsException>(CoInitialize(NULL));
		m_windowsClass = std::make_unique<WindowsLowLevel>(GetModuleHandle(nullptr), std::format(L"{}_class", windowName));

		RECT rWindowDim{ 0, 0, static_cast<std::int32_t>(width), static_cast<std::int32_t>(height) };
		AdjustWindowRect(&rWindowDim, WS_CAPTION | WS_SIZEBOX, FALSE);

		m_hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
								 m_windowsClass->get_class_name().c_str(),
								 windowName.c_str(),
								 WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
								 100 + rWindowDim.left,
								 100 + rWindowDim.top,
								 rWindowDim.right - rWindowDim.left,
								 rWindowDim.bottom - rWindowDim.top,
								 nullptr,
								 nullptr,
								 m_windowsClass->get_hinstance(),
								 this);
		if (m_hwnd == NULL)
		{
			throw err::WindowsException{ std::source_location::current() };
		}
		ShowWindow(m_hwnd, SW_SHOW);
	}

	Windows::~Windows() noexcept
	{
		if (m_hwnd != NULL)
		{
			DestroyWindow(m_hwnd);
		}
		CoUninitialize();
	}

	std::optional<int> Windows::sys_update() const noexcept
	{
		MSG msg{};
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return static_cast<int>(msg.wParam);
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return {};
	}

	LRESULT CALLBACK Windows::msg_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		using uint16_t = std::uint16_t;
		static WindowsMessages wmMap{};
		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
		{
			m_eventSystem->push_keyboard_event(KeyboardEvent{
				get_key(static_cast<size_t>(wParam)),
				true,
				static_cast<uint16_t>(LOWORD(lParam))
												});
			return 0;
		}
		case WM_CHAR:
		{
			m_eventSystem->push_keyboard_event(KeyboardEvent{
				get_key(static_cast<char16_t>(wParam)),
				true,
				static_cast<uint16_t>(LOWORD(lParam)) });
			return 0;
		}
		case WM_KEYUP:
		{
			m_eventSystem->push_keyboard_event(KeyboardEvent{
				get_key(static_cast<size_t>(wParam)),
				false,
				static_cast<uint16_t>(LOWORD(lParam)) });
			return 0;
		}
		case WM_SIZING:
		{
			m_eventSystem->push_window_resizing_event(WindowResizingEvent{
				*reinterpret_cast<RECT*>(lParam) });
			break;
		}
		default:
			std::wstring wstrUncaught = std::format(L"Uncaught msg: {}", wmMap(msg, wParam, lParam));
			OutputDebugStringW(wstrUncaught.c_str());
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK Windows::win_callback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Windows* const ptrWnd = reinterpret_cast<Windows*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		return ptrWnd->msg_handler(hwnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK Windows::win_callback_setup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			Windows* const ptrWnd = static_cast<Windows*>(pCreate->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)ptrWnd);
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)Windows::win_callback);
			return ptrWnd->win_callback(hwnd, msg, wParam, lParam);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	Windows::WindowsLowLevel::WindowsLowLevel(HINSTANCE hInstIn, const std::wstring& wstrClassnameIn)
		:
		hInst(hInstIn),
		wstrClassname(wstrClassnameIn)
	{
		WNDCLASSEXW wcex{};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = win_callback_setup;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = nullptr;
		wcex.hCursor = nullptr;
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = wstrClassname.c_str();
		wcex.hIconSm = nullptr;

		err::check_ret_val<err::WindowsException>(RegisterClassExW(&wcex));
	}

	Windows::WindowsLowLevel::~WindowsLowLevel() noexcept
	{
		UnregisterClassW(wstrClassname.c_str(), hInst);
	}

	void Windows::set_window_title(const std::wstring& newWindowTitle) noexcept
	{
		SetWindowTextW(m_hwnd, newWindowTitle.c_str());
	}
}