//-----------------------------------------------------------------------------
// Class:	NPLMemPool
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.15
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLMemPool.h"

#ifdef TEST_ME

void TestMemPool_Char()
{

	ParaEngine::NPLString A;
	A = "01234567891234";
	A += "01234567891234";
	A += "01234567891234";
	A += "01234567891234";
	A.clear();
	A.reserve(511);
	A.reserve(512);
	A.resize(255);
	A.resize(256);

	A[A.size()-1] = 'A';

	ParaEngine::NPLString B;
	B = A;
	//String A("this is some string A");
	//String B("this is some string B");
}

#endif