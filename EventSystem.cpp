#include "EventSystem.h"

namespace clm {
	bool KeyboardEvent::get_pressed() const noexcept { return bPressed; }
	uint16_t KeyboardEvent::get_repeat_cnt() const noexcept { return nRepeatCnt; }
	Key KeyboardEvent::get_key() const noexcept { return kKey; }

	KeyboardEvent::KeyboardEvent(Key kKey, bool bPressed, uint16_t nRepeatCnt) noexcept : kKey(kKey), bPressed(bPressed), nRepeatCnt(nRepeatCnt) { }
	KeyboardEvent::~KeyboardEvent() noexcept { }
	KeyboardEvent::KeyboardEvent(const KeyboardEvent& rhs) noexcept
	{
		kKey = rhs.kKey;
		bPressed = rhs.bPressed;
		nRepeatCnt = rhs.nRepeatCnt;
	}
	KeyboardEvent::KeyboardEvent(KeyboardEvent&& rhs) noexcept
	{
		kKey = rhs.kKey;
		rhs.kKey = Key::NONE;
		bPressed = rhs.bPressed;
		nRepeatCnt = rhs.nRepeatCnt;
		rhs.nRepeatCnt = 0;
	}
	const KeyboardEvent& KeyboardEvent::operator=(const KeyboardEvent& rhs) noexcept
	{
		kKey = rhs.kKey;
		bPressed = rhs.bPressed;
		nRepeatCnt = rhs.nRepeatCnt;
		return *this;
	}
	const KeyboardEvent& KeyboardEvent::operator=(KeyboardEvent&& rhs) noexcept
	{
		kKey = rhs.kKey;
		rhs.kKey = Key::NONE;
		bPressed = rhs.bPressed;
		nRepeatCnt = rhs.nRepeatCnt;
		rhs.nRepeatCnt = 0;
		return *this;
	}

	WindowResizingEvent::WindowResizingEvent(math::Rect rect) noexcept
		:
		rect(rect)
	{ }
	WindowResizingEvent::WindowResizingEvent(const WindowResizingEvent& rhs) noexcept
		:
		rect(rhs.rect)
	{ }
	WindowResizingEvent::WindowResizingEvent(WindowResizingEvent&& rhs) noexcept
	{
		rect = std::move(rhs.rect);
	}
	const WindowResizingEvent& WindowResizingEvent::operator=(const WindowResizingEvent& rhs) noexcept
	{
		rect = rhs.rect;
		return *this;
	}
	const WindowResizingEvent& WindowResizingEvent::operator=(WindowResizingEvent&& rhs) noexcept
	{
		rect = std::move(rhs.rect);
		return *this;
	}

	const math::Rect_t& WindowResizingEvent::get_rect() const noexcept { return rect; }

	std::optional<EventType_t> EventSystem::get_next_event_type() noexcept
	{
		if (eventQueue.size() == 0)
		{
			return {};
		}
		else
		{
			return { eventQueue.front() };
		}
	}

	void EventSystem::push_keyboard_event(const KeyboardEvent_t& keyboardEvent) noexcept
	{
		eventQueue.push(EventType::Keyboard);
		keyboardEventQueue.push(keyboardEvent);
	}

	const KeyboardEvent_t EventSystem::pop_keyboard_event()
	{
		// Commented code causes internal compiler error
//#ifdef _DEBUG
//		if (eventQueue.size() == 0)
//		{
//			throw err::EmptyQueueException{ std::source_location::current() };
//		}
//		if (keyboardEventQueue.size() == 0)
//		{
//			throw err::EmptyQueueException{ std::source_location::current() };
//		}
//#endif
		_ASSERTE(eventQueue.front() == EventType::Keyboard);
		_ASSERTE(eventQueue.size() != 0);
		_ASSERTE(keyboardEventQueue.size() != 0);
		eventQueue.pop();
		KeyboardEvent_t keyboardEvent = keyboardEventQueue.front();
		keyboardEventQueue.pop();
		return keyboardEvent;
	}

	void EventSystem::push_window_resizing_event(const WindowResizingEvent_t& windowResizingEvent) noexcept
	{
		eventQueue.push(EventType::WindowResizing);
		windowResizingEventQueue.push(windowResizingEvent);
	}

	const WindowResizingEvent_t EventSystem::pop_window_resizing_event()
	{
		// Internal compiler error caused by throwing
		_ASSERTE(eventQueue.front() == EventType::WindowResizing);
		_ASSERTE(eventQueue.size() != 0);
		_ASSERTE(windowResizingEventQueue.size() != 0);
		eventQueue.pop();
		WindowResizingEvent_t windowResizingEvent = windowResizingEventQueue.front();
		windowResizingEventQueue.pop();
		return windowResizingEvent;
	}
}