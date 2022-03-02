#ifndef APPLICATION_BASE_H
#define APPLICATION_BASE_H
#include <memory>
#include <string>
#include <optional>
#include <format>

#include <clmMath/clm_rect.h>

#include "win32.h"
#include <GraphicsDevice.h>
#include <Font.h>
#include "EventSystem.h"
#include "Keyboard.h"


namespace clm {
	class Application {
	public:
		Application(const std::wstring&, std::uint32_t, std::uint32_t);
		~Application() = default;
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		int run();

		void set_window_title(const std::wstring&) noexcept;
	protected:
		std::shared_ptr<EventSystem> m_eventSystem;
		std::wstring m_applicationName;
		std::unique_ptr<Windows> m_window;
#ifdef GFX_REFAC
		GfxDevice m_gfxDevice;
#else
		std::unique_ptr<GraphicsDevice> m_gfx;
#endif
		Keyboard_t m_keyboard;
		Font m_font;

		void process_keyboard_event(const KeyboardEvent_t& keyboardEvent);
	};
}
#endif