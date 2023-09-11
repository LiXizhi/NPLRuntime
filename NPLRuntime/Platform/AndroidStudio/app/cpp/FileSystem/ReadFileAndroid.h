//-----------------------------------------------------------------------------
// ReadFileAndroid.h
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#pragma once

#include "IFile.h"

#include <string>

struct AAsset;

namespace ParaEngine
{
	class CReadFileAndroid : public IReadFile
	{
	public:
		CReadFileAndroid(const std::string& fileName);
		virtual ~CReadFileAndroid() override;
		virtual uint32_t read(void* buffer, uint32_t sizeToRead) override;
		virtual bool seek(uint32_t finalPos, bool relativeMovement = false) override;
		virtual uint32_t getSize() override;
		virtual bool isOpen() override;
		virtual uint32_t getPos() override;
		virtual const char* getFileName() override;
		virtual void Release() override;
	protected:
		void openFile();
		std::string m_filename;
		AAsset* m_asset;
	};
}
