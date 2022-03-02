#ifndef KEYBOARD_INFO_H
#define KEYBOARD_INFO_H

#include <unordered_map>
#include <ostream>
#include <format>
#include <locale>

#include <clmUtil/clm_system.h>
#include <clmUtil/clm_util.h>

namespace clm
{
	enum class Key : size_t {
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		N1, N2, N3, N4, N5, N6, N7, N8, N9, N0,
		Period, Comma, SingleDoubleQuote, ForwardSlashQuestion, BackSlashPipe, Plus, Equal, Minus,
		Underscore, AccentTilde, LeftAngle, RightAngle, LeftSquareCurly, RightSquareCurly, LeftParen, RightParen,
		Colon, Semicolon, Exclamation, At, Pound, Dollar, Percent, Caret, Ampersand, Asterix,
		LShift, RShift, LCtrl, RCtrl, LAlt, RAlt, LeftArrow, RightArrow, UpArrow, DownArrow, Space,
		NONE
	};
	extern std::unordered_map<size_t, Key> keyCodeToKey;
	extern std::unordered_map<char16_t, Key> u16KeyCodeToKey;
	extern std::unordered_map<Key, char> keyToChar;
	extern std::unordered_map<Key, wchar_t> keyToWChar;

	extern bool is_printable(Key key);

	extern bool is_control_key(Key key);

