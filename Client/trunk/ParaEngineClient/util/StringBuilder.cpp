//-----------------------------------------------------------------------------
// Class:	StringBuilderT
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2008.6.16
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "util/StringBuilder.h"
#include "util/StringBuilder.hpp"

namespace ParaEngine
{
	// instantiate class so that no link errors when seperating template implementation to hpp file. 
	template class StringBuilderT< ParaEngine::CNPLPool_Char_allocator >; 
}


#ifdef TEST_ME
void TestMemPool_StringBuilder()
{
	ParaEngine::StringBuilder strBuilder("some string");
	strBuilder = "some other string";
	strBuilder.append("01234567891234");
	strBuilder += "01234567891234";
	strBuilder += "01234567891234";
	strBuilder.clear();
	strBuilder.reserve(511);
	strBuilder.reserve(512);
	strBuilder.resize(255);
	strBuilder.resize(256);

	strBuilder.append(std::string("std::string"));
	strBuilder.append(20);
	strBuilder.append(10.f);
	strBuilder.append('\0');

	OUTPUT_LOG("strBuilder: %s \n", strBuilder.c_str());
}
#endif
