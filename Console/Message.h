#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

#include <ConsoleCommon.h>

namespace clm {
	enum class MessageType : uint8_t {
		SetForeground = 1,
		SetBackground,
		SetForegroundRGB,
		SetBackgroundRGB,
		Text,
		Close
	};

	template<MessageType type>
	struct MessageHeader {
		const MessageType m_type = type;
	};

	template<MessageType type>
	struct Message : public MessageHeader<type> {};

	template<>
	struct Message<MessageType::Text> : public MessageHeader<MessageType::Text> {
		std::string m_text;

		Message()
			:
			m_text("")
		{}
		Message(std::string msg)
			:
			m_text(msg)
		{}
	};

	template<>
	struct Message<MessageType::SetForeground> : public MessageHeader<MessageType::SetForeground> {
		ForegroundColor m_color;

		Message()
			:
			m_color(ForegroundColor::White)
		{}
		Message(ForegroundColor color)
			:
			m_color(color)
		{}
	};

	template<>
	struct Message<MessageType::SetBackground> : public MessageHeader<MessageType::SetBackground> {
		BackgroundColor m_color;

		Message()
			:
			m_color(BackgroundColor::Black)
		{}
		Message(BackgroundColor color)
			:
			m_color(color)
		{}
	};

	template<>
	struct Message<MessageType::SetForegroundRGB> : public MessageHeader<MessageType::SetForegroundRGB> {
		uint8_t m_red, m_green, m_blue;

		Message()
			:
			m_red(0), m_green(0), m_blue(0)
		{}
		Message(uint8_t red, uint8_t green, uint8_t blue)
			:
			m_red(red), m_green(green), m_blue(blue)
		{}
	};

	template<>
	struct Message<MessageType::SetBackgroundRGB> : public MessageHeader<MessageType::SetBackgroundRGB> {
		uint8_t m_red, m_green, m_blue;

		Message()
			:
			m_red(0), m_green(0), m_blue(0)
		{}
		Message(uint8_t red, uint8_t green, uint8_t blue)
			:
			m_red(red), m_green(green), m_blue(blue)
		{}
	};
}
#endif