	extern inline wchar_t shift_down(Key key) noexcept(util::release)
	{
		switch (key)
		{
		case Key::A:
			[[fallthrough]];
		case Key::B:
			[[fallthrough]];
		case Key::C:
			[[fallthrough]];
		case Key::D:
			[[fallthrough]];
		case Key::E:
			[[fallthrough]];
		case Key::F:
			[[fallthrough]];
		case Key::G:
			[[fallthrough]];
		case Key::H:
			[[fallthrough]];
		case Key::I:
			[[fallthrough]];
		case Key::J:
			[[fallthrough]];
		case Key::K:
			[[fallthrough]];
		case Key::L:
			[[fallthrough]];
		case Key::M:
			[[fallthrough]];
		case Key::N:
			[[fallthrough]];
		case Key::O:
			[[fallthrough]];
		case Key::P:
			[[fallthrough]];
		case Key::Q:
			[[fallthrough]];
		case Key::R:
			[[fallthrough]];
		case Key::S:
			[[fallthrough]];
		case Key::T:
			[[fallthrough]];
		case Key::U:
			[[fallthrough]];
		case Key::V:
			[[fallthrough]];
		case Key::W:
			[[fallthrough]];
		case Key::X:
			[[fallthrough]];
		case Key::Y:
			[[fallthrough]];
		case Key::Z:
		{
#ifdef _DEBUG
			if (!keyToWChar.contains(key))
			{
				throw std::runtime_error{std::format("Can't complete shift_down on key {}", static_cast<size_t>(key))};
			}
#endif
			char16_t character = keyToWChar.find(key)->second;
			return std::toupper(character, std::locale(std::locale(), new std::ctype<char16_t>));
		}
		case Key::SingleDoubleQuote:
			return u'"';
		case Key::ForwardSlashQuestion:
			return u'?';
		case Key::BackSlashPipe:
			return u'|';
		case Key::LeftSquareCurly:
			return u'{';
		case Key::RightSquareCurly:
			return u'}';
		default:
			return u'\0';
		}
	}
	/*extern std::unordered_map<size_t, Key> keyCodeToKey{
		{'A', Key::A},
		{'B', Key::B},
		{'C', Key::C},
		{'D', Key::D},
		{'E', Key::E},
		{'F', Key::F},
		{'G', Key::G},
		{'H', Key::H},
		{'I', Key::I},
		{'J', Key::J},
		{'K', Key::K},
		{'L', Key::L},
		{'M', Key::M},
		{'N', Key::N},
		{'O', Key::O},
		{'P', Key::P},
		{'Q', Key::Q},
		{'R', Key::R},
		{'S', Key::S},
		{'T', Key::T},
		{'U', Key::U},
		{'V', Key::V},
		{'W', Key::W},
		{'X', Key::X},
		{'Y', Key::Y},
		{'Z', Key::Z},
		{'1', Key::N1},
		{'2', Key::N2},
		{'3', Key::N3},
		{'4', Key::N4},
		{'5', Key::N5},
		{'6', Key::N6},
		{'7', Key::N7},
		{'8', Key::N8},
		{'9', Key::N9},
		{'0', Key::N0},
		{VK_DECIMAL, Key::Period},
		{VK_OEM_COMMA, Key::Comma},
		{VK_OEM_7, Key::SingleDoubleQuote},
		{VK_OEM_2, Key::ForwardSlashQuestion},
		{VK_OEM_5, Key::BackSlashPipe},
		{VK_OEM_PLUS, Key::Plus},
		{VK_OEM_NEC_EQUAL, Key::Equal},
		{VK_OEM_MINUS, Key::Minus},
		{VK_OEM_3, Key::AccentTilde},
		{VK_LSHIFT, Key::LShift},
		{VK_RSHIFT, Key::RShift},
		{VK_SHIFT, Key::LShift},
		{VK_LCONTROL, Key::LCtrl},
		{VK_RCONTROL, Key::RCtrl},
		{VK_LMENU, Key::LAlt},
		{VK_RMENU, Key::RAlt},
		{VK_RIGHT, Key::RightArrow},
		{VK_LEFT, Key::LeftArrow},
		{VK_UP, Key::UpArrow},
		{VK_DOWN, Key::DownArrow},
		{VK_SPACE, Key::Space}
	};

	extern std::unordered_map<char16_t, Key> u16KeyCodeToKey{
		{u'a', Key::A},
		{u'b', Key::B},
		{u'c', Key::C},
		{u'd', Key::D},
		{u'e', Key::E},
		{u'f', Key::F},
		{u'g', Key::G},
		{u'h', Key::H},
		{u'i', Key::I},
		{u'j', Key::J},
		{u'k', Key::K},
		{u'l', Key::L},
		{u'm', Key::M},
		{u'n', Key::N},
		{u'o', Key::O},
		{u'p', Key::P},
		{u'q', Key::Q},
		{u'r', Key::R},
		{u's', Key::S},
		{u't', Key::T},
		{u'u', Key::U},
		{u'v', Key::V},
		{u'w', Key::W},
		{u'x', Key::X},
		{u'y', Key::Y},
		{u'z', Key::Z},
		{u'1', Key::N1},
		{u'2', Key::N2},
		{u'3', Key::N3},
		{u'4', Key::N4},
		{u'5', Key::N5},
		{u'6', Key::N6},
		{u'7', Key::N7},
		{u'8', Key::N8},
		{u'9', Key::N9},
		{u'0', Key::N0}
	};

	extern std::unordered_map<Key, char> keyToChar{
		{Key::A, 'a'},
		{Key::B, 'b'},
		{Key::C, 'c'},
		{Key::D, 'd'},
		{Key::E, 'e'},
		{Key::F, 'f'},
		{Key::G, 'g'},
		{Key::H, 'h'},
		{Key::I, 'i'},
		{Key::J, 'j'},
		{Key::K, 'k'},
		{Key::L, 'l'},
		{Key::M, 'm'},
		{Key::N, 'n'},
		{Key::O, 'o'},
		{Key::P, 'p'},
		{Key::Q, 'q'},
		{Key::R, 'r'},
		{Key::S, 's'},
		{Key::T, 't'},
		{Key::U, 'u'},
		{Key::V, 'v'},
		{Key::W, 'w'},
		{Key::X, 'x'},
		{Key::Y, 'y'},
		{Key::Z, 'z'},
		{Key::N1, '1'},
		{Key::N2, '2'},
		{Key::N3, '3'},
		{Key::N4, '4'},
		{Key::N5, '5'},
		{Key::N6, '6'},
		{Key::N7, '7'},
		{Key::N8, '8'},
		{Key::N9, '9'},
		{Key::N0, '0'}
	};

	extern std::unordered_map<Key, wchar_t> keyToWChar{
		{Key::A, u'a'},
		{Key::B, u'b'},
		{Key::C, u'c'},
		{Key::D, u'd'},
		{Key::E, u'e'},
		{Key::F, u'f'},
		{Key::G, u'g'},
		{Key::H, u'h'},
		{Key::I, u'i'},
		{Key::J, u'j'},
		{Key::K, u'k'},
		{Key::L, u'l'},
		{Key::M, u'm'},
		{Key::N, u'n'},
		{Key::O, u'o'},
		{Key::P, u'p'},
		{Key::Q, u'q'},
		{Key::R, u'r'},
		{Key::S, u's'},
		{Key::T, u't'},
		{Key::U, u'u'},
		{Key::V, u'v'},
		{Key::W, u'w'},
		{Key::X, u'x'},
		{Key::Y, u'y'},
		{Key::Z, u'z'},
		{Key::N1, u'1'},
		{Key::N2, u'2'},
		{Key::N3, u'3'},
		{Key::N4, u'4'},
		{Key::N5, u'5'},
		{Key::N6, u'6'},
		{Key::N7, u'7'},
		{Key::N8, u'8'},
		{Key::N9, u'9'},
		{Key::N0, u'0'}
	};*/
}
#endif