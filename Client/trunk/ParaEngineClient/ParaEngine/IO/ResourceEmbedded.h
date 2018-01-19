#pragma once
#include "IO/FileUtils.h"

/** @def load and return resource object */
#define LOAD_RESOURCE(RESOURCE) ([]() {                      \
        extern const unsigned char _resource_##RESOURCE[]; extern const size_t _resource_##RESOURCE##_len;  \
        return ParaEngine::Resource((const char*)_resource_##RESOURCE,_resource_##RESOURCE##_len);  \
    })()

/** @def add resource at compile time */
#define ADD_RESOURCE(NAME, FILENAME)   {Resource res = LOAD_RESOURCE(FILENAME); ParaEngine::CFileUtils::AddEmbeddedResource(NAME, res.data(), res.size());}



namespace ParaEngine
{
	class Resource {
	public:
		Resource(const char* start, const size_t len) : resource_data(start), data_len(len) {}

		const char * const &data() const { return resource_data; }
		const size_t &size() const { return data_len; }

		const char *begin() const { return resource_data; }
		const char *end() const { return resource_data + data_len; }

		std::string toString() { return std::string(data(), size()); }

	private:
		const char* resource_data;
		const size_t data_len;
	};


}
class CStaticInitRes
{
public:
	CStaticInitRes();
	static CStaticInitRes& StaticInit();
};
