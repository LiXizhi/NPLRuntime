
#include "ParaEngine.h"
#include "CCFileUtils.h"

#include "FileUtils.h"

NS_CC_BEGIN

Data FileUtils::getDataFromFile(const std::string& filename)
{


	Data d;

#ifdef WIN32
#else
	ParaEngine::FileData data = ParaEngine::CFileUtils::GetDataFromFile(filename.c_str());
	d.fastSet((unsigned char*)data.GetBytes(),data.GetSize());
#endif


	return d;
}


std::string FileUtils::getStringFromFile(const std::string& filename)
{
	Data data = getDataFromFile(filename);
	if (data.isNull())
		return "";

	std::string ret((const char*)data.getBytes());
	return ret;
}


const std::string FileUtils::fullPathForFilename(const std::string& filename)
{
	//TODO: wangpeng
	return filename;
}


NS_CC_END
