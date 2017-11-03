#include "ParaEngine.h"
#include "platform/OpenGLWrapper.h"
#include "platform/win32/ParaEngineGLView.h"
#include "platform/win32/ParaEngineApp.h"

#include "2dengine/GUIDirectInput.h"
#include "2dengine/GUIRoot.h"

#include "2dengine/GUIIME.h"

namespace ParaEngine {
	class GLFWEventHandler
	{
	public:
		static void onGLFWError(int errorID, const char* errorDesc)
		{
			if (_view)
				_view->onGLFWError(errorID, errorDesc);
		}

		static void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify)
		{
			if (_view)
				_view->onGLFWMouseCallBack(window, button, action, modify);
		}

		static void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
		{
			if (_view)
				_view->onGLFWMouseMoveCallBack(window, x, y);
		}

		static void onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y)
		{
			if (_view)
				_view->onGLFWMouseScrollCallback(window, x, y);
		}

		static void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (_view)
				_view->onGLFWKeyCallback(window, key, scancode, action, mods);
		}

		static void onGLFWCharCallback(GLFWwindow* window, unsigned int character)
		{
			if (_view)
				_view->onGLFWCharCallback(window, character);
		}

		static void onGLFWWindowPosCallback(GLFWwindow* windows, int x, int y)
		{
			if (_view)
				_view->onGLFWWindowPosCallback(windows, x, y);
		}

		static void onGLFWframebuffersize(GLFWwindow* window, int w, int h)
		{
			if (_view)
				_view->onGLFWframebuffersize(window, w, h);
		}

		static void onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height)
		{
			if (_view)
				_view->onGLFWWindowSizeFunCallback(window, width, height);
		}

		static void setGLViewImpl(CParaEngineGLView* view)
		{
			_view = view;
		}

		static void onGLFWWindowIconifyCallback(GLFWwindow* window, int iconified)
		{
			if (_view)
			{
				_view->onGLFWWindowIconifyCallback(window, iconified);
			}
		}

	private:
		static CParaEngineGLView* _view;
	};

	CParaEngineGLView* GLFWEventHandler::_view = nullptr;

	// set OpenGL context attributes: red,green,blue,alpha,depth,stencil
	GLContextAttrs CParaEngineGLView::_glContextAttrs = {8, 8, 8, 8, 24, 8};


	CParaEngineGLView::CParaEngineGLView()
		: _captured(false)
		, _supportTouch(false)
		, _isInRetinaMonitor(false)
		, _isRetinaEnabled(false)
		, _retinaFactor(1)
		, _frameZoomFactor(1.0f)
		, _mainWindow(nullptr)
		, _monitor(nullptr)
		, _mouseX(0.0f)
		, _mouseY(0.0f)
		, _scaleX(1.0f)
		, _scaleY(1.0f)
	{
		_viewName = "ParaEngine";

		/*
		g_keyCodeMap.clear();
		for (auto& item : g_keyCodeStructArray)
		{
			g_keyCodeMap[item.glfwKeyCode] = item.keyCode;
		}
		*/

		GLFWEventHandler::setGLViewImpl(this);

		glfwSetErrorCallback(GLFWEventHandler::onGLFWError);
		glfwInit();
	}

	CParaEngineGLView::~CParaEngineGLView()
	{
		OUTPUT_LOG("deallocing CParaEngineGLView: %p", this);

		GLFWEventHandler::setGLViewImpl(nullptr);
		glfwTerminate();
	}

	CParaEngineGLView* CParaEngineGLView::create(const std::string& viewName)
	{
		auto ret = new (std::nothrow) CParaEngineGLView;
		if (ret && ret->initWithRect(viewName, Rect(0, 0, 960, 640), 1)) {
			return ret;
		}

		return nullptr;
	}

	CParaEngineGLView* CParaEngineGLView::createWithRect(const std::string& viewName, const Rect& rect, float frameZoomFactor)
	{
		auto ret = new (std::nothrow) CParaEngineGLView;
		if (ret && ret->initWithRect(viewName, rect, frameZoomFactor)) {
			return ret;
		}

		return nullptr;
	}

	CParaEngineGLView* CParaEngineGLView::createWithFullScreen(const std::string& viewName)
	{
		auto ret = new (std::nothrow) CParaEngineGLView();
		if (ret && ret->initWithFullScreen(viewName)) {
			return ret;
		}

		return nullptr;
	}

	CParaEngineGLView* CParaEngineGLView::createWithFullScreen(const std::string& viewName, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
	{
		auto ret = new (std::nothrow) CParaEngineGLView();
		if (ret && ret->initWithFullscreen(viewName, videoMode, monitor)) {
			return ret;
		}

		return nullptr;
	}

	bool CParaEngineGLView::initWithRect(const std::string& viewName, const Rect& rect, float frameZoomFactor)
	{
		setViewName(viewName);

		_frameZoomFactor = frameZoomFactor;

		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		glfwWindowHint(GLFW_RED_BITS, _glContextAttrs.redBits);
		glfwWindowHint(GLFW_GREEN_BITS, _glContextAttrs.greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, _glContextAttrs.blueBits);
		glfwWindowHint(GLFW_ALPHA_BITS, _glContextAttrs.alphaBits);
		glfwWindowHint(GLFW_DEPTH_BITS, _glContextAttrs.depthBits);
		glfwWindowHint(GLFW_STENCIL_BITS, _glContextAttrs.stencilBits);

		_mainWindow = glfwCreateWindow((int)(rect.size.width * _frameZoomFactor),
			(int)(rect.size.height * _frameZoomFactor),
			_viewName.c_str(),
			_monitor,
			nullptr);
		glfwMakeContextCurrent(_mainWindow);

		glfwSetMouseButtonCallback(_mainWindow, GLFWEventHandler::onGLFWMouseCallBack);
		glfwSetCursorPosCallback(_mainWindow, GLFWEventHandler::onGLFWMouseMoveCallBack);
		glfwSetScrollCallback(_mainWindow, GLFWEventHandler::onGLFWMouseScrollCallback);
		glfwSetCharCallback(_mainWindow, GLFWEventHandler::onGLFWCharCallback);
		glfwSetKeyCallback(_mainWindow, GLFWEventHandler::onGLFWKeyCallback);
		glfwSetWindowPosCallback(_mainWindow, GLFWEventHandler::onGLFWWindowPosCallback);
		glfwSetFramebufferSizeCallback(_mainWindow, GLFWEventHandler::onGLFWframebuffersize);
		glfwSetWindowSizeCallback(_mainWindow, GLFWEventHandler::onGLFWWindowSizeFunCallback);
		glfwSetWindowIconifyCallback(_mainWindow, GLFWEventHandler::onGLFWWindowIconifyCallback);

		setFrameSize(rect.size.width, rect.size.height);

		// check OpenGL version at first
		const GLubyte* glVersion = glGetString(GL_VERSION);

		if (atof((const char*)glVersion) < 1.5)
		{
			char strComplain[256] = { 0 };
			sprintf(strComplain,
				"OpenGL 1.5 or higher is required (your version is %s). Please upgrade the driver of your video card.",
				glVersion);
			MessageBoxA(NULL, strComplain, "OpenGL version too old", MB_OK);
			return false;
		}

		initGlew();

		// Enable point size by default.
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		return true;
	}

	bool CParaEngineGLView::initWithFullScreen(const std::string& viewName)
	{
		//Create fullscreen window on primary monitor at its current video mode.
		_monitor = glfwGetPrimaryMonitor();
		if (nullptr == _monitor)
			return false;

		const GLFWvidmode* videoMode = glfwGetVideoMode(_monitor);
		return initWithRect(viewName, Rect(0, 0, (float)videoMode->width, (float)videoMode->height), 1.0f);
	}

	bool CParaEngineGLView::initWithFullscreen(const std::string &viewname, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
	{
		//Create fullscreen on specified monitor at the specified video mode.
		_monitor = monitor;
		if (nullptr == _monitor)
			return false;

		//These are soft contraints. If the video mode is retrieved at runtime, the resulting window and context should match these exactly. If invalid attribs are passed (eg. from an outdated cache), window creation will NOT fail but the actual window/context may differ.
		glfwWindowHint(GLFW_REFRESH_RATE, videoMode.refreshRate);
		glfwWindowHint(GLFW_RED_BITS, videoMode.redBits);
		glfwWindowHint(GLFW_BLUE_BITS, videoMode.blueBits);
		glfwWindowHint(GLFW_GREEN_BITS, videoMode.greenBits);

		return initWithRect(viewname, Rect(0, 0, (float)videoMode.width, (float)videoMode.height), 1.0f);
	}

	bool CParaEngineGLView::isOpenGLReady()
	{
		return nullptr != _mainWindow;
	}

	void CParaEngineGLView::end()
	{
		if (_mainWindow)
		{
			glfwSetWindowShouldClose(_mainWindow, 1);
			_mainWindow = nullptr;
		}
	}

	void CParaEngineGLView::swapBuffers()
	{
		if (_mainWindow)
			glfwSwapBuffers(_mainWindow);
	}

	bool CParaEngineGLView::windowShouldClose()
	{
		if (_mainWindow)
			return glfwWindowShouldClose(_mainWindow) ? true : false;
		else
			return true;
	}

	void CParaEngineGLView::pollEvents()
	{
		glfwPollEvents();
	}

	void CParaEngineGLView::enableRetina(bool enabled)
	{
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
//		_isRetinaEnabled = enabled;
//		if (_isRetinaEnabled)
//		{
//			_retinaFactor = 1;
//		}
//		else
//		{
//			_retinaFactor = 2;
//		}
//		updateFrameSize();
//#endif
	}

	void CParaEngineGLView::setIMEKeyboardState(bool /*bOpen*/)
	{

	}

	void CParaEngineGLView::setCursorVisible(bool isVisible)
	{
		if (_mainWindow == NULL)
			return;

		if (isVisible)
			glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}

	void CParaEngineGLView::setFrameZoomFactor(float zoomFactor)
	{
		assert(zoomFactor > 0.0f/*, "zoomFactor must be larger than 0"*/);

		if (fabs(_frameZoomFactor - zoomFactor) < FLT_EPSILON)
		{
			return;
		}

		_frameZoomFactor = zoomFactor;
		updateFrameSize();
	}

	float CParaEngineGLView::getFrameZoomFactor() const
	{
		return _frameZoomFactor;
	}

	const Size& CParaEngineGLView::getFrameSize() const
	{
		return _screenSize;
	}

	void CParaEngineGLView::setFrameSize(float width, float height)
	{
		_designResolutionSize = _screenSize = Size(width, height);
		updateFrameSize();
	}


	void CParaEngineGLView::updateFrameSize()
	{
		if (_screenSize.width > 0 && _screenSize.height > 0)
		{
			int w = 0, h = 0;
			glfwGetWindowSize(_mainWindow, &w, &h);

			int frameBufferW = 0, frameBufferH = 0;
			glfwGetFramebufferSize(_mainWindow, &frameBufferW, &frameBufferH);

			if (frameBufferW == 2 * w && frameBufferH == 2 * h)
			{
				if (_isRetinaEnabled)
				{
					_retinaFactor = 1;
				}
				else
				{
					_retinaFactor = 2;
				}
				glfwSetWindowSize(_mainWindow, (int)(_screenSize.width / 2 * _retinaFactor * _frameZoomFactor), (int)(_screenSize.height / 2 * _retinaFactor * _frameZoomFactor));

				_isInRetinaMonitor = true;
			}
			else
			{
				if (_isInRetinaMonitor)
				{
					_retinaFactor = 1;
				}
				glfwSetWindowSize(_mainWindow, (int)(_screenSize.width * _retinaFactor * _frameZoomFactor), (int)(_screenSize.height *_retinaFactor * _frameZoomFactor));

				_isInRetinaMonitor = false;
			}
		}
	}

	void CParaEngineGLView::setViewPortInPoints(float x, float y, float w, float h)
	{
		glViewport((GLint)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
			(GLint)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
			(GLsizei)(w * _scaleX * _retinaFactor * _frameZoomFactor),
			(GLsizei)(h * _scaleY * _retinaFactor * _frameZoomFactor));
	}

	void CParaEngineGLView::setScissorInPoints(float x, float y, float w, float h)
	{
		glScissor((GLint)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
			(GLint)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
			(GLsizei)(w * _scaleX * _retinaFactor * _frameZoomFactor),
			(GLsizei)(h * _scaleY * _retinaFactor * _frameZoomFactor));
	}

	void CParaEngineGLView::onGLFWError(int errorID, const char* errorDesc)
	{
		OUTPUT_LOG("GLFWError #%d Happen, %s\n", errorID, errorDesc);
	}

	void CParaEngineGLView::onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify)
	{
		UINT msg = WM_LBUTTONDOWN;
		bool bSend = true;
		if (GLFW_MOUSE_BUTTON_LEFT == button)
		{

		}
		else if (GLFW_MOUSE_BUTTON_RIGHT == button)
		{
			msg = WM_RBUTTONDOWN;
		}
		else if (GLFW_MOUSE_BUTTON_MIDDLE == button)
		{
			msg = WM_MBUTTONDOWN;
		}
		else
		{
			bSend = false;
		}

		if (!bSend)
			return;

		if (GLFW_PRESS == action)
		{
		}
		else if (GLFW_RELEASE == action)
		{
			msg++;
		}


		//if (GLFW_PRESS == action)
		//{
		//	_captured = true;

		//	if (this->getViewPortRect().equals(Rect::ZERO) || this->getViewPortRect().containsPoint(CCVector2(_mouseX, _mouseY)))
		//	{

		//	}
		//	else
		//	{
		//		bSend = false;
		//	}
		//}
		//else if (GLFW_RELEASE == action)
		//{
		//	if (_captured)
		//	{
		//		_captured = false;
		//		msg++;
		//	}
		//	else
		//	{
		//		bSend = false;
		//	}
		//}

		if (!bSend)
			return;

		LPARAM param = MAKELPARAM(_mouseX, _mouseY);
		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(msg, 0, param);
	}

	void CParaEngineGLView::onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
	{
		_mouseX = (float)x;
		_mouseY = (float)y;

		WPARAM wParam = 0;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			wParam |= MK_LBUTTON;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			wParam |= MK_RBUTTON;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		{
			wParam |= MK_MBUTTON;
		}

		LPARAM param = MAKELPARAM(_mouseX, _mouseY);
		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MOUSEMOVE, wParam, param);
	}

	void CParaEngineGLView::onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y)
	{
		WPARAM wParam = MAKEWPARAM(0, (int)y * WHEEL_DELTA);

		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MOUSEWHEEL, wParam, 0);
	}

	void CParaEngineGLView::onGLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		//if (GLFW_REPEAT != action)
		//{
		//	EventKeyboard event(g_keyCodeMap[key], GLFW_PRESS == action);
		//	auto dispatcher = Director::getInstance()->getEventDispatcher();
		//	dispatcher->dispatchEvent(&event);
		//}

		//if (GLFW_RELEASE != action && g_keyCodeMap[key] == EventKeyboard::KeyCode::KEY_BACKSPACE)
		//{
		//	IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
		//}
		UINT msg = WM_KEYDOWN;
		if (GLFW_PRESS == action)
		{

		}
		else if (GLFW_RELEASE == action)
		{
			msg++;
		}

		CGUIRoot::GetInstance()->GetKeyboard()->PushKeyEvent(msg, key, scancode);
	}

	void CParaEngineGLView::onGLFWCharCallback(GLFWwindow *window, unsigned int character)
	{
		//char16_t wcharString[2] = { (char16_t)character, 0 };
		//std::string utf8String;

		//StringUtils::UTF16ToUTF8(wcharString, utf8String);
		//IMEDispatcher::sharedDispatcher()->dispatchInsertText(utf8String.c_str(), utf8String.size());


		//CGUIRoot::GetInstance()->GetKeyboard()->PushKeyEvent(WM_CHAR, character, 0);
		//CGUIIME::SendWinMsgChar((WCHAR)character);

		auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
		if (pGUI)
		{
			std::wstring s;
			s += (WCHAR)character;
			pGUI->OnHandleWinMsgChars(s);
		}
	}


	void CParaEngineGLView::onGLFWWindowPosCallback(GLFWwindow *windows, int x, int y)
	{
		//Director::getInstance()->setViewport();
	}

	void CParaEngineGLView::onGLFWframebuffersize(GLFWwindow* window, int w, int h)
	{
		float frameSizeW = _screenSize.width;
		float frameSizeH = _screenSize.height;
		float factorX = frameSizeW / w * _retinaFactor * _frameZoomFactor;
		float factorY = frameSizeH / h * _retinaFactor * _frameZoomFactor;

		if (fabs(factorX - 0.5f) < FLT_EPSILON && fabs(factorY - 0.5f) < FLT_EPSILON)
		{
			_isInRetinaMonitor = true;
			if (_isRetinaEnabled)
			{
				_retinaFactor = 1;
			}
			else
			{
				_retinaFactor = 2;
			}

			glfwSetWindowSize(window, static_cast<int>(frameSizeW * 0.5f * _retinaFactor * _frameZoomFactor), static_cast<int>(frameSizeH * 0.5f * _retinaFactor * _frameZoomFactor));
		}
		else if (fabs(factorX - 2.0f) < FLT_EPSILON && fabs(factorY - 2.0f) < FLT_EPSILON)
		{
			_isInRetinaMonitor = false;
			_retinaFactor = 1;
			glfwSetWindowSize(window, static_cast<int>(frameSizeW * _retinaFactor * _frameZoomFactor), static_cast<int>(frameSizeH * _retinaFactor * _frameZoomFactor));
		}
	}

	void CParaEngineGLView::onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height)
	{
		//if (_resolutionPolicy != ResolutionPolicy::UNKNOWN)
		//{
		//	updateDesignResolutionSize();
		//	Director::getInstance()->setViewport();
		//}
	}


	void CParaEngineGLView::onGLFWWindowIconifyCallback(GLFWwindow* window, int iconified)
	{
		//if (iconified == GL_TRUE)
		//{
		//	Application::getInstance()->applicationDidEnterBackground();
		//}
		//else
		//{
		//	Application::getInstance()->applicationWillEnterForeground();
		//}
	}

