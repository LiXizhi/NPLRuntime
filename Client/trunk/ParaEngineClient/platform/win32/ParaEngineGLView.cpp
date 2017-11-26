#include "ParaEngine.h"
#include "OpenGLWrapper.h"
#include "platform/win32/ParaEngineGLView.h"
#include "platform/win32/ParaEngineApp.h"

#include "2dengine/GUIDirectInput.h"
#include "2dengine/GUIRoot.h"

#include "2dengine/GUIIME.h"
#include "2dengine/EventBinding.h"

namespace ParaEngine {

	struct keyCodeItem
	{
		int glfwKeyCode;
		WPARAM keyCode;
	};


	static std::unordered_map<int, UINT> g_keyCodeMap;
	static std::unordered_map<UINT, int> g_naviteKeyCodeMap;

	static keyCodeItem g_keyCodeStructArray[] = {
		/* The unknown key */
		{ GLFW_KEY_UNKNOWN         , 0 },

		/* Printable keys */
		{ GLFW_KEY_SPACE           , VK_SPACE },
		{ GLFW_KEY_APOSTROPHE      , VK_OEM_7 },
		{ GLFW_KEY_COMMA           , VK_OEM_COMMA },
		{ GLFW_KEY_MINUS           , VK_OEM_MINUS },
		{ GLFW_KEY_PERIOD          , VK_OEM_PERIOD },
		{ GLFW_KEY_SLASH           , VK_OEM_2 },
		{ GLFW_KEY_0               , '0' },
		{ GLFW_KEY_1               , '1' },
		{ GLFW_KEY_2               , '2' },
		{ GLFW_KEY_3               , '3' },
		{ GLFW_KEY_4               , '4' },
		{ GLFW_KEY_5               , '5' },
		{ GLFW_KEY_6               , '6' },
		{ GLFW_KEY_7               , '7' },
		{ GLFW_KEY_8               , '8' },
		{ GLFW_KEY_9               , '9' },
		{ GLFW_KEY_SEMICOLON       , VK_OEM_1 },
		{ GLFW_KEY_EQUAL           , VK_OEM_PLUS },
		{ GLFW_KEY_A               , 'A' },
		{ GLFW_KEY_B               , 'B' },
		{ GLFW_KEY_C               , 'C' },
		{ GLFW_KEY_D               , 'D' },
		{ GLFW_KEY_E               , 'E' },
		{ GLFW_KEY_F               , 'F' },
		{ GLFW_KEY_G               , 'G' },
		{ GLFW_KEY_H               , 'H' },
		{ GLFW_KEY_I               , 'I' },
		{ GLFW_KEY_J               , 'J' },
		{ GLFW_KEY_K               , 'K' },
		{ GLFW_KEY_L               , 'L' },
		{ GLFW_KEY_M               , 'M' },
		{ GLFW_KEY_N               , 'N' },
		{ GLFW_KEY_O               , 'O' },
		{ GLFW_KEY_P               , 'P' },
		{ GLFW_KEY_Q               , 'Q' },
		{ GLFW_KEY_R               , 'R' },
		{ GLFW_KEY_S               , 'S' },
		{ GLFW_KEY_T               , 'T' },
		{ GLFW_KEY_U               , 'U' },
		{ GLFW_KEY_V               , 'V' },
		{ GLFW_KEY_W               , 'W' },
		{ GLFW_KEY_X               , 'X' },
		{ GLFW_KEY_Y               , 'Y' },
		{ GLFW_KEY_Z               , 'Z' },
		{ GLFW_KEY_LEFT_BRACKET    , VK_OEM_4 },
		{ GLFW_KEY_BACKSLASH       , VK_OEM_5 },
		{ GLFW_KEY_RIGHT_BRACKET   , VK_OEM_6 },
		{ GLFW_KEY_GRAVE_ACCENT    , VK_OEM_3 },
		{ GLFW_KEY_WORLD_1         , 0 },
		{ GLFW_KEY_WORLD_2         , 0 },

		/* Function keys */
		{ GLFW_KEY_ESCAPE          , VK_ESCAPE },
		{ GLFW_KEY_ENTER           , VK_RETURN },
		{ GLFW_KEY_TAB             , VK_TAB },
		{ GLFW_KEY_BACKSPACE       , VK_BACK },
		{ GLFW_KEY_INSERT          , VK_INSERT },
		{ GLFW_KEY_DELETE          , VK_DELETE },
		{ GLFW_KEY_RIGHT           , VK_RIGHT },
		{ GLFW_KEY_LEFT            , VK_LEFT },
		{ GLFW_KEY_DOWN            , VK_DOWN },
		{ GLFW_KEY_UP              , VK_UP },
		{ GLFW_KEY_PAGE_UP         , VK_PRIOR },
		{ GLFW_KEY_PAGE_DOWN       , VK_NEXT },
		{ GLFW_KEY_HOME            , VK_HOME },
		{ GLFW_KEY_END             , VK_END },
		{ GLFW_KEY_CAPS_LOCK       , VK_CAPITAL },
		{ GLFW_KEY_SCROLL_LOCK     , VK_SCROLL },
		{ GLFW_KEY_NUM_LOCK        , VK_NUMLOCK },
		{ GLFW_KEY_PRINT_SCREEN    , VK_SNAPSHOT },
		{ GLFW_KEY_PAUSE           , VK_PAUSE },
		{ GLFW_KEY_F1              , VK_F1 },
		{ GLFW_KEY_F2              , VK_F2 },
		{ GLFW_KEY_F3              , VK_F3 },
		{ GLFW_KEY_F4              , VK_F4 },
		{ GLFW_KEY_F5              , VK_F5 },
		{ GLFW_KEY_F6              , VK_F6 },
		{ GLFW_KEY_F7              , VK_F7 },
		{ GLFW_KEY_F8              , VK_F8 },
		{ GLFW_KEY_F9              , VK_F9 },
		{ GLFW_KEY_F10             , VK_F10 },
		{ GLFW_KEY_F11             , VK_F11 },
		{ GLFW_KEY_F12             , VK_F12 },
		{ GLFW_KEY_F13             , VK_F13 },
		{ GLFW_KEY_F14             , VK_F14 },
		{ GLFW_KEY_F15             , VK_F15 },
		{ GLFW_KEY_F16             , VK_F16 },
		{ GLFW_KEY_F17             , VK_F17 },
		{ GLFW_KEY_F18             , VK_F18 },
		{ GLFW_KEY_F19             , VK_F19 },
		{ GLFW_KEY_F20             , VK_F20 },
		{ GLFW_KEY_F21             , VK_F21 },
		{ GLFW_KEY_F22             , VK_F22 },
		{ GLFW_KEY_F23             , VK_F23 },
		{ GLFW_KEY_F24             , VK_F24 },
		{ GLFW_KEY_F25             , 0 },
		{ GLFW_KEY_KP_0            , VK_NUMPAD0 },
		{ GLFW_KEY_KP_1            , VK_NUMPAD1 },
		{ GLFW_KEY_KP_2            , VK_NUMPAD2 },
		{ GLFW_KEY_KP_3            , VK_NUMPAD3 },
		{ GLFW_KEY_KP_4            , VK_NUMPAD4 },
		{ GLFW_KEY_KP_5            , VK_NUMPAD5 },
		{ GLFW_KEY_KP_6            , VK_NUMPAD6 },
		{ GLFW_KEY_KP_7            , VK_NUMPAD7 },
		{ GLFW_KEY_KP_8            , VK_NUMPAD8 },
		{ GLFW_KEY_KP_9            , VK_NUMPAD9 },
		{ GLFW_KEY_KP_DECIMAL      , VK_DECIMAL },
		{ GLFW_KEY_KP_DIVIDE       , VK_DIVIDE },
		{ GLFW_KEY_KP_MULTIPLY     , VK_MULTIPLY },
		{ GLFW_KEY_KP_SUBTRACT     , VK_SUBTRACT },
		{ GLFW_KEY_KP_ADD          , VK_ADD },
		{ GLFW_KEY_KP_ENTER        , VK_SEPARATOR },
		{ GLFW_KEY_KP_EQUAL        , 0 },
		{ GLFW_KEY_LEFT_SHIFT      , VK_LSHIFT },
		{ GLFW_KEY_LEFT_CONTROL    , VK_LCONTROL },
		{ GLFW_KEY_LEFT_ALT        , VK_LMENU },
		{ GLFW_KEY_LEFT_SUPER      , VK_LWIN },
		{ GLFW_KEY_RIGHT_SHIFT     , VK_RSHIFT },
		{ GLFW_KEY_RIGHT_CONTROL   , VK_RCONTROL },
		{ GLFW_KEY_RIGHT_ALT       , VK_RMENU },
		{ GLFW_KEY_RIGHT_SUPER     , VK_RWIN },
		{ GLFW_KEY_MENU            , VK_MENU },
		{ GLFW_KEY_LAST            , 0 }
	};


