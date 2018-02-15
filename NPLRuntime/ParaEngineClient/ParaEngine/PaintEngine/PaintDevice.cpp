//-----------------------------------------------------------------------------
// Class: PaintDevice
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.23
// Desc: I have referred to QT framework's qpaintdevice.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PaintDevice.h"

using namespace ParaEngine;

ParaEngine::CPaintDevice::CPaintDevice()
	: painters(0)
{

}

ParaEngine::CPaintDevice::~CPaintDevice()
{
	if (paintingActive())
	{
		OUTPUT_LOG("warning: Cannot destroy paint device that is being painted\n");
	}
}

int ParaEngine::CPaintDevice::metric(PaintDeviceMetric metric) const
{
	return 0;
}

void ParaEngine::CPaintDevice::initPainter(CPainter *painter) const
{

}

CPainter * ParaEngine::CPaintDevice::sharedPainter() const
{
	return 0;
}

int ParaEngine::CPaintDevice::devType() const
{
	return 0;
}
