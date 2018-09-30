#include "ParaEngine.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "texture/TextureOpenGL.h"
#include "RenderDeviceEGL.h"


#if GLAD_CORE_DEBUG
void _post_call_callback_default(const char *name, void *funcptr, int len_args, ...) {
	GLenum error_code;
	error_code = glad_glGetError();

	if (error_code != GL_NO_ERROR) {
		OUTPUT_LOG("GL ERROR %x in %s\n", error_code, name);
	}
}
#endif


ParaEngine::RenderDeviceEGL::RenderDeviceEGL(EGLDisplay display, EGLSurface surface)
	:m_Display(display)
	,m_Surface(surface)
	, m_FBO(0)
{

#if GLAD_CORE_DEBUG
	glad_set_post_callback(_post_call_callback_default);
#endif

	InitCpas();
	InitFrameBuffer();

	auto file = std::make_shared<CParaFile>(":IDR_FX_DOWNSAMPLE");
	std::string error;
	m_DownSampleEffect = CreateEffect(file->getBuffer(), file->getSize(), nullptr, error);
	if (!m_DownSampleEffect)
	{
		OUTPUT_LOG("load downsample fx failed.\n%s\n", error.c_str());
	}
}

ParaEngine::RenderDeviceEGL::~RenderDeviceEGL()
{
	glDeleteFramebuffers(1, &m_FBO);
}

void ParaEngine::RenderDeviceEGL::Reset(EGLDisplay display, EGLSurface surface)
{
	m_Display = display;
	m_Surface = surface;
}

bool ParaEngine::RenderDeviceEGL::Present()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_DownSampleEffect->SetTechnique(m_DownSampleEffect->GetTechnique(0));
	m_DownSampleEffect->Begin();
	m_DownSampleEffect->SetTexture("tex0", m_CurrentRenderTargets[0]);
	m_DownSampleEffect->BeginPass(0);
	m_DownSampleEffect->CommitChanges();
	DrawQuad();
	m_DownSampleEffect->EndPass();
	m_DownSampleEffect->End();
	
	if (!eglSwapBuffers(m_Display, m_Surface))
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	return true;
}


bool ParaEngine::RenderDeviceEGL::StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect)
{
	std::vector<IParaEngine::ITexture*> oldTargets;
	for (size_t i = 0; i < m_DeviceCpas.NumSimultaneousRTs; i++)
	{
		oldTargets.push_back(m_CurrentRenderTargets[i]);
		SetRenderTarget(i, nullptr);
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


bool ParaEngine::RenderDeviceEGL::SetRenderTarget(uint32_t index, IParaEngine::ITexture* target)
{
	if (index >= 1) return false;
	if (target == nullptr) return false;
	if (target == m_CurrentRenderTargets[index]) return true;
	m_CurrentRenderTargets[index] = target;
	GLuint id = 0;
	if (target != nullptr)
	{
		TextureOpenGL* tex = static_cast<TextureOpenGL*>(target);
		id = tex->GetTextureID();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, id, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		assert(false);
		return false;
	}
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
	return true;
}


bool ParaEngine::RenderDeviceEGL::SetDepthStencil(IParaEngine::ITexture* target)
{
	if (target == m_CurrentDepthStencil) return true;
	m_CurrentDepthStencil = target;

	GLuint id = 0;
	if (target != nullptr)
	{
		TextureOpenGL* tex = static_cast<TextureOpenGL*>(target);
		id = tex->GetTextureID();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, id, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		assert(false);
	}
	return true;
}

void ParaEngine::RenderDeviceEGL::InitFrameBuffer()
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

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void ParaEngine::RenderDeviceEGL::DrawQuad()
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




void ParaEngine::RenderDeviceEGL::InitCpas()
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
	m_DeviceCpas.NPOT = IsSupportExt("GL_OES_texture_npot");
	m_DeviceCpas.BorderClamp = IsSupportExt("GL_OES_texture_border_clamp");
}
