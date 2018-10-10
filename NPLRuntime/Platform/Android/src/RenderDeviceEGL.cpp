#include "ParaEngine.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "texture/TextureOpenGL.h"
#include "RenderDeviceEGL.h"
#include "RenderWindowAndroid.h"
#include <EGL/egl.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ParaEngine", __VA_ARGS__))

using namespace  ParaEngine;
#if GLAD_CORE_DEBUG
void _post_call_callback_default(const char *name, void *funcptr, int len_args, ...) {
	GLenum error_code;
	error_code = glad_glGetError();

	if (error_code != GL_NO_ERROR) {
		OUTPUT_LOG("GL ERROR %x in %s\n", error_code, name);
	}
}
#endif


IRenderDevice* IRenderDevice::Create(const RenderConfiguration& cfg)
{

	RenderWindowAndroid* renderWindow = static_cast<RenderWindowAndroid*>(cfg.renderWindow);
	ANativeWindow* nativeWindow = renderWindow->GetNativeWindow();
	const EGLint attribs[] = { EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT, //Request opengl ES2.0
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24,EGL_STENCIL_SIZE,8, EGL_NONE };
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	if (!numConfigs)
	{
		//Fall back to 16bit depth buffer
		const EGLint attribs_d16[] = { EGL_RENDERABLE_TYPE,
			EGL_OPENGL_ES2_BIT, //Request opengl ES2.0
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_NONE };
		eglChooseConfig(display, attribs_d16, &config, 1, &numConfigs);
	}


	if (!numConfigs)
	{
		LOGW("Unable to retrieve EGL config");
		return nullptr;
	}

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
	surface = eglCreateWindowSurface(display, config, nativeWindow, NULL);
	const EGLint context_attrib_list[] = {
		// request a context using Open GL ES 2.0
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	context = eglCreateContext(display, config, NULL, context_attrib_list);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return nullptr;
	}

	if (!loadGL())
	{
		LOGW("Unable to load gl ext.");
	}

	auto version = glGetString(GL_VERSION);
	LOGI("GL_VERSION:%s", version);


	RenderDeviceEGL* pDevice = new RenderDeviceEGL();
	pDevice->m_Surface = surface;
	pDevice->m_Context = context;
	pDevice->m_Display = display;
	
	
	if (!pDevice->Initialize())
	{
		delete pDevice;
		return nullptr;
	}
	eglSwapBuffers(display, surface);
	return pDevice;
}


ParaEngine::RenderDeviceEGL::RenderDeviceEGL()
	:m_Display(EGL_NO_DISPLAY)
	,m_Surface(EGL_NO_SURFACE)
	,m_Context(EGL_NO_CONTEXT)
	,m_FBO(0)
{


}

ParaEngine::RenderDeviceEGL::~RenderDeviceEGL()
{

	if (m_FBO != 0)
	{
		glDeleteFramebuffers(1, &m_FBO);
	}

	if (m_Display != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (m_Context != EGL_NO_CONTEXT)
		{
			eglDestroyContext(m_Display, m_Context);
		}
		if (m_Surface != EGL_NO_SURFACE)
		{
			eglDestroySurface(m_Display, m_Surface);
		}
		eglTerminate(m_Display);
	}

	m_Display = EGL_NO_DISPLAY;
	m_Context = EGL_NO_CONTEXT;
	m_Surface = EGL_NO_SURFACE;
	
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


bool ParaEngine::RenderDeviceEGL::Reset(const RenderConfiguration& cfg)
{
	auto it = std::find(m_Resources.begin(), m_Resources.end(), m_backbufferRenderTarget);
	if (it != m_Resources.end())
	{
		m_Resources.erase(it);
	}
	it = std::find(m_Resources.begin(), m_Resources.end(), m_backbufferDepthStencil);
	if (it != m_Resources.end())
	{
		m_Resources.erase(it);
	}


	m_backbufferRenderTarget->Release();
	m_backbufferDepthStencil->Release();

	return InitFrameBuffer();
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

bool ParaEngine::RenderDeviceEGL::InitFrameBuffer()
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
		return false;
	}

	m_Resources.push_back(m_backbufferDepthStencil);
	m_Resources.push_back(m_backbufferRenderTarget);

	m_CurrentDepthStencil = m_backbufferDepthStencil;

	m_CurrentRenderTargets[0] = m_backbufferRenderTarget;

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	return true;
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




bool ParaEngine::RenderDeviceEGL::Initialize()
{
#if GLAD_CORE_DEBUG
	glad_set_post_callback(_post_call_callback_default);
#endif

	InitCpas();

	m_CurrentRenderTargets = new IParaEngine::ITexture*[m_DeviceCpas.NumSimultaneousRTs];
	memset(m_CurrentRenderTargets, 0, sizeof(IParaEngine::ITexture*) * m_DeviceCpas.NumSimultaneousRTs);





	if (!InitFrameBuffer())
	{
		return false;
	}

	auto file = std::make_shared<CParaFile>(":IDR_FX_DOWNSAMPLE");
	std::string error;
	m_DownSampleEffect = CreateEffect(file->getBuffer(), file->getSize(), nullptr, error);
	if (!m_DownSampleEffect)
	{
		OUTPUT_LOG("load downsample fx failed.\n%s\n", error.c_str());
		return false;
	}

	return true;
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
