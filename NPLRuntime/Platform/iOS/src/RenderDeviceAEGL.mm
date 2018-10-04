#import <UIKit/UIKit.h>
#include "ParaEngine.h"

#include "renderer/VertexDeclarationOpenGL.h"
#include "texture/TextureOpenGL.h"
#include "OpenGL.h"

#include <sstream>
#include <iostream>

#include "RenderDeviceAEGL.h"


#if GLAD_CORE_DEBUG
void _post_call_callback_default(const char *name, void *funcptr, int len_args, ...) {
    GLenum error_code;
    error_code = glad_glGetError();
    
    if (error_code != GL_NO_ERROR) {
        fprintf(stderr, "GL ERROR %d in %s\n", error_code, name);
    }
}
#endif


ParaEngine::RenderDeviceAEGL::RenderDeviceAEGL(EAGLContext* context,CAEAGLLayer* layer)
:m_GLContext(context)
,m_GLLayer(layer)
,m_FrameBuffer(0)
,m_FBO(0)
,m_ColorBuffer(0)
,m_DepthBuffer(0)
{
#if GLAD_CORE_DEBUG
    glad_set_post_callback(_post_call_callback_default);
#endif

    
    // Initialize
    // frame buffer
    glGenFramebuffers(1, &m_FrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FrameBuffer);
    // color buffer
    glGenRenderbuffers(1, &m_ColorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ColorBuffer);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBuffer);
    
    [m_GLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:m_GLLayer];
    
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

	InitCpas();
	InitFrameBuffer();

	auto file = std::make_shared<CParaFile>(":IDR_FX_DOWNSAMPLE");
	std::string error_str;
	m_DownSampleEffect = CreateEffect(file->getBuffer(), file->getSize(), nullptr, error_str);
	if (!m_DownSampleEffect)
	{
		OUTPUT_LOG("load downsample fx failed.\n%s\n", error_str.c_str());
	}


    
}

ParaEngine::RenderDeviceAEGL::~RenderDeviceAEGL()
{
    m_GLContext = nullptr;
    glDeleteFramebuffers(1, &m_FBO);  
}


bool ParaEngine::RenderDeviceAEGL::StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect)
{
	std::vector<IParaEngine::ITexture*> oldTargets;
	for (size_t i = 0; i < m_DeviceCpas.NumSimultaneousRTs; i++)
	{
		oldTargets.push_back(m_CurrentRenderTargets[i]);
        if(i>0)
        {
          SetRenderTarget(i, nullptr);
        }
	}
	SetRenderTarget(0, dest);
	m_DownSampleEffect->SetTechnique(m_DownSampleEffect->GetTechnique(0));
	m_DownSampleEffect->Begin();
	m_DownSampleEffect->SetTexture("tex0", source);
	m_DownSampleEffect->BeginPass(0);
	m_DownSampleEffect->CommitChanges();
	DrawQuad();
	m_DownSampleEffect->EndPass();
	m_DownSampleEffect->End();

	for (size_t i = 0; i < m_DeviceCpas.NumSimultaneousRTs; i++)
	{
		SetRenderTarget(i, oldTargets[i]);
	}
	return true;
}




bool ParaEngine::RenderDeviceAEGL::SetRenderTarget(uint32_t index, IParaEngine::ITexture* target)
{
	if (index >= m_DeviceCpas.NumSimultaneousRTs) return false;
	if (target == m_CurrentRenderTargets[index]) return true;
    if (index == 0 && target == nullptr)
    {
        return false;
    }
	m_CurrentRenderTargets[index] = target;
	GLuint id = 0;
	if (target != nullptr)
	{
		TextureOpenGL* tex = static_cast<TextureOpenGL*>(target);
        id = tex->GetTextureID();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, id, 0);


	if (target != nullptr)
	{
		ParaViewport vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = target->GetWidth();
		vp.Height = target->GetHeight();
		vp.MinZ = 1;
		vp.MaxZ = 0;
		SetViewport(vp);
	}

    GLenum status =glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        OUTPUT_LOG("SetRenderTarget failed! %x",status);
        assert(false);
        return false;
    }
	return true;
}


