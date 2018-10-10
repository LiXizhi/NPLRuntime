#include "ParaEngine.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "texture/TextureOpenGL.h"
#include "RenderDeviceWGL.h"
#include "glad/glad.h"
#include "glad/glad_wgl.h"
#include "RenderWindowWin32.h"

#include <sstream>
#include <iostream>

using namespace ParaEngine;



LRESULT CALLBACK TempWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


IRenderDevice* IRenderDevice::Create(const RenderConfiguration& cfg)
{

	HINSTANCE hInstance = GetModuleHandle(NULL);

	// ������ʱ����
	WNDCLASSEX wc;
	memset(&wc,0 ,sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = _T("ParaEngineGL");
	wc.lpfnWndProc = TempWindowProc;
	RegisterClassEx(&wc);
	HWND hWndTemp = CreateWindowExW(0,
		L"ParaEngineGL",      // name of the window class
		L"ParaEngineGL-TempWindow",             // title of the window
		WS_OVERLAPPEDWINDOW,              // window style
		CW_USEDEFAULT,                    // x-position of the window
		CW_USEDEFAULT,                    // y-position of the window
		400,             // width of the window
		300,            // height of the window
		NULL,                             // we have no parent window, NULL
		NULL,                             // we aren't using menus, NULL
		hInstance,                        // application handle
		NULL);                            // used with multiple windows, NULL


										  ///////////////////////
										  //// ������ʱ������

	HDC deviceContext;
	PIXELFORMATDESCRIPTOR pixelFormatDesc;
	int error;

	deviceContext = GetDC(hWndTemp);
	if (!deviceContext)
	{
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}
	// ����һ����ʱ��Ĭ�����ظ�ʽ
	error = SetPixelFormat(deviceContext, 1, &pixelFormatDesc);
	if (error != 1)
	{
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}

	// ����һ����ʱ��Ⱦ������
	HGLRC tempRenderingContext = wglCreateContext(deviceContext);
	if (!tempRenderingContext)
	{
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}

	// ���ղŴ�������ʱ��Ⱦ������,����Ϊ������ڵĵ�ǰ��Ⱦ������.
	error = wglMakeCurrent(deviceContext, tempRenderingContext);
	if (error != 1)
	{
		wglDeleteContext(tempRenderingContext);
		tempRenderingContext = nullptr;
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}

	///////////////////////
	// ����WGL
	///////////////////////

	if (!gladLoadWGL(deviceContext))
	{
		wglDeleteContext(tempRenderingContext);
		tempRenderingContext = nullptr;
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}
	// ������ʱ����
	DestroyWindow(hWndTemp);
	hWndTemp = nullptr;


	RenderWindowWin32* pWindow = (RenderWindowWin32*)cfg.renderWindow;
	HWND hWnd = pWindow->GetHandle();
	deviceContext = GetDC(hWnd);
	if (!deviceContext) {
		return nullptr;
	}

	int attrib[] = { 
		WGL_SUPPORT_OPENGL_ARB,TRUE,
		WGL_DRAW_TO_WINDOW_ARB,TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,24,
		WGL_DEPTH_BITS_ARB,24,
		WGL_STENCIL_BITS_ARB,8,
		WGL_DOUBLE_BUFFER_ARB,TRUE,
		WGL_SWAP_METHOD_ARB,WGL_SWAP_EXCHANGE_ARB,
		WGL_PIXEL_TYPE_ARB,WGL_TYPE_RGBA_ARB,
		0
	};

	int pixelFormat[1];
	uint32_t formatCount = 0;
	// Query for a pixel format that fits the attributes we want.
	int result = wglChoosePixelFormatARB(deviceContext, attrib, NULL, 1, pixelFormat, &formatCount);
	if (!result)
	{
		ReleaseDC(hWnd, deviceContext);
		return nullptr;
	}
	result = SetPixelFormat(deviceContext, pixelFormat[0], &pixelFormatDesc);
	if (!result)
	{
		ReleaseDC(hWnd, deviceContext);
		return nullptr;
	}

	int contextAttriutes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB,2,
		WGL_CONTEXT_MINOR_VERSION_ARB,0,
		0
	};

	// Create a OpenGL 2.0 rendering context.
	HGLRC renderingContext = wglCreateContextAttribsARB(deviceContext, 0, contextAttriutes);
	if (!renderingContext) {
		return nullptr;
	}
	result = wglMakeCurrent(deviceContext, renderingContext);
	if (!result)
	{
		ReleaseDC(hWnd, deviceContext);
		wglDeleteContext(renderingContext);
		return nullptr;
	}

	if (!gladLoadGL())
	{
		ReleaseDC(hWnd, deviceContext);
		wglDeleteContext(renderingContext);
		return nullptr;
	}

	RenderDeviceOpenWGL* renderDevice = new RenderDeviceOpenWGL();
	
	renderDevice->m_GLRenderingContext = renderingContext;
	renderDevice->m_DeviceContext = deviceContext;

	if (!renderDevice->Initialize())
	{
		ReleaseDC(hWnd, deviceContext);
		wglDeleteContext(renderingContext);
		delete renderDevice;
		return nullptr;
	}

	return renderDevice;
}

