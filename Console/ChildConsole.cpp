#include <ChildConsole.h>

#include <ConsoleCommon.h>

#include <iostream>
#include <vector>
#include <thread>
#include <format>

namespace clm {
	ConsoleProcess::ConsoleProcess(std::string consoleName)
		:
		m_pipe(nullptr),
		m_listening(false)
	{
		using namespace std::chrono_literals;

		if (SetConsoleTitleA(consoleName.c_str()) == FALSE)
		{
			std::cerr << "Unable to set console title.\n";
		}
		std::string pipeName{get_pipe_name(consoleName)};
		std::cout << std::format("Pipe name: {}\n", pipeName);
		m_pipe = CreateFileA(pipeName.c_str(),
							 GENERIC_READ | FILE_WRITE_ATTRIBUTES,
							 0,
							 nullptr,
							 OPEN_EXISTING,
							 0,
							 nullptr);
		if (m_pipe == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error{std::format("Unable to open pipe. Error: {}",
												 get_error_msg())};
		}
		else
		{
			DWORD pipeMode{PIPE_READMODE_MESSAGE};
			if (SetNamedPipeHandleState(m_pipe,
										&pipeMode,
										nullptr,
										nullptr) == FALSE)
			{
				throw std::runtime_error{std::format("Unable to change pipe state. Error: {}",
													 get_error_msg())};
			}
			else
			{
				m_listening = true;
			}
		}

		configure_console();
	}

	ConsoleProcess::~ConsoleProcess()
	{
		CloseHandle(m_pipe);
	}

	void ConsoleProcess::run()
	{
		std::vector<char> buffer{std::vector(BUFFER_SIZE, '\0')};
		DWORD bytesRead{};
		while (m_listening)
		{
			if (ReadFile(m_pipe,
						 buffer.data(),
						 BUFFER_SIZE,
						 &bytesRead,
						 nullptr) == FALSE)
			{
				throw std::runtime_error{std::format("Error getting message. Error: {}",
													 get_error_msg())};
				break;
			}
			else
			{
				MessageType msgType = *reinterpret_cast<MessageType*>(buffer.data());
				char* msgData = buffer.data() + sizeof(MessageType);
				if (msgType == MessageType::Close)
				{
					m_listening = false;
				}
				else
				{
					switch (msgType)
					{
					case MessageType::Text:
					{
#ifdef _DEBUG
						if (msgData[bytesRead - sizeof(MessageType) - 1] != '\0')
						{
							std::cerr << "Error: Invalid string received.\n";
							std::cerr << "Received " << bytesRead << " bytes.\n";
						}
#endif
						Message<MessageType::Text> msg{};
						msg.m_text = std::string(msgData, bytesRead - sizeof(MessageType) - 1);
#ifdef _DEBUG
						if (msgData[bytesRead - sizeof(MessageType) - 1] != '\0')
						{
							std::cout << std::format("String length: {}\n", msg.m_text.size());
						}
#endif
						std::cout << msg.m_text;
						break;
					}
					case MessageType::SetForeground:
					{
#ifdef _DEBUG
						if (bytesRead != sizeof(Message<MessageType::SetForeground>))
						{
							std::cerr << "Error: Incorrect message size for MessageType::SetForeground.\n";
						}
#endif
						Message<MessageType::SetForeground> msg{};
						msg.m_color = *reinterpret_cast<ForegroundColor*>(msgData);
						set_foreground(msg.m_color);
						break;
					}
					case MessageType::SetBackground:
					{
#ifdef _DEBUG
						if (bytesRead != sizeof(Message<MessageType::SetBackground>))
						{
							std::cerr << "Error: Incorrect message size for MessageType::SetBackground.\n";
						}
#endif
						Message<MessageType::SetBackground> msg{};
						msg.m_color = *reinterpret_cast<BackgroundColor*>(msgData);
						set_background(msg.m_color);
						break;
					}
					case MessageType::SetForegroundRGB:
					{
#ifdef _DEBUG
						if (bytesRead != sizeof(Message<MessageType::SetForegroundRGB>))
						{
							std::cerr << "Error: Incorrect message size for MessageType::SetForegroundRGB.\n";
						}
#endif
						Message<MessageType::SetForegroundRGB> msg{};
						msg.m_red = *reinterpret_cast<rgb_t*>(msgData);
						msg.m_green = *reinterpret_cast<rgb_t*>(msgData + sizeof(rgb_t));
						msg.m_blue = *reinterpret_cast<rgb_t*>(msgData + 2 * sizeof(rgb_t));
						set_foreground_rgb(msg.m_red, msg.m_green, msg.m_blue);
						break;
					}
					case MessageType::SetBackgroundRGB:
					{
#ifdef _DEBUG
						if (bytesRead != sizeof(Message<MessageType::SetBackgroundRGB>))
						{
							std::cerr << "Error: Incorrect message size for MessageType::SetBackgroundRGB.\n";
						}
#endif
						Message<MessageType::SetBackgroundRGB> msg{};
						msg.m_red = *reinterpret_cast<rgb_t*>(msgData);
						msg.m_green = *reinterpret_cast<rgb_t*>(msgData + sizeof(rgb_t));
						msg.m_blue = *reinterpret_cast<rgb_t*>(msgData + 2 * sizeof(rgb_t));
						set_background_rgb(msg.m_red, msg.m_green, msg.m_blue);
						break;
					}
					default:
						std::cerr << "\x1b[37mUnknown message type. Closing...\n";
						m_listening = false;
					}
				}
			}
		}
	}

	void ConsoleProcess::set_foreground(ForegroundColor color) noexcept
	{
		// Based on table under Text Formatting at https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example
		const uint8_t colorCode = static_cast<uint8_t>(color) + (static_cast<uint8_t>(color) > 8 ? 81 : 29);
		std::cout << std::format("Color code: {}\n", std::to_string(colorCode));
		std::cout << std::format("\x1b[{}m", std::to_string(colorCode));
	}

	void ConsoleProcess::set_background(BackgroundColor color) noexcept
	{
		// Based on table under Text Formatting at https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example
		const uint8_t colorCode = static_cast<uint8_t>(color) + (static_cast<uint8_t>(color) > 8 ? 91 : 39);
		std::cout << std::format("\x1b[{}m", std::to_string(colorCode));
	}

	void ConsoleProcess::set_foreground_rgb(rgb_t red, rgb_t green, rgb_t blue) noexcept
	{
		std::cout << std::format("\x1b[38;2;{};{};{}",
								 std::to_string(red),
								 std::to_string(green),
								 std::to_string(blue));
	}

	void ConsoleProcess::set_background_rgb(rgb_t red, rgb_t green, rgb_t blue) noexcept
	{
		std::cout << std::format("\x1b[48;2;{};{};{}",
								 std::to_string(red),
								 std::to_string(green),
								 std::to_string(blue));
	}
}