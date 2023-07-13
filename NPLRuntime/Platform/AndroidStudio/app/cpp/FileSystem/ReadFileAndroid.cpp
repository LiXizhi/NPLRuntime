//-----------------------------------------------------------------------------
// ReadFileAndroid.cpp
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ReadFileAndroid.h"
#include "ParaAppAndroid.h"
#include "jni/JniHelper.h"

#include <cstdio>

using namespace ParaEngine;
ParaEngine::CReadFileAndroid::CReadFileAndroid(const std::string& fileName)
	:m_filename(fileName)
	,m_asset(nullptr)
{
	m_filename = fileName;
	openFile();
}

ParaEngine::CReadFileAndroid::~CReadFileAndroid()
{
	Release();
}

uint32_t ParaEngine::CReadFileAndroid::read(void* buffer, uint32_t sizeToRead)
{
	if (!isOpen())return 0;
	return AAsset_read(m_asset, buffer, sizeToRead);
}

bool ParaEngine::CReadFileAndroid::seek(uint32_t finalPos, bool relativeMovement /*= false*/)
{
	if (!isOpen())return false;
	return AAsset_seek(m_asset, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
}

uint32_t ParaEngine::CReadFileAndroid::getSize()
{
	if (!isOpen())return 0;
	return AAsset_getLength(m_asset);
}

bool ParaEngine::CReadFileAndroid::isOpen()
{
	return m_asset != nullptr;
}

uint32_t ParaEngine::CReadFileAndroid::getPos()
{
	if (!isOpen())return 0;
	return getSize() - AAsset_getRemainingLength(m_asset);
}

const char* ParaEngine::CReadFileAndroid::getFileName()
{
	return m_filename.c_str();
}

void ParaEngine::CReadFileAndroid::Release()
{
	if (m_asset)
	{
		AAsset_close(m_asset);
		m_asset = nullptr;
	}
}

void ParaEngine::CReadFileAndroid::openFile()
{
	auto assetManager = JniHelper::getAssetManager();
	m_asset = AAssetManager_open(assetManager, m_filename.c_str(), AASSET_MODE_STREAMING);
}