	//static keyCodeItem g_keyCodeStructArray[] = {
	//	/* The unknown key */
	//	{ GLFW_KEY_UNKNOWN         , 0 },

	//	/* Printable keys */
	//	{ GLFW_KEY_SPACE           , 0x039 },
	//	{ GLFW_KEY_APOSTROPHE      , 0x028 },
	//	{ GLFW_KEY_COMMA           , 0x033 },
	//	{ GLFW_KEY_MINUS           , 0x00C },
	//	{ GLFW_KEY_PERIOD          , 0x034 },
	//	{ GLFW_KEY_SLASH           , 0x035 },
	//	{ GLFW_KEY_0               , 0x00B },
	//	{ GLFW_KEY_1               , 0x002 },
	//	{ GLFW_KEY_2               , 0x003 },
	//	{ GLFW_KEY_3               , 0x004 },
	//	{ GLFW_KEY_4               , 0x005 },
	//	{ GLFW_KEY_5               , 0x006 },
	//	{ GLFW_KEY_6               , 0x007 },
	//	{ GLFW_KEY_7               , 0x008 },
	//	{ GLFW_KEY_8               , 0x009 },
	//	{ GLFW_KEY_9               , 0x00A },
	//	{ GLFW_KEY_SEMICOLON       , 0x027 },
	//	{ GLFW_KEY_EQUAL           , 0x00D },
	//	{ GLFW_KEY_A               , 0x01E },
	//	{ GLFW_KEY_B               , 0x030 },
	//	{ GLFW_KEY_C               , 0x02E },
	//	{ GLFW_KEY_D               , 0x020 },
	//	{ GLFW_KEY_E               , 0x012 },
	//	{ GLFW_KEY_F               , 0x021 },
	//	{ GLFW_KEY_G               , 0x022 },
	//	{ GLFW_KEY_H               , 0x023 },
	//	{ GLFW_KEY_I               , 0x017 },
	//	{ GLFW_KEY_J               , 0x024 },
	//	{ GLFW_KEY_K               , 0x025 },
	//	{ GLFW_KEY_L               , 0x026 },
	//	{ GLFW_KEY_M               , 0x032 },
	//	{ GLFW_KEY_N               , 0x031 },
	//	{ GLFW_KEY_O               , 0x018 },
	//	{ GLFW_KEY_P               , 0x019 },
	//	{ GLFW_KEY_Q               , 0x010 },
	//	{ GLFW_KEY_R               , 0x013 },
	//	{ GLFW_KEY_S               , 0x01F },
	//	{ GLFW_KEY_T               , 0x014 },
	//	{ GLFW_KEY_U               , 0x016 },
	//	{ GLFW_KEY_V               , 0x02F },
	//	{ GLFW_KEY_W               , 0x011 },
	//	{ GLFW_KEY_X               , 0x02D },
	//	{ GLFW_KEY_Y               , 0x015 },
	//	{ GLFW_KEY_Z               , 0x02C },
	//	{ GLFW_KEY_LEFT_BRACKET    , 0x01A },
	//	{ GLFW_KEY_BACKSLASH       , 0x02B },
	//	{ GLFW_KEY_RIGHT_BRACKET   , 0x01B },
	//	{ GLFW_KEY_GRAVE_ACCENT    , 0x029 },
	//	{ GLFW_KEY_WORLD_1         , 0 },
	//	{ GLFW_KEY_WORLD_2         , 0x056 },

