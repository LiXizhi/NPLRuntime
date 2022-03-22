#import <UIKit/UIKit.h>
#include "RenderDeviceAEGL.h"

ParaEngine::RenderDeviceAEGL::RenderDeviceAEGL(EAGLContext* context,CAEAGLLayer* layer)
    :m_Context(context)
    ,m_GLLayer(layer)
    ,m_FrameBuffer(0)
    ,m_ColorBuffer(0)
    ,m_DepthBuffer(0)
{
    // Initialize
    // frame buffer
    glGenFramebuffers(1, &m_FrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FrameBuffer);
    
    // color buffer
    glGenRenderbuffers(1, &m_ColorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ColorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
    [m_Context renderbufferStorage:GL_RENDERBUFFER fromDrawable:m_GLLayer];
    
    GLint backingWidth=0,backingHeight=0;
    
    // depth buffer
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
    
    glGenRenderbuffers(1, &m_DepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);
    
    // check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"failed to make complete frame buffer object %x", status);
        exit(1);
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, m_ColorBuffer);
}

ParaEngine::RenderDeviceAEGL::~RenderDeviceAEGL()
{
}

void ParaEngine::RenderDeviceAEGL::Reset()
{
}

bool ParaEngine::RenderDeviceAEGL::Present()
{
    [m_Context presentRenderbuffer:m_ColorBuffer];
    return true;
}
