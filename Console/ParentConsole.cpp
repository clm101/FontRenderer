#include <ParentConsole.h>

#include <iostream>
#include <format>
#include <thread>
#include <stdexcept>

namespace clm {
	Console::Console(std::string consoleName)
		:
		m_processInfo({}),
		m_startupInfo({.cb = sizeof(STARTUPINFOA)}),
		m_pipe(nullptr),
		m_connected(false)
	{
		std::string pipeName{get_pipe_name(consoleName)};
		std::string consoleProcess{"ChildProcess.exe"};
		std::string consoleProcessPath = std::string(GetCurrentDirectoryA(0, nullptr),
													 '\0');
		GetCurrentDirectoryA(static_cast<DWORD>(consoleProcessPath.size()),
							 consoleProcessPath.data());
		consoleProcessPath.pop_back();
		if (IsDebuggerPresent())
		{
#ifdef _DEBUG
			consoleProcessPath += "\\x64\\Debug";
#else
			consoleProcessPath += "\\x64\\Release";
#endif
		}
		consoleProcessPath += std::format("\\{}", consoleProcess);

		std::string commandLine{std::format("{} \"{}\"",
											consoleProcessPath,
											consoleName)};
		SECURITY_ATTRIBUTES securityAttributes{};
		securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		SECURITY_DESCRIPTOR securityDescriptor{};
		if (InitializeSecurityDescriptor(&securityDescriptor,
										 SECURITY_DESCRIPTOR_REVISION) == FALSE)
		{
			throw std::runtime_error{std::format("Unable to create pipe for console. Error: {}\n",
												 get_error_msg())};
		}
		securityAttributes.lpSecurityDescriptor = &securityDescriptor;
		securityAttributes.bInheritHandle = FALSE;

		m_pipe = CreateNamedPipeA(pipeName.c_str(),
								  PIPE_ACCESS_OUTBOUND,
								  PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
								  1,
								  BUFFER_SIZE,
								  BUFFER_SIZE,
								  0,
								  nullptr);
		if (m_pipe == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error{std::format("Unable to create pipe for console. Error: {}", get_error_msg())};
		}

		if (CreateProcessA(consoleProcessPath.data(),
						   commandLine.data(),
						   nullptr,
						   nullptr,
						   FALSE,
						   CREATE_NEW_CONSOLE,
						   nullptr,
						   nullptr,
						   &m_startupInfo,
						   &m_processInfo) == FALSE)
		{
			throw std::runtime_error{std::format("Failed to create client process. Error: {}\n",
												 get_error_msg())};
		}
		else
		{
			using namespace std::chrono_literals;
			if (ConnectNamedPipe(m_pipe,
								 nullptr) == 0)
			{
				throw std::runtime_error{std::format("ConnectNamedPipe error. Error: {}\n",
													 get_error_msg())};
			}
			else
			{
				m_connected = true;
			}
		}

		configure_console();
	}

	Console::~Console()
	{
		using namespace std::chrono_literals;
		send_message(Message<MessageType::Close>{});

		std::chrono::system_clock::time_point start{std::chrono::system_clock::now()};
		DWORD exitCode{STILL_ACTIVE};
		size_t iterations{};
		while (exitCode == STILL_ACTIVE)
		{
			GetExitCodeProcess(m_processInfo.hProcess,
							   &exitCode);
			if (std::chrono::system_clock::now() - start > 10s)
			{
				std::cerr << "Client did not close in time. Force terminating...\n";
				TerminateProcess(m_processInfo.hProcess, 0);
				break;
			}
			std::this_thread::sleep_for(1ms);
			++iterations;
		}
		DisconnectNamedPipe(m_pipe);
		CloseHandle(m_pipe);
		CloseHandle(m_processInfo.hProcess);
		CloseHandle(m_processInfo.hThread);
	}

	void Console::send_text(std::string msg)
	{

	}

	Console& operator<<(Console& console, ForegroundColor color)
	{
		console.send_message(Message<MessageType::SetForeground>{color});
		return console;
	}

	Console& operator<<(Console& console, BackgroundColor color)
	{
		console.send_message(Message<MessageType::SetBackground>{color});
		return console;
	}

	Console& operator<<(Console& console, std::string text)
	{
		console.send_message(Message<MessageType::Text>{text});
		return console;
	}

	Console& operator<<(Console& console, const char* text)
	{
		console.send_message(Message<MessageType::Text>{text});
		return console;
	}
}