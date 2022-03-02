#ifndef FILE_H
#define FILE_H
#include <string>
#include <vector>
#include <exception>
#include <format>
#include <array>
#include <cstddef>
#include <concepts>

#include <clmUtil/clm_util.h>
#include <clmUtil/clm_system.h>
#include <clmUtil/clm_concepts_ext.h>
#include <clmUtil/clm_err.h>

using std::byte;

namespace clm {
	enum class Endian {
		Big, Little
	};

	template<std::integral T>
	constexpr void change_endian(T& t)
	{
		if constexpr (sizeof(T) != 1)
		{
			T tmp = 0;
			constexpr const size_t iter = sizeof(T) >> 1;
			size_t i = 0;
			while (i < iter)
			{
				const size_t byteShift = sizeof(T) - 2 * i - 1;
				T upperToLower = static_cast<T>((static_cast<size_t>(t) >> (8 * byteShift)) & (0xFFllu << (8 * i)));
				T lowerToUpper = static_cast<T>((static_cast<size_t>(t) << (8 * byteShift)) & (0xFFllu << (8 * (sizeof(T) - i - 1))));
				tmp |= (upperToLower | lowerToUpper);
				++i;
			}
			t = tmp;
		}
	}

	class File {
	public:
		File() noexcept = default;
		File(const std::string&, Endian = Endian::Little, Endian = Endian::Little);
		~File() noexcept = default;
		File(const File&) noexcept = default;
		File(File&&) noexcept = default;
		File& operator=(const File&) noexcept = default;
		File& operator=(File&&) noexcept = default;
		static File open_file(const std::string&, const Endian = Endian::Little, const Endian = Endian::Little);
		size_t size() const noexcept;

		template<typename T>
		void get_data_raw(T& dest, size_t customFileOffset = 0) const noexcept(util::release)
		{
			err::assert<std::runtime_error>((customFileOffset + sizeof(T)) <= m_fileBuffer.size(), "Read request goes beyond file buffer.");
			size_t pos = (customFileOffset == 0 ? m_offset : customFileOffset);
			T data = *reinterpret_cast<const T*>(m_fileBuffer.data() + pos);
			m_offset += (customFileOffset == 0 ? sizeof(T) : customFileOffset);

			dest = data;
		}
		template<typename T>
		void get_data_raw(T* dest, size_t count = 1, size_t customFileOffset = 0) const noexcept(util::release)
		{
			err::assert<std::runtime_error>((customFileOffset + count * sizeof(T)) <= m_fileBuffer.size(), "Read request goes beyond file buffer.");
			for (size_t i = 0; i < count; i++)
			{
				get_data_raw(*dest, customFileOffset);
				dest++;
			}
		}
		template<typename T>
		void get_data(T& dest, size_t customFileOffset = 0) const noexcept(util::release)
		{
			get_data_raw(dest, customFileOffset);
			if (m_fileEndian != m_requestEndian)
			{
				change_endian(dest);
			}
		}
		template<typename T>
		void get_data(T* dest, size_t count = 1, size_t customFileOffset = 0) const noexcept(util::release)
		{
			err::assert<std::runtime_error>((customFileOffset + count * sizeof(T)) <= m_fileBuffer.size(), "Read request goes beyond file buffer.");
			for (size_t i = 0; i < count; i++)
			{
				get_data(*dest, customFileOffset);
				dest++;
			}
		}
		template<typename Src, typename Dest> requires util::not_same_as<Src, Dest>
		void fill_vec(std::vector<Dest>& dest, size_t customFileOffset = 0) const noexcept(util::release)
		{
			err::assert<std::runtime_error>((customFileOffset + sizeof(Src) * dest.size()) <= m_fileBuffer.size(), "Read request goes beyond file buffer.");
			for (auto& elem : dest)
			{
				Src tmp{};
				get_data(tmp, customFileOffset);
				elem = static_cast<Dest>(tmp);
				customFileOffset += (customFileOffset == 0 ? 0 : sizeof(Src));
			}
		}
		template<typename T>
		void fill_vec(std::vector<T>& dest, size_t customFileOffset = 0) const noexcept(util::release)
		{
			err::assert<std::runtime_error>((customFileOffset + sizeof(T) * dest.size()) <= m_fileBuffer.size(), "Read request goes beyond file buffer.");
			for (auto& elem : dest)
			{
				get_data(elem, customFileOffset);
				customFileOffset += (customFileOffset == 0 ? 0 : sizeof(T));
			}
		}
		void set_position(size_t) const noexcept(util::release);
		size_t get_position() const noexcept;
	private:
		Endian m_fileEndian = Endian::Little;
		Endian m_requestEndian = Endian::Little;
		std::string m_fileName;
		std::vector<byte> m_fileBuffer;
		mutable size_t m_offset;
	};

	template <std::integral T>
	const File& operator>>(const File& file, T& dest)
	{
		file.get_data(dest);
		return file;
	}

	template <std::integral T>
	const File& operator<<(T& dest, const File& file)
	{
		return file >> dest;
	}

	const File& operator>>(const File& file, std::string& dest);

	const File& operator<<(std::string& dest, const File& file);

	template<std::integral T>
	const File& operator>>(const File& file, std::vector<T>& dest)
	{
		file.fill_vec<T>(dest);
		return file;
	}

	template<std::integral T>
	const File& operator<<(std::vector<T>& dest, const File& file)
	{
		return file >> dest;
	}
}


#endif