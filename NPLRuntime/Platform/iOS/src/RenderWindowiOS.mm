#include "ParaEngine.h"
#include "RenderWindowiOS.h"

using namespace ParaEngine;

IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	return new RenderWindowiOS();
}

ParaEngine::RenderWindowiOS::RenderWindowiOS()
{

}

ParaEngine::RenderWindowiOS::~RenderWindowiOS()
{

}

unsigned int ParaEngine::RenderWindowiOS::GetWidth() const
{
    return 0;
}

unsigned int ParaEngine::RenderWindowiOS::GetHeight() const
{
    return 0;
}

intptr_t ParaEngine::RenderWindowiOS::GetNativeHandle() const
{
    return 0;
}