#if (WIN32)
	static bool glew_dynamic_binding()
	{
		const char *gl_extensions = (const char*)glGetString(GL_EXTENSIONS);

		// If the current opengl driver doesn't have framebuffers methods, check if an extension exists
		if (glGenFramebuffers == nullptr)
		{
			OUTPUT_LOG("OpenGL: glGenFramebuffers is nullptr, try to detect an extension");
			if (strstr(gl_extensions, "ARB_framebuffer_object"))
			{
				OUTPUT_LOG("OpenGL: ARB_framebuffer_object is supported");

				glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");
				glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
				glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
				glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
				glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
				glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
				glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebuffer");
				glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
				glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
				glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
				glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
				glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)wglGetProcAddress("glFramebufferTexture1D");
				glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
				glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)wglGetProcAddress("glFramebufferTexture3D");
				glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
				glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
				glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
			}
			else
				if (strstr(gl_extensions, "EXT_framebuffer_object"))
				{
					OUTPUT_LOG("OpenGL: EXT_framebuffer_object is supported");
					glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbufferEXT");
					glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbufferEXT");
					glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
					glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffersEXT");
					glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorageEXT");
					glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");
					glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebufferEXT");
					glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebufferEXT");
					glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
					glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffersEXT");
					glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
					glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)wglGetProcAddress("glFramebufferTexture1DEXT");
					glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
					glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)wglGetProcAddress("glFramebufferTexture3DEXT");
					glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
					glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
					glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmapEXT");
				}
				else
				{
					OUTPUT_LOG("OpenGL: No framebuffers extension is supported");
					OUTPUT_LOG("OpenGL: Any call to Fbo will crash!");
					return false;
				}
		}
		return true;
	}
#endif


	// helper
	bool CParaEngineGLView::initGlew()
	{
#ifdef WIN32
		GLenum GlewInitResult = glewInit();
		if (GLEW_OK != GlewInitResult)
		{
			MessageBoxA(NULL, (char *)glewGetErrorString(GlewInitResult), "OpenGL error", MB_OK);
			return false;
		}

		if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
		{
			OUTPUT_LOG("Ready for GLSL");
		}
		else
		{
			OUTPUT_LOG("Not totally ready :(");
		}

		if (glewIsSupported("GL_VERSION_2_0"))
		{
			OUTPUT_LOG("Ready for OpenGL 2.0");
		}
		else
		{
			OUTPUT_LOG("OpenGL 2.0 not supported");
		}

		if (glew_dynamic_binding() == false)
		{
			MessageBoxA(NULL, "No OpenGL framebuffer support. Please upgrade the driver of your video card.", "OpenGL error", MB_OK);
			return false;
		}
#endif

		return true;
	}


	const Rect& CParaEngineGLView::getViewPortRect() const
	{
		return _viewPortRect;
	}

} // end namespace