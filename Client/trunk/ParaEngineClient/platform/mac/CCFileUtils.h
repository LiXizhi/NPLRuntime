


#ifndef __CC_FILE_UTILS_H__
#define __CC_FILE_UTILS_H__


#include "CCType.h"




NS_CC_BEGIN



class FileUtils
{
public:
	static Data getDataFromFile(const std::string& filename);

	static std::string getStringFromFile(const std::string& filename);

	static const std::string fullPathForFilename(const std::string& filename);

};


NS_CC_END

#endif
