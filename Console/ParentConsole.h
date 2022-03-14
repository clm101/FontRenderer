#ifndef PARENT_CONSOLE_H
#define PARENT_CONSOLE_H

#include <string>
#include <chrono>
#include <iostream>

#include <Windows.h>

#include <Message.h>
#include <ConsoleCommon.h>

namespace clm {
	class Console {
		using duration_t = std::chrono::duration<size_t, std::milli>;
	public:
		Console() = delete;
		Console(std::string);
		~Console();
		Console(const Console&) = delete;
		Console(Console&&) = delete;
		Console& operator=(const Console&) = delete;
		Console& operator=(Console&&) = delete;

		template<MessageType type>
		void send_message(Message<type>);
	private:
		//bool client_opens_pipe(duration_t);
		void send_text(std::string);

		PROCESS_INFORMATION m_processInfo;
		STARTUPINFOA m_startupInfo{};
		HANDLE m_pipe;
		bool m_connected;
	};


	template<MessageType type>
	void Console::send_message(Message<type> msg)
	{
		size_t msgSize{sizeof(MessageHeader<type>)};
		if constexpr (type == MessageType::Text)
		{
			const Message<MessageType::Text>& msgRef{msg};
			msgSize += msgRef.m_text.size();
			msgSize += 1;
		}
		else if constexpr (type == MessageType::SetForeground)
		{
			msgSize += sizeof(ForegroundColor);
		}
		else if constexpr (type == MessageType::SetBackground)
		{
			msgSize += sizeof(BackgroundColor);
		}
		else if constexpr (type == MessageType::SetForegroundRGB)
		{
			msgSize += 3 * sizeof(rgb_t);
		}
		else if constexpr (type == MessageType::SetBackgroundRGB)
		{
			msgSize += 3 * sizeof(rgb_t);
		}

		std::vector<char> buffer{std::vector(msgSize, '\0')};
		std::memcpy(reinterpret_cast<void*>(buffer.data()),
					reinterpret_cast<void*>(&msg),
					sizeof(MessageType));
		uintptr_t offset{sizeof(MessageType)};
		if constexpr (type == MessageType::Text)
		{

			Message<MessageType::Text>& msgRef{msg};
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(msgRef.m_text.data()),
						msgSize - offset);
		}
		else if constexpr (type == MessageType::SetForeground)
		{
			Message<MessageType::SetForeground>& msgRef{msg};
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_color),
						sizeof(ForegroundColor));
		}
		else if constexpr (type == MessageType::SetBackground)
		{
			Message<MessageType::SetBackground>& msgRef{msg};
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_color),
						sizeof(BackgroundColor));
		}
		else if constexpr (type == MessageType::SetForegroundRGB)
		{
			Message<MessageType::SetForegroundRGB>& msgRef{msg};
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_red),
						sizeof(rgb_t));
			offset += sizeof(rgb_t);
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_green),
						sizeof(rgb_t));
			offset += sizeof(rgb_t);
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_green),
						sizeof(rgb_t));
		}
		else if constexpr (type == MessageType::SetBackgroundRGB)
		{
			Message<MessageType::SetBackgroundRGB>& msgRef{msg};
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_red),
						sizeof(rgb_t));
			offset += sizeof(rgb_t);
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_green),
						sizeof(rgb_t));
			offset += sizeof(rgb_t);
			std::memcpy(reinterpret_cast<void*>(buffer.data() + offset),
						reinterpret_cast<void*>(&msgRef.m_green),
						sizeof(rgb_t));
		}

		DWORD bytesWritten{};
		if (WriteFile(m_pipe,
					  buffer.data(),
					  static_cast<DWORD>(msgSize),
					  &bytesWritten,
					  nullptr) == FALSE)
		{
			std::cerr << std::format("Error writing data to pipe. Error: {}", get_error_msg());
		}
		std::cout << std::format("Wrote {} bytes.\n", bytesWritten);
	}

	template<MessageType type>
	Console& operator<<(Console& console, const Message<type>& msg)
	{
		console.send_message(msg);
		return console;
	}

	Console& operator<<(Console&, ForegroundColor);
	Console& operator<<(Console&, BackgroundColor);
	Console& operator<<(Console&, std::string);
	Console& operator<<(Console&, const char*);
}

#endif