	//	/* Function keys */
	//	{ GLFW_KEY_ESCAPE          , 0x001 },
	//	{ GLFW_KEY_ENTER           , 0x01C },
	//	{ GLFW_KEY_TAB             , 0x00F },
	//	{ GLFW_KEY_BACKSPACE       , 0x00E },
	//	{ GLFW_KEY_INSERT          , 0x152 },
	//	{ GLFW_KEY_DELETE          , 0x153 },
	//	{ GLFW_KEY_RIGHT           , 0x14D },
	//	{ GLFW_KEY_LEFT            , 0x14B },
	//	{ GLFW_KEY_DOWN            , 0x150 },
	//	{ GLFW_KEY_UP              , 0x148 },
	//	{ GLFW_KEY_PAGE_UP         , 0x149 },
	//	{ GLFW_KEY_PAGE_DOWN       , 0x151 },
	//	{ GLFW_KEY_HOME            , 0x147 },
	//	{ GLFW_KEY_END             , 0x14F },
	//	{ GLFW_KEY_CAPS_LOCK       , 0x03A },
	//	{ GLFW_KEY_SCROLL_LOCK     , 0x046 },
	//	{ GLFW_KEY_NUM_LOCK        , 0x145 },
	//	{ GLFW_KEY_PRINT_SCREEN    , 0x137 },
	//	{ GLFW_KEY_PAUSE           , 0x046 },
	//	{ GLFW_KEY_F1              , 0x03B },
	//	{ GLFW_KEY_F2              , 0x03C },
	//	{ GLFW_KEY_F3              , 0x03D },
	//	{ GLFW_KEY_F4              , 0x03E },
	//	{ GLFW_KEY_F5              , 0x03F },
	//	{ GLFW_KEY_F6              , 0x040 },
	//	{ GLFW_KEY_F7              , 0x041 },
	//	{ GLFW_KEY_F8              , 0x042 },
	//	{ GLFW_KEY_F9              , 0x043 },
	//	{ GLFW_KEY_F10             , 0x044 },
	//	{ GLFW_KEY_F11             , 0x057 },
	//	{ GLFW_KEY_F12             , 0x058 },
	//	{ GLFW_KEY_F13             , 0x064 },
	//	{ GLFW_KEY_F14             , 0x065 },
	//	{ GLFW_KEY_F15             , 0x066 },
	//	{ GLFW_KEY_F16             , 0x067 },
	//	{ GLFW_KEY_F17             , 0x068 },
	//	{ GLFW_KEY_F18             , 0x069 },
	//	{ GLFW_KEY_F19             , 0x06A },
	//	{ GLFW_KEY_F20             , 0x06B },
	//	{ GLFW_KEY_F21             , 0x06C },
	//	{ GLFW_KEY_F22             , 0x06D },
	//	{ GLFW_KEY_F23             , 0x06E },
	//	{ GLFW_KEY_F24             , 0x076 },
	//	{ GLFW_KEY_F25             , 0 },
	//	{ GLFW_KEY_KP_0            , 0x052 },
	//	{ GLFW_KEY_KP_1            , 0x04F },
	//	{ GLFW_KEY_KP_2            , 0x050 },
	//	{ GLFW_KEY_KP_3            , 0x051 },
	//	{ GLFW_KEY_KP_4            , 0x04B },
	//	{ GLFW_KEY_KP_5            , 0x04C },
	//	{ GLFW_KEY_KP_6            , 0x04D },
	//	{ GLFW_KEY_KP_7            , 0x047 },
	//	{ GLFW_KEY_KP_8            , 0x048 },
	//	{ GLFW_KEY_KP_9            , 0x049 },
	//	{ GLFW_KEY_KP_DECIMAL      , 0x053 },
	//	{ GLFW_KEY_KP_DIVIDE       , 0x135 },
	//	{ GLFW_KEY_KP_MULTIPLY     , 0x037 },
	//	{ GLFW_KEY_KP_SUBTRACT     , 0x04A },
	//	{ GLFW_KEY_KP_ADD          , 0x04E },
	//	{ GLFW_KEY_KP_ENTER        , 0x11C },
	//	{ GLFW_KEY_KP_EQUAL        , 0 },
	//	{ GLFW_KEY_LEFT_SHIFT      , 0x02A },
	//	{ GLFW_KEY_LEFT_CONTROL    , 0x01D },
	//	{ GLFW_KEY_LEFT_ALT        , 0x038 },
	//	{ GLFW_KEY_LEFT_SUPER      , 0x15B },
	//	{ GLFW_KEY_RIGHT_SHIFT     , 0x036 },
	//	{ GLFW_KEY_RIGHT_CONTROL   , 0x11D },
	//	{ GLFW_KEY_RIGHT_ALT       , 0x138 },
	//	{ GLFW_KEY_RIGHT_SUPER     , 0x15C },
	//	{ GLFW_KEY_MENU            , 0x15D },
	//	{ GLFW_KEY_LAST            , 0 }
	//};


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

		
		g_keyCodeMap.clear();
		for (auto& item : g_keyCodeStructArray)
		{
			g_keyCodeMap[item.glfwKeyCode] = item.keyCode;
			g_naviteKeyCodeMap[item.keyCode] = item.glfwKeyCode;
		}


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
		if (ret && ret->initWithRect(viewName, NPLRect(0, 0, 960, 640), 1)) {
			return ret;
		}

