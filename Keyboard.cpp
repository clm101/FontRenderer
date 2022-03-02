#include "Keyboard.h"
#include <format>

namespace clm {
	Keyboard::Keyboard() noexcept : umPressedKeys({}), shiftDown(false) { }
	Keyboard::~Keyboard() noexcept { umPressedKeys.clear(); }
	Keyboard::Keyboard(const Keyboard& rhs) noexcept { umPressedKeys = rhs.umPressedKeys; }
	Keyboard::Keyboard(Keyboard&& rhs) noexcept { umPressedKeys = std::move(rhs.umPressedKeys); }
	const Keyboard& Keyboard::operator=(const Keyboard& rhs) noexcept
	{
		umPressedKeys = rhs.umPressedKeys;
		return *this;
	}
	const Keyboard& Keyboard::operator=(Keyboard&& rhs) noexcept
	{
		umPressedKeys = std::move(rhs.umPressedKeys);
		return *this;
	}

	void Keyboard::key_pressed(Key k) noexcept(util::release)
	{
		if (!umPressedKeys.contains(k))
		{
#ifdef _DEBUG
			std::pair<decltype(umPressedKeys.begin()), bool> ret = umPressedKeys.insert({ k, 0 });
			if (ret.second == false)
			{
				throw KeyboardException{ k, std::source_location::current() };
			}
#else
			umPressedKeys.insert({ k, 0 });
#endif
		}
		auto keyPair = *(umPressedKeys.find(k));
		keyPair.second++;

		if (is_control_key(keyPair.first))
		{
			if (keyPair.first == Key::LShift || keyPair.first == Key::RShift) shiftDown = true;
		}
	}

	void Keyboard::key_released(Key k) noexcept
	{
		if (umPressedKeys.contains(k))
		{
			const auto keyPair = *(umPressedKeys.find(k));
			if (keyPair.first == Key::LShift || keyPair.first == Key::RShift) shiftDown = false;
			umPressedKeys.erase(umPressedKeys.find(k));
		}
	}

	bool Keyboard::shift_down() const noexcept { return shiftDown; }

	Key get_key(size_t nKeyCode)
	{
		const auto& i = keyCodeToKey.find(nKeyCode);
		if (i == keyCodeToKey.end())
		{
			return Key::NONE;
		}
		else
		{
			return i->second;
		}
	}

	Key get_key(char16_t u16KeyCode)
	{
		const auto& i = u16KeyCodeToKey.find(u16KeyCode);
		if (i == u16KeyCodeToKey.end())
		{
			return Key::NONE;
		}
		else
		{
			return i->second;
		}
	}

	char get_char(Key key)
	{
		const auto& i = keyToChar.find(key);
		if (i == keyToChar.end())
		{
			return '\0';
		}
		else
		{
			return i->second;
		}
	}

	wchar_t get_wchar(Key key)
	{
		const auto& i = keyToWChar.find(key);
		if (i == keyToWChar.end())
		{
			return L'\0';
		}
		else
		{
			return i->second;
		}
	}

	KeyboardException::KeyboardException(Key k, const std::source_location& loc) noexcept
		:
		std::exception(),
		strMsg({})
	{
		strMsg = std::format("KeyboardException: key {}\nLine: {}\nFunction: {}\nFile: {}\n", static_cast<size_t>(k), loc.line(), loc.function_name(), loc.file_name());
	}

	KeyboardException::~KeyboardException() noexcept { strMsg.clear(); }

	const char* KeyboardException::what() const noexcept
	{
		return strMsg.c_str();
	}
}