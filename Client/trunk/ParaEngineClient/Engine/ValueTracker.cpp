//-----------------------------------------------------------------------------
// Class:	CValueTracker
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "ValueTracker.h"

#include "memdebug.h"

using namespace ParaEngine;

// TODO: remove this
void test()
{
	CVector3Tracker g_test;
	g_test.Reset();
	g_test.SetSize(2);
	assert(g_test.GetSize() == 2);
	g_test.Push(2, Vector3(1.f,2.f,3.f));
	assert(g_test.GetTime(0) == 2);
	assert(g_test.GetValue(0) == Vector3(1.f,2.f,3.f));

	CIntTracker g_testInt;
	CFloatTracker g_testFloat;
	CStringTracker g_testString;
}
