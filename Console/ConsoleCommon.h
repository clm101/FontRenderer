#ifndef CONSOLE_COMMON_H
#define CONSOLE_COMMON_H

#include <string>

#include <Windows.h>

namespace clm {
	extern constexpr size_t BUFFER_SIZE = 1024;

	std::string get_error_msg();

	std::string get_pipe_name(std::string);

	void configure_console();

	enum class ForegroundColor : uint8_t {
		Black = 1,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,
		BrightBlack,
		BrightRed,
		BrightGreen,
		BrightYellow,
		BrightBlue,
		BrightMagenta,
		BrightCyan,
		BrightWhite,
	};

	enum class BackgroundColor : uint8_t {
		Black = 1,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,
		BrightBlack,
		BrightRed,
		BrightGreen,
		BrightYellow,
		BrightBlue,
		BrightMagenta,
		BrightCyan,
		BrightWhite,
	};

	using rgb_t = uint8_t;
}
#endif