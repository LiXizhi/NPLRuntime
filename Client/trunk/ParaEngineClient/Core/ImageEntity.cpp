//-----------------------------------------------------------------------------
// Class:	ImageEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.3.15
// Desc: Some of the code is based on cocos2d CCImage.cpp
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_OPENGL_RENDERER
#include "platform/OpenGLWrapper.h"
USING_NS_CC;
#endif

#include "ImageEntity.h"

using namespace ParaEngine;

ParaEngine::ImageEntity::ImageEntity()
	: _data(nullptr)
	, _dataLen(0)
	, _width(0)
	, _height(0)
	, _unpack(false)
	, m_bIsOwnData(true)
	, _fileType(0)
	, _renderFormat(D3DFMT_UNKNOWN)
	, _hasPremultipliedAlpha(false)
	, _numberOfMipmaps(0)
{
	m_bIsValid = false;
}

ParaEngine::ImageEntity::~ImageEntity()
{
	if (m_bIsOwnData){
		SAFE_DELETE_ARRAY(_data);
	}
}

bool ParaEngine::ImageEntity::LoadFromFile(const std::string& path)
{
	return false;
}

bool ParaEngine::ImageEntity::LoadFromMemory(const unsigned char * data, size_t dataLen, bool bOwnData)
{
	bool ret = false;
	if (bOwnData)
	{
		_dataLen = dataLen;
		_data = new (std::nothrow) unsigned char[_dataLen];
		if (_data)
		{
			memcpy(_data, data, _dataLen);
			m_bIsOwnData = true;
			m_bIsValid = true;
			ret = true;
		}
	}
	else
	{
		_dataLen = dataLen;
		_data = (unsigned char*)data;
		m_bIsOwnData = false;
		m_bIsValid = true;
		ret = true;
	}
	return ret;
}

bool ParaEngine::ImageEntity::LoadFromRawData(const unsigned char * data, size_t dataLen, int width, int height, int bitsPerComponent, bool preMulti /*= false*/)
{
	bool ret = false;
	do
	{
		if(0 == width || 0 == height)
			break;

		_height = height;
		_width = width;
		_hasPremultipliedAlpha = preMulti;
		_renderFormat = D3DFMT_A8R8G8B8;

		// only RGBA8888 supported
		int bytesPerComponent = 4;
		_dataLen = height * width * bytesPerComponent;
		_data = new (std::nothrow) unsigned char[_dataLen];
		if(!_data)
			break;
		memcpy(_data, data, _dataLen);

		ret = true;
		m_bIsValid = true;
	} while (0);

	return ret;
}

bool ParaEngine::ImageEntity::SaveToFile(const std::string &filename, bool isToRGB /*= true*/)
{
#ifdef USE_OPENGL_RENDERER
	std::string filepath = CParaFile::GetWritablePath() + filename;
	CParaFile::CreateDirectory(filepath.c_str());
	Image image;
	image.initWithRawData(getData(), getDataLen(), getWidth(), getHeight(), getBitPerPixel());
	bool res =image.saveToFile(filepath, isToRGB);
	if (res){
		OUTPUT_LOG("successfully saved image to file :%s\n", filepath.c_str());
	}
	else
	{
		OUTPUT_LOG("failed to save image to file :%s\n", filepath.c_str());
	}
	return res;
#else
	return false;
#endif
}

int ParaEngine::ImageEntity::getBitPerPixel()
{
	if (getRenderFormat() == D3DFMT_A8R8G8B8)
		return 32;
	else
		return 0;
}

bool ParaEngine::ImageEntity::hasAlpha()
{
	return false;
}

bool ParaEngine::ImageEntity::isCompressed()
{
	return false;
}