		return nullptr;
	}

	CParaEngineGLView* CParaEngineGLView::createWithRect(const std::string& viewName, const NPLRect& rect, float frameZoomFactor)
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

	bool CParaEngineGLView::initWithRect(const std::string& viewName, const NPLRect& rect, float frameZoomFactor)
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
		return initWithRect(viewName, NPLRect(0, 0, (float)videoMode->width, (float)videoMode->height), 1.0f);
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

		return initWithRect(viewname, NPLRect(0, 0, (float)videoMode.width, (float)videoMode.height), 1.0f);
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

		WPARAM wParam = g_keyCodeMap[key];

		if (wParam != 0)
			CGUIRoot::GetInstance()->GetKeyboard()->PushKeyEvent(msg, wParam, 0);
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


	const NPLRect& CParaEngineGLView::getViewPortRect() const
	{
		return _viewPortRect;
	}

	bool CParaEngineGLView::IsKeyPressed(DWORD nKey)
	{
		if (_mainWindow)
		{
			auto vkey = CEventBinding::DIKToWinVirtualKey[nKey];
			if (vkey == 0)
				return false;

			auto glfwKey = g_naviteKeyCodeMap[vkey];

			return glfwGetKey(_mainWindow, glfwKey) == GLFW_PRESS;
		}
		else
		{
			return false;
		}
	}

} // end namespace