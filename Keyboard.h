#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <exception>
#include <source_location>
#include <unordered_map>
#include <clmUtil/clm_util.h>
#include "KeyboardInfo.h"

namespace clm {
	class Keyboard {
	public:
		Keyboard() noexcept;
		~Keyboard() noexcept;
		Keyboard(const Keyboard& rhs) noexcept;
		Keyboard(Keyboard&& rhs) noexcept;
		const Keyboard& operator=(const Keyboard& rhs) noexcept;
		const Keyboard& operator=(Keyboard&& rhs) noexcept;
		void key_pressed(Key k) noexcept(util::release);
		void key_released(Key k) noexcept;
		bool shift_down() const noexcept;
	private:
		std::unordered_map<Key, unsigned int> umPressedKeys;
		bool shiftDown;
	};
	typedef Keyboard Keyboard_t;

	Key get_key(size_t nKeyCode);
	Key get_key(char16_t u16KeyCode);
	char get_char(Key key);
	wchar_t get_wchar(Key key);

	class KeyboardException : public std::exception {
	public:
		/*KeyboardException(Key k, const std::source_location& loc = std::source_location::current()) noexcept*/
		KeyboardException(Key k, const std::source_location& loc) noexcept;
		~KeyboardException() noexcept;
		const char* what() const noexcept override;
	private:
		std::string strMsg;
	};
}

#endif