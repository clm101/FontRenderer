#include <ConsoleCommon.h>

#include <format>
#include <stdexcept>

#include <Windows.h>

namespace clm {
	std::string get_error_msg()
	{
		DWORD error{GetLastError()};
		char* buffer{nullptr};
		DWORD bufferSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
										  nullptr,
										  error,
										  0,
										  reinterpret_cast<LPSTR>(&buffer),
										  1,
										  nullptr);
		if (bufferSize == 0)
		{
			return "Failed to get error message.\n";
		}
		else
		{
			std::string errorMessage{std::format("{}", buffer)};
			LocalFree(buffer);
			return errorMessage;
		}
	}

	std::string get_pipe_name(std::string consoleName)
	{
		std::string pipeName{};
		for (const char& c : consoleName)
		{
			if (c == ' ') continue;
			pipeName.push_back(c);
		}
		return std::format("\\\\.\\pipe\\{}", pipeName);
	}

	void configure_console()
	{
		HANDLE stdOutput{GetStdHandle(STD_OUTPUT_HANDLE)};
		if (stdOutput == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error{std::format("Invalid handle. Error: {}",
												 get_error_msg())};
		}
		else
		{
			DWORD consoleMode{};
			if (GetConsoleMode(stdOutput, &consoleMode) == FALSE)
			{
				throw std::runtime_error{std::format("Unable to get console mode. Error: {}\n",
													 get_error_msg())};
			}
			else
			{
				consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				if (SetConsoleMode(stdOutput,
								   consoleMode) == FALSE)
				{
					throw std::runtime_error{std::format("Unable to set console Mode. Error: {}\n",
														 get_error_msg())};
				}
			}
		}
	}
}