#pragma once
#include "RenderDeviceOpenGL.h"

@class EAGLContext;
@class CAEAGLLayer;

namespace ParaEngine
{
	class RenderDeviceAEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceAEGL(EAGLContext* context,CAEAGLLayer* layer);
		~RenderDeviceAEGL();
		void Reset();
		virtual bool Present() override;
    private:
        EAGLContext* m_Context;
        CAEAGLLayer* m_GLLayer;
        GLuint              m_FrameBuffer;
        GLuint              m_ColorBuffer;
        GLuint              m_DepthBuffer;
	};
}
