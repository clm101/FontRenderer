#ifndef CLM_WINDOWS_H
#define CLM_WINDOWS_H
#include <clmUtil/clm_system.h>
#include <string>
#include <exception>
#include <source_location>
#include <format>
#include <optional>
#include <memory>
#include "WindowsMessageMap.h"
#include "EventSystem.h"
#include "Keyboard.h"

namespace clm {
	class Windows {
	public:
		Windows(const std::wstring&, std::uint32_t, std::uint32_t, std::shared_ptr<EventSystem>);
		~Windows() noexcept;
		Windows(const Windows&) = delete;
		Windows(const Windows&&) = delete;
		Windows& operator=(const Windows&) = delete;
		Windows& operator=(Windows&&) = delete;

		std::optional<int> sys_update() const noexcept;
		const HWND get_hwnd() const noexcept { return m_hwnd; }
		const math::Rect_t window_dimensions() const noexcept { return m_windowDimensions; }

		void set_window_title(const std::wstring&) noexcept;
	private:
		LRESULT CALLBACK msg_handler(HWND, UINT, WPARAM, LPARAM);
		static LRESULT CALLBACK win_callback(HWND, UINT, WPARAM, LPARAM);
		static LRESULT CALLBACK win_callback_setup(HWND, UINT, WPARAM, LPARAM);
		// If Windows throws, need to deregister
		class WindowsLowLevel {
		public:
			WindowsLowLevel(HINSTANCE, const std::wstring&);
			~WindowsLowLevel() noexcept;
			const std::wstring& get_class_name() const noexcept { return wstrClassname; }
			const HINSTANCE get_hinstance() const noexcept { return hInst; }
		private:
			std::wstring wstrClassname;
			HINSTANCE hInst;
		};
		std::shared_ptr<EventSystem> m_eventSystem;
		std::unique_ptr<WindowsLowLevel> m_windowsClass;
		HWND m_hwnd;
		math::Rect_t m_windowDimensions;
	};
}

#endif