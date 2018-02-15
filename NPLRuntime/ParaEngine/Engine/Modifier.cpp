//-----------------------------------------------------------------------------
// Class:	CModifier
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.8.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "Modifier.h"

using namespace ParaEngine;


CModifier::CModifier(void)
{
}

CModifier::~CModifier(void)
{
}

void CModifier::DeleteThis()
{
	delete this;
}
