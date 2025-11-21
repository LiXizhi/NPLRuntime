#include "RenderWindowHarmony.h"

namespace ParaEngine
{
    RenderWindowHarmony::RenderWindowHarmony(int width, int height)
        : m_width(width), m_height(height)
    {
    }

    RenderWindowHarmony::~RenderWindowHarmony()
    {
    }
}

// Global function to create render window
ParaEngine::IRenderWindow* CreateParaRenderWindow(int width, int height)
{
    return new ParaEngine::RenderWindowHarmony(width, height);
}
