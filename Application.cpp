#include <Application.h>

namespace clm {
	Application::Application(const std::wstring& applicationName, std::uint32_t width, std::uint32_t height)
		:
		m_eventSystem(std::make_shared<EventSystem>()),
		m_applicationName(applicationName),
		m_window(std::make_unique<Windows>(m_applicationName, width, height, m_eventSystem)),
#ifdef GFX_REFAC
		m_gfxDevice(),
#else
		m_gfx(std::make_unique<GraphicsDevice>(m_window->get_hwnd(), m_window->window_dimensions())),
#endif
		m_font("Bahnschrift", 500.0f)
	{
		//m_eventSystem = std::make_shared<EventSystem>();
		//m_window = std::make_unique<Windows>(m_applicationName, width, height, m_eventSystem);
		//m_gfx = std::make_unique<GraphicsDevice>(m_window->get_hwnd(), m_window->window_dimensions());
	}

	int Application::run()
	{
		while (true)
		{
			if (const std::optional<int>& ret = m_window->sys_update())
			{
				return *ret;
			}
			while (std::optional<EventType_t> nextEventType = m_eventSystem->get_next_event_type())
			{
				switch (*nextEventType)
				{
				case EventType::Keyboard:
				{
					KeyboardEvent_t keyboardEvent = m_eventSystem->pop_keyboard_event();
					if (keyboardEvent.get_pressed())
					{
						m_keyboard.key_pressed(keyboardEvent.get_key());
					}
					else
					{
						m_keyboard.key_released(keyboardEvent.get_key());
					}
					process_keyboard_event(keyboardEvent);
					break;
				}
				case EventType::WindowResizing:
				{
					WindowResizingEvent_t windowResizingEvent = m_eventSystem->pop_window_resizing_event();
#ifdef GFX_REFAC
#else
					m_gfx->resize_buffer(windowResizingEvent.get_rect());
#endif
					break;
				}
				default:
				{
					break;
				}
				}
			}
#ifdef GFX_REFAC
#else
			m_gfx->draw_frame();
#endif
		}
		return 0;
	}

	void Application::set_window_title(const std::wstring& newWindowTitle) noexcept
	{
		m_window->set_window_title(newWindowTitle);
	}

	void Application::process_keyboard_event(const KeyboardEvent_t& keyboardEvent)
	{
		if (keyboardEvent.get_pressed())
		{
			Key key = keyboardEvent.get_key();
			wchar_t letter = get_wchar(key);
			std::wstring windowTitle = std::format(
				L"Key pressed: {}", letter);
			set_window_title(windowTitle);
			if (is_printable(key))
			{
#ifdef GFX_REFAC
#else
				std::vector<font_triangle_t> triangles = m_font.get_triangles(m_keyboard.shift_down() ? shift_down(key) : letter);
				std::vector<font_triangle_t> missingGlyph = m_font.get_triangles(L'\0');
				m_gfx->draw_triangles(triangles);
#endif
			}
		}
	}
}