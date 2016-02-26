//-----------------------------------------------------------------------------
// Class:	CAIBase
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedObject.h"
#include "AIBase.h"
#include "BipedController.h"

#include "AIBase.h"

using namespace ParaEngine;


CAIBase::CAIBase(CBipedObject* pBiped)
:m_pBiped(pBiped)
{
}

CAIBase::CAIBase(void)
	:m_pBiped(NULL)
{
}

CAIBase::~CAIBase(void)
{
}

CBipedObject* CAIBase::GetBiped()
{
	return m_pBiped;
}

void CAIBase::SetBiped(CBipedObject* pBiped)
{
	m_pBiped = pBiped;
}
