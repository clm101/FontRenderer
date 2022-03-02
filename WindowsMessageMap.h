#ifndef WINDOWS_MESSAGE_MAP_H
#define WINDOWS_MESSAGE_MAP_H
#include <clmUtil/clm_system.h>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

#define REGISTER_MESSAGE(msg) {msg, L#msg}

namespace clm {
	class WindowsMessages {
	private:
		std::unordered_map<UINT, std::wstring> umMessages;
	public:
		WindowsMessages() noexcept;
		~WindowsMessages() noexcept;

		std::wstring operator()(UINT, WPARAM, LPARAM) const noexcept;
	};
}


#endif