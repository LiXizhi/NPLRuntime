#include "ParaEngine.h"
#include "StringHelper.h"
#include "VirtualKey.h"

using namespace ParaEngine;


std::string ParaEngine::DeviceMouseEvent::ToString()
{
	return "MouseEvent";
}

std::string ParaEngine::DeviceMouseMoveEvent::ToString()
{
	char tmp[256] = "\0";
	StringHelper::fast_sprintf(tmp, "MouseMove x=%d y=%d", m_x, m_y);
	return tmp;
}

std::string ParaEngine::DeviceMouseButtonEvent::ToString()
{
	char tmp[256] = "\0";
	StringHelper::fast_sprintf(tmp, "MouseBtn %s btn %s x=%d y=%d", GetButton() == EMouseButton::LEFT ? "left" : (GetButton() == EMouseButton::RIGHT ? "right" : "unknown"), 
		GetKeyState() == EKeyState::PRESS ? "down" : "up",
		m_x, m_y);
	return tmp;
}

std::string ParaEngine::DeviceMouseWheelEvent::ToString()
{
	return std::string("MouseWheel");
}
