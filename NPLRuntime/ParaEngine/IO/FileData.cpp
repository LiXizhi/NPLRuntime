//-----------------------------------------------------------------------------
// Class: FileFileData
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.8
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileData.h"

#include <string>
using namespace ParaEngine;

const FileData FileData::Null;

FileData::FileData() : m_bytes(nullptr), m_size(0)
{
}

FileData::FileData(FileData&& other) :m_bytes(nullptr), m_size(0)
{
	move(other);
}

FileData::FileData(const FileData& other) : m_bytes(nullptr), m_size(0)
{
	copy(other.m_bytes, other.m_size);
}

FileData::~FileData()
{
	Clear();
}

FileData& FileData::operator= (const FileData& other)
{
	copy(other.m_bytes, other.m_size);
	return *this;
}

FileData& FileData::operator= (FileData&& other)
{
	move(other);
	return *this;
}

void FileData::move(FileData& other)
{
	m_bytes = other.m_bytes;
	m_size = other.m_size;

	other.m_bytes = nullptr;
	other.m_size = 0;
}

bool FileData::isNull() const
{
	return (m_bytes == nullptr || m_size == 0);
}

char* FileData::GetBytes() const
{
	return m_bytes;
}

size_t FileData::GetSize() const
{
	return m_size;
}

void FileData::copy(char* bytes, const size_t size)
{
	Clear();

	if (size > 0)
	{
		m_size = size;
		m_bytes = new char[m_size];
		memcpy(m_bytes, bytes, m_size);
	}
}

void FileData::SetOwnBuffer(char* bytes, const size_t size)
{
	m_bytes = bytes;
	m_size = size;
}

void FileData::Clear()
{
	if (m_bytes)
	{
		delete[] m_bytes;
		m_bytes = nullptr;
		m_size = 0;
	}
}

void ParaEngine::FileData::ReleaseOwnership()
{
	SetOwnBuffer(nullptr, 0);
}