bool ParaEngine::RenderDeviceOpenWGL::Reset(const RenderConfiguration& cfg)
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

	delete m_backbufferRenderTarget;
	delete m_backbufferDepthStencil;

	return InitFrameBuffer();
}



#if GLAD_CORE_DEBUG
void _post_call_callback_default(const char *name, void *funcptr, int len_args, ...) {
	GLenum error_code;
	error_code = glad_glGetError();

	if (error_code != GL_NO_ERROR) {
		fprintf(stderr, "GL ERROR %d in %s\n", error_code, name);
	}
}
#endif

ParaEngine::RenderDeviceOpenWGL::RenderDeviceOpenWGL()
	:m_DeviceContext(NULL)
	,m_GLRenderingContext(NULL)
	,m_FBO(0)

{

}

ParaEngine::RenderDeviceOpenWGL::~RenderDeviceOpenWGL()
{

	glDeleteFramebuffers(1, &m_FBO);
}


bool ParaEngine::RenderDeviceOpenWGL::StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect)
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




bool ParaEngine::RenderDeviceOpenWGL::SetRenderTarget(uint32_t index, IParaEngine::ITexture* target)
{
	if (index >= m_DeviceCpas.NumSimultaneousRTs) return false;
	if (target == m_CurrentRenderTargets[index]) return true;
	m_CurrentRenderTargets[index] = target;
	static GLenum* drawBufers = nullptr;
	if (drawBufers == nullptr)
	{
		drawBufers = new GLenum[m_DeviceCpas.NumSimultaneousRTs];
		for (size_t i = 0; i < m_DeviceCpas.NumSimultaneousRTs; i++)
		{
			drawBufers[i] = GL_NONE;
		}
		drawBufers[0] = GL_COLOR_ATTACHMENT0;
	}
	GLuint id = 0;
	if (target != nullptr)
	{
		TextureOpenGL* tex = static_cast<TextureOpenGL*>(target);
		id = tex->GetTextureID();
		drawBufers[index] = GL_COLOR_ATTACHMENT0 + index;
	}
	else {
		drawBufers[index] = GL_NONE;
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

	glDrawBuffers(m_DeviceCpas.NumSimultaneousRTs, drawBufers);
	return true;
}


bool ParaEngine::RenderDeviceOpenWGL::SetDepthStencil(IParaEngine::ITexture* target)
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
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, id, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		assert(false);
	}
	return true;
}

bool ParaEngine::RenderDeviceOpenWGL::InitFrameBuffer()
{

	auto pWindow = CGlobals::GetRenderWindow();


	m_backbufferRenderTarget = TextureOpenGL::Create(pWindow->GetWidth(), pWindow->GetHeight(), EPixelFormat::A8R8G8B8, ETextureUsage::RenderTarget);
	m_backbufferDepthStencil = TextureOpenGL::Create(pWindow->GetWidth(), pWindow->GetHeight(), EPixelFormat::D24S8, ETextureUsage::DepthStencil);


	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_backbufferRenderTarget->GetTextureID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_backbufferDepthStencil->GetTextureID(), 0);

	GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		assert(false);
		return false;
	}

	m_Resources.push_back(m_backbufferDepthStencil);
	m_Resources.push_back(m_backbufferRenderTarget);

	m_CurrentDepthStencil = m_backbufferDepthStencil;
	m_CurrentRenderTargets[0] = m_backbufferRenderTarget;

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	return true;
}

void ParaEngine::RenderDeviceOpenWGL::DrawQuad()
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





bool ParaEngine::RenderDeviceOpenWGL::Initialize()
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

void ParaEngine::RenderDeviceOpenWGL::InitCpas()
{



	m_DeviceCpas.DynamicTextures = true;
	GLint maxDrawBuffers = 0;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

	if (maxDrawBuffers > 1)
	{
		m_DeviceCpas.MRT = true;
	}

	GLint texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

	m_DeviceCpas.MaxSimultaneousTextures = texture_units;
	m_DeviceCpas.ScissorTest = true;
	m_DeviceCpas.Stencil = true;
	m_DeviceCpas.NumSimultaneousRTs = maxDrawBuffers;
	m_DeviceCpas.SupportS3TC = IsSupportExt("GL_EXT_texture_compression_s3tc");
	m_DeviceCpas.Texture_R32F = true;
	m_DeviceCpas.Texture_RGBA16F = true;
	m_DeviceCpas.BorderClamp = IsSupportExt("GL_ARB_texture_border_clamp");
}



bool ParaEngine::RenderDeviceOpenWGL::Present()
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
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	bool ret = SwapBuffers(m_DeviceContext);
	return ret;
}

