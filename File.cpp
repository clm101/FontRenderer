#include "File.h"

namespace clm {
	File::File(const std::string& fileName, Endian fileEndian, Endian requestEndian)
		:
		File()
	{
		m_fileEndian = fileEndian;
		m_requestEndian = requestEndian;
		m_fileName = fileName;

		HANDLE fileHandle = CreateFileA(fileName.c_str(),
										GENERIC_READ,
										FILE_SHARE_READ,
										NULL,
										OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,
										NULL);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			throw GetLastError();
		}

		LARGE_INTEGER fileSizeTmp{};
		BOOL ret = GetFileSizeEx(fileHandle, &fileSizeTmp);
		if (ret == 0)
		{
			CloseHandle(fileHandle);
			throw GetLastError();
		}
		std::uint64_t fileSize = static_cast<std::uint64_t>(fileSizeTmp.QuadPart);
		m_fileBuffer.resize(fileSize);
		DWORD bytesRead{};

		if (ReadFile(fileHandle,
					 m_fileBuffer.data(),
					 static_cast<DWORD>(fileSize),
					 &bytesRead,
					 NULL) == 0 || static_cast<uint64_t>(bytesRead) != fileSize)
		{
			CloseHandle(fileHandle);
			throw GetLastError();
		}

		CloseHandle(fileHandle);
	}

	File File::open_file(const std::string& fileName, const Endian fileEndian, const Endian requestEndian)
	{
		File file{ fileName, fileEndian, requestEndian };
		return file;
	}

	size_t File::size() const noexcept { return m_fileBuffer.size(); }

	void File::set_position(size_t fileOffset) const noexcept(util::release)
	{
#ifdef _DEBUG
		if (fileOffset >= m_fileBuffer.size())
		{
			throw std::out_of_range{ "Attempting to set fileOffset beyond fileBuffer bounds" };
		}
		m_offset = fileOffset;
#else
		m_offset = fileOffset >= m_fileBuffer.size() ? 0 : fileOffset;
#endif
	}

	size_t File::get_position() const noexcept { return m_offset; }

	const File& operator>>(const File& file, std::string& dest)
	{
		file.get_data(dest.data(), dest.size());
		return file;
	}

	const File& operator<<(std::string& dest, const File& file)
	{
		return file >> dest;
	}
}