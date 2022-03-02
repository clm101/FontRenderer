#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H
#include "System.h"
#include <queue>
#include <optional>
#include <memory>
#include <concepts>
//#include <cassert>
#include "Keyboard.h"
#include <clmMath/clm_rect.h>

namespace clm {
	using uint16_t = std::uint16_t;
	typedef size_t EventID;

	//static EventID register_event() { static EventID currentID = 0; return ++currentID; }

	enum class EventType : size_t {
		Keyboard,
		Mouse,
		WindowResizing
	};
	typedef EventType EventType_t;
	typedef std::queue<EventType_t> EventQueue_t;

	class KeyboardEvent {
	public:
		KeyboardEvent(Key, bool, uint16_t) noexcept;
		~KeyboardEvent() noexcept;
		KeyboardEvent(const KeyboardEvent&) noexcept;
		KeyboardEvent(KeyboardEvent&&) noexcept;
		const KeyboardEvent& operator=(const KeyboardEvent&) noexcept;
		const KeyboardEvent& operator=(KeyboardEvent&&) noexcept;

		bool get_pressed() const noexcept;
		uint16_t get_repeat_cnt() const noexcept;
		Key get_key() const noexcept;
	private:
		bool bPressed;
		uint16_t nRepeatCnt;
		Key kKey;
	};
	typedef KeyboardEvent KeyboardEvent_t;
	typedef std::queue<KeyboardEvent_t> KeyboardEventQueue_t;

	class WindowResizingEvent {
	public:
		WindowResizingEvent(math::Rect rect) noexcept;
		WindowResizingEvent(const WindowResizingEvent& rhs) noexcept;
		WindowResizingEvent(WindowResizingEvent&& rhs) noexcept;
		const WindowResizingEvent& operator=(const WindowResizingEvent& rhs) noexcept;
		const WindowResizingEvent& operator=(WindowResizingEvent&& rhs) noexcept;

		const math::Rect_t& get_rect() const noexcept;
	private:
		math::Rect_t rect;
	};
	typedef WindowResizingEvent WindowResizingEvent_t;
	typedef std::queue<WindowResizingEvent_t> WindowResizingEventQueue_t;

	class EventSystem {
	public:
		EventSystem() = default;
		~EventSystem() = default;
		EventSystem(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		const EventSystem& operator=(const EventSystem&) = delete;
		const EventSystem& operator=(EventSystem&&) = delete;

		std::optional<EventType_t> get_next_event_type() noexcept;
		void push_keyboard_event(const KeyboardEvent_t& k) noexcept;
		const KeyboardEvent_t pop_keyboard_event();
		void push_window_resizing_event(const WindowResizingEvent_t&) noexcept;
		const WindowResizingEvent_t pop_window_resizing_event();
		//void post_mouse_event(const MouseEvent& m) { dqEventQueue.push_back(m); }
		//void post_event(Event_t e) { dqEventQueue.push_back(e); }

	private:
		EventQueue_t eventQueue;
		KeyboardEventQueue_t keyboardEventQueue;
		WindowResizingEventQueue_t windowResizingEventQueue;
	};
}
#endif