bool ParaEngine::RenderDeviceAEGL::SetDepthStencil(IParaEngine::ITexture* target)
{
	if (target == m_CurrentDepthStencil) return true;
    if(target == nullptr){
        return false;
    }
	m_CurrentDepthStencil = target;

	
    TextureOpenGL* tex = static_cast<TextureOpenGL*>(target);
	GLuint id = tex->GetTextureID();
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, id, 0);
    GLenum status =glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        OUTPUT_LOG("SetRenderTarget failed! %x",status);
        assert(false);
        return false;
    }
	return true;
}

void ParaEngine::RenderDeviceAEGL::InitFrameBuffer()
{

	auto pWindow = CGlobals::GetRenderWindow();


	m_backbufferRenderTarget = TextureOpenGL::Create(pWindow->GetWidth(), pWindow->GetHeight(), EPixelFormat::A8R8G8B8, ETextureUsage::RenderTarget);
    

	m_backbufferDepthStencil = TextureOpenGL::Create(pWindow->GetWidth(), pWindow->GetHeight(), EPixelFormat::D24S8, ETextureUsage::DepthStencil);

  
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_backbufferRenderTarget->GetTextureID(), 0);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_backbufferDepthStencil->GetTextureID(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_backbufferDepthStencil->GetTextureID(), 0);

	GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		assert(false);
	}

	m_Resources.push_back(m_backbufferDepthStencil);
	m_Resources.push_back(m_backbufferRenderTarget);

	m_CurrentDepthStencil = m_backbufferDepthStencil;


	m_CurrentRenderTargets = new IParaEngine::ITexture*[m_DeviceCpas.NumSimultaneousRTs];
	memset(m_CurrentRenderTargets, 0, sizeof(IParaEngine::ITexture*) * m_DeviceCpas.NumSimultaneousRTs);
	m_CurrentRenderTargets[0] = m_backbufferRenderTarget;

}

void ParaEngine::RenderDeviceAEGL::DrawQuad()
{
	static GLfloat quadVertices[] = {
		-1,-1,0,
		1, -1, 0,
		-1, 1, 0,
		1, 1, 0,
	};

	static VertexElement vertexdesc_pos[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};


	VertexDeclarationPtr pDecl = nullptr;
	if (pDecl == nullptr)
	{
		CreateVertexDeclaration(vertexdesc_pos, &pDecl);
	}

	if (pDecl == nullptr) return;

	SetVertexDeclaration(pDecl);
	DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, quadVertices, sizeof(GLfloat) * 3);
}





void ParaEngine::RenderDeviceAEGL::InitCpas()
{



	m_DeviceCpas.DynamicTextures = true;
    m_DeviceCpas.MRT = false;
	GLint texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

	m_DeviceCpas.MaxSimultaneousTextures = texture_units;
	m_DeviceCpas.ScissorTest = true;
	m_DeviceCpas.Stencil = true;
	m_DeviceCpas.NumSimultaneousRTs = 1;
	m_DeviceCpas.SupportS3TC = IsSupportExt("GL_EXT_texture_compression_s3tc");
	m_DeviceCpas.Texture_R32F = false;
	m_DeviceCpas.Texture_RGBA16F = false;
    m_DeviceCpas.BorderClamp = IsSupportExt("GL_OES_texture_border_clamp");
}



bool ParaEngine::RenderDeviceAEGL::Present()
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	m_DownSampleEffect->SetTechnique(m_DownSampleEffect->GetTechnique(0));
	m_DownSampleEffect->Begin();
	m_DownSampleEffect->SetTexture("tex0", m_CurrentRenderTargets[0]);
	m_DownSampleEffect->BeginPass(0);
	m_DownSampleEffect->CommitChanges();
	DrawQuad();
	m_DownSampleEffect->EndPass();
	m_DownSampleEffect->End();


    [m_GLContext presentRenderbuffer:m_ColorBuffer];
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    return true;
}

