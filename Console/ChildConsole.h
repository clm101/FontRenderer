#ifndef CHILD_CONSOLE_H
#define CHILD_CONSOLE_H

#include <string>

#include <Windows.h>

#include <Message.h>
#include <ConsoleCommon.h>

namespace clm {
	class ConsoleProcess {
	public:
		ConsoleProcess() = delete;
		ConsoleProcess(std::string);
		~ConsoleProcess();
		ConsoleProcess(const ConsoleProcess&) = delete;
		ConsoleProcess(ConsoleProcess&&) = delete;
		ConsoleProcess& operator=(const ConsoleProcess&) = delete;
		ConsoleProcess& operator=(ConsoleProcess&&) = delete;

		void run();
	private:
		void set_foreground(ForegroundColor) noexcept;
		void set_background(BackgroundColor) noexcept;
		void set_foreground_rgb(rgb_t, rgb_t, rgb_t) noexcept;
		void set_background_rgb(rgb_t, rgb_t, rgb_t) noexcept;
		HANDLE m_pipe;
		bool m_listening;
	};
}

#endif