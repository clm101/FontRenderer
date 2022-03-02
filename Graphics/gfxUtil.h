#ifndef GFX_UTIL_H
#define GFX_UTIL_H

#ifdef GFX_REFAC
#include <source_location>
#include <string>
#include <format>
#include <stdexcept>
#include <iostream>


#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

namespace clm {
	inline void check_vk_ret(vk::Result result,
							 std::string message,
							 std::source_location location = std::source_location::current())
	{
		if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error{std::format("{}\nFile: {}\nFunction: {}\nLine: {}\n",
											message,
											location.file_name(),
											location.function_name(),
											location.line())};
		}
	}

	inline void check_vk_ret(VkResult result,
							 std::string message,
							 std::source_location location = std::source_location::current())
	{
		check_vk_ret(vk::Result(result), message, location);
	}

	namespace vkutil {
		constexpr uint32_t make_mask(uint32_t shiftAmount, uint32_t width)
		{
			uint32_t block = 0xFFFFFFFF >> (8 * sizeof(uint32_t) - width);
			return (block << shiftAmount);
		}
		struct Masks {
			static const uint32_t variantShiftAmount{29};
			static const uint32_t variantWidth{3};
			static const uint32_t majorShiftAmount{22};
			static const uint32_t majorWidth{7};
			static const uint32_t minorShiftAmount{12};
			static const uint32_t minorWidth{10};
			static const uint32_t patchShiftAmount{0};
			static const uint32_t patchWidth{12};

			static const uint32_t variant{make_mask(variantShiftAmount,
													variantWidth)};
			static const uint32_t major{make_mask(majorShiftAmount,
												  majorWidth)};
			static const uint32_t minor{make_mask(minorShiftAmount,
												  minorWidth)};
			static const uint32_t patch{make_mask(patchShiftAmount,
												  patchWidth)};

			static_assert(0xFFFFFFFF == (variant | major | minor | patch),
						  "Masks do not align");
		};
		// Wanting to make VK_MAKE_API_VERSION into a consteval function
		template<uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch>
		consteval uint32_t MAKE_VERSION()
		{
			static_assert((static_cast<size_t>(variant) << 29) <= static_cast<size_t>(Masks::variant),
						  "Variant value is too high");
			constexpr uint32_t majorMask{0x1FC00000};
			static_assert((static_cast<size_t>(major) << 22) <= static_cast<size_t>(majorMask),
						  "Major value is too high");
			constexpr uint32_t minorMask{0x003FF000};
			static_assert((static_cast<size_t>(minor) << 12) <= static_cast<size_t>(minorMask),
						  "Minor value is too high");
			constexpr uint32_t patchMask{0x00000FFF};
			static_assert(static_cast<size_t>(patch) <= static_cast<size_t>(patchMask),
						  "Patch value is too high");

			uint32_t versionNumber{(variant << 29) | (major << 22) | (minor << 12) | patch};
			return versionNumber;
		}
		std::string MAKE_VERSION_STRING(uint32_t versionNumber)
		{
			uint32_t variant = (versionNumber & Masks::variant) >> Masks::variantShiftAmount;
			uint32_t major = (versionNumber & Masks::major) >> Masks::majorShiftAmount;
			uint32_t minor = (versionNumber & Masks::minor) >> Masks::minorShiftAmount;
			uint32_t patch = (versionNumber & Masks::patch) >> Masks::patchShiftAmount;
			return std::format("{}.{}.{}.{}",
							   variant,
							   major,
							   minor,
							   patch);
		}


		template<typename T, typename U>
		void append_data(std::string& input, const std::string& format, const T& t, const U& u)
		{
			input += std::format(format, t, u);
		}

		template<typename...Ts, size_t...Ns>
		std::string format_fancy_impl(const std::string& title, const std::tuple<Ts...>& ts, std::index_sequence<Ns...>)
		{
			std::string format = "\t{}: {}\n";
			std::string output = title + '\n';
			(append_data(output, format, std::get<2 * Ns>(ts), std::get<2 * Ns + 1>(ts)), ...);
			return output;
		}

		template<typename...Ts>
		std::string format_fancy(std::string title, Ts...ts)
		{
			static_assert(sizeof...(Ts) % 2 == 0,
						  "Odd number of variadic arguments passed, expecting an even amount");
			title += ':';
			std::tuple<Ts...> args{ts...};
			return format_fancy_impl(title, args, std::make_index_sequence<sizeof...(Ts) / 2>());
		}

		template<typename T>
		std::string entry(std::string label, T data)
		{
			return std::format("{}: {}\n", label, data);
		}

		void parse_strings(std::string& inout, const std::string& str)
		{
			if (str.size() == 0)
			{
				return;
			}
			inout += '\t';
			for (size_t i = 0; i < str.size() - 1; i += 1)
			{
				inout.push_back(str[i]);
				if (str[i] == '\n')
				{
					inout.push_back('\t');
				}
			}
			if (str[str.size() - 1] != '\n')
			{
				inout.push_back(str[str.size() - 1]);
			}
			inout.push_back('\n');
		}

		template<typename...Strings>
		std::string table(std::string title, Strings...entries)
		{
			std::string output{};
			(parse_strings(output, entries), ...);
			return title + ":\n" + output;
		}

		std::string translate_physical_device_type(vk::PhysicalDeviceType type)
		{
			if (type == vk::PhysicalDeviceType::eOther)
			{
				return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
			}
			vk::Flags<vk::PhysicalDeviceType> flags{};
			std::string deviceType{};
			if (type & vk::Flags(vk::PhysicalDeviceType::eIntegratedGpu))
			{
				deviceType += "Integrated GPU | ";
			}
			if (type & vk::Flags(vk::PhysicalDeviceType::eDiscreteGpu))
			{
				deviceType += "Discrete GPU | ";
			}
			if (type & vk::Flags(vk::PhysicalDeviceType::eVirtualGpu))
			{
				deviceType += "Virtual GPU | ";
			}
			if (type & vk::Flags(vk::PhysicalDeviceType::eCpu))
			{
				deviceType += "CPU | ";
			}

			return deviceType.substr(0, deviceType.size() - 3);
		}

		std::string list_uuid(const std::vector<uint8_t> uuid)
		{
			std::string output{'['};
			for (size_t i = 0; i < uuid.size() - 1; i += 1)
			{
				output += std::format("{}, ", uuid[i]);
			}
			output += std::format("{}]", *(uuid.rbegin()));
			return output;
		}
	}
}

#endif

#endif