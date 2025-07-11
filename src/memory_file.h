#pragma once

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

class memory_file
{
public:
	memory_file(uint8_t * data, size_t memory_size);
	memory_file(size_t memory_size);
	memory_file(const std::string & filename);
	memory_file(const std::filesystem::path & filename);

	template<typename T>
	T peek() const
	{
		T result;
		peek_n(reinterpret_cast<uint8_t *>(&result), sizeof(T));
		return result;
	}

	template<typename T>
	T read()
	{
		T result;
		read_n(reinterpret_cast<uint8_t *>(&result), sizeof(T));
		return result;
	}

	template<typename T>
	void read(T & ptr)
	{
		read(&ptr, 1);
	}

	template<typename T>
	void read(T * ptr, size_t count)
	{
		read_n(reinterpret_cast<uint8_t *>(ptr), sizeof(T) * count);
	}

	template<typename T>
	void write(const T & ptr)
	{
		write(&ptr, 1);
	}

	template<typename T>
	void write(const T * ptr, size_t count)
	{
		write_n(reinterpret_cast<const uint8_t *>(ptr), sizeof(T) * count);
	}

	size_t tell()
	{
		return m_data_ptr - m_data_begin;
	}

	void skip(size_t count)
	{
		m_data_ptr += count;
	}

	void set(size_t count)
	{
		m_data_ptr = m_data_begin + count;
	}

	size_t size()
	{
		return m_data_end - m_data_begin;
	}

	bool eof()
	{
		return m_data_ptr == m_data_end;
	}

	uint8_t * ptr()
	{
		return m_data_ptr;
	}

private:
	void peek_n(uint8_t * ptr, size_t count) const
	{
		assert(m_data_ptr + count <= m_data_end);
		std::copy_n(m_data_ptr, count, ptr);
	}

	void read_n(uint8_t * ptr, size_t count)
	{
		peek_n(ptr, count);
		m_data_ptr += count;
	}

	void write_n(const uint8_t * ptr, size_t count)
	{
		assert(m_data_ptr + count <= m_data_end);
		std::copy_n(ptr, count, m_data_ptr);
		m_data_ptr += count;
	}

	std::unique_ptr<uint8_t[]> m_data_storage;
	uint8_t * m_data_begin;
	uint8_t * m_data_ptr;
	uint8_t * m_data_end;
};

inline memory_file::memory_file(uint8_t * data, size_t memory_size)
	: m_data_storage(nullptr)
	, m_data_begin(data)
	, m_data_ptr(data)
	, m_data_end(data + memory_size)
{
}

inline memory_file::memory_file(size_t memory_size)
	: m_data_storage(new uint8_t[memory_size])
	, m_data_begin(m_data_storage.get())
	, m_data_ptr(m_data_storage.get())
	, m_data_end(m_data_storage.get() + memory_size)
{
}

inline memory_file::memory_file(const std::string & filename)
{
#ifdef _MSC_VER
	FILE * file_ptr;
	fopen_s(&file_ptr, filename.c_str(), "rb");
#else
	FILE * file_ptr = fopen(filename.c_str(), "rb");
#endif
	assert(file_ptr != nullptr);
	fseek(file_ptr, 0, SEEK_END);
#ifdef _MSC_VER
	auto fsize = _ftelli64(file_ptr);
#else
	auto fsize = ftell(file_ptr);
#endif
	assert(fsize > 0);
	fseek(file_ptr, 0, SEEK_SET);

	m_data_storage.reset(new uint8_t[fsize]);
	m_data_begin = m_data_storage.get();
	m_data_ptr = m_data_storage.get();
	m_data_end = m_data_storage.get() + fsize;
	fread(m_data_storage.get(), sizeof(uint8_t), fsize, file_ptr);
	fclose(file_ptr);
}

inline memory_file::memory_file(const std::filesystem::path & filename)
{
#ifdef _MSC_VER
	FILE * file_ptr;
	_wfopen_s(&file_ptr, filename.native().c_str(), L"rb");
#else
	FILE * file_ptr = fopen(filename.c_str(), "rb");
#endif
	assert(file_ptr != nullptr);
	fseek(file_ptr, 0, SEEK_END);
#ifdef _MSC_VER
	auto fsize = _ftelli64(file_ptr);
#else
	auto fsize = ftell(file_ptr);
#endif
	assert(fsize > 0);
	fseek(file_ptr, 0, SEEK_SET);

	m_data_storage.reset(new uint8_t[fsize]);
	m_data_begin = m_data_storage.get();
	m_data_ptr = m_data_storage.get();
	m_data_end = m_data_storage.get() + fsize;
	int read_size = fread(m_data_storage.get(), sizeof(uint8_t), fsize, file_ptr);
	assert(read_size == fsize);
	fclose(file_ptr);
}
