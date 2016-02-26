//-----------------------------------------------------------------------------
// Class: DynamicAttributeField
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.9, refactored 2015.9.2
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLHelper.h"
#include "DynamicAttributeField.h"

using namespace ParaEngine;

ParaEngine::CDynamicAttributeField::CDynamicAttributeField(const std::string& name, DWORD dwType)
	: CVariable(dwType), m_sName(name)
{
}


ParaEngine::CDynamicAttributeField::CDynamicAttributeField()
{
}

ParaEngine::CDynamicAttributeField::~CDynamicAttributeField()
{
	
}

void ParaEngine::CDynamicAttributeField::operator=(const CVariable& val)
{
	Clone(val);
}
