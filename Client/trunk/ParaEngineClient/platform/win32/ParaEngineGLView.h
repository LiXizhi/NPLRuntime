#pragma once


#include "IAttributeFields.h"

#if (WIN32)
	#ifndef GLFW_EXPOSE_NATIVE_WIN32
		#define GLFW_EXPOSE_NATIVE_WIN32
	#endif

	#ifndef GLFW_EXPOSE_NATIVE_WGL
		#define GLFW_EXPOSE_NATIVE_WGL
	#endif

	#include "GLFW/glfw3native.h"
#endif

#include "GLType.h"

namespace ParaEngine {

	/** @struct GLContextAttrs
	*
	* There are six opengl Context Attrs.
	*/
	struct GLContextAttrs
	{
		int redBits;
		int greenBits;
		int blueBits;
		int alphaBits;
		int depthBits;
		int stencilBits;
	};




	class CParaEngineGLView : public IObject
	{
	public:
		/** Static method and member so that we can modify it on all platforms before create OpenGL context.
		*
		* @param glContextAttrs The OpenGL context attrs.
		*/
		static void setGLContextAttrs(GLContextAttrs& glContextAttrs);

		/** Return the OpenGL context attrs.
		*
		* @return Return the OpenGL context attrs.
		*/
		const GLContextAttrs& getGLContextAttrs();

		/** The OpenGL context attrs. */
		static GLContextAttrs _glContextAttrs;

	public:
		static CParaEngineGLView* create(const std::string& viewName);
		static CParaEngineGLView* createWithRect(const std::string& viewName, const Rect& rect, float frameZoomFactor = 1.0f);
		static CParaEngineGLView* createWithFullScreen(const std::string& viewName);
		static CParaEngineGLView* createWithFullScreen(const std::string& viewName, const GLFWvidmode &videoMode, GLFWmonitor *monitor);

		/*
		*frameZoomFactor for frame. This method is for debugging big resolution (e.g.new ipad) app on desktop.
		*/
		float getFrameZoomFactor() const;

		void setViewPortInPoints(float x, float y, float w, float h);
		void setScissorInPoints(float x, float y, float w, float h);

		bool windowShouldClose();
		void pollEvents();
		GLFWwindow* getWindow() const { return _mainWindow; }

		bool isOpenGLReady();
		void end();
		void swapBuffers();
		void setIMEKeyboardState(bool bOpen);

		/*
		* Set zoom factor for frame. This method is for debugging big resolution (e.g.new ipad) app on desktop.
		*/
		void setFrameZoomFactor(float zoomFactor);

		/**
		* Hide or Show the mouse cursor if there is one.
		*/
		void setCursorVisible(bool isVisible);

		/** Retina support is disabled by default
		*  @note This method is only available on Mac.
		*/
		void enableRetina(bool enabled);

		/** Check whether retina display is enabled. */
		bool isRetinaEnabled() const { return _isRetinaEnabled; };

		/** Get retina factor */
		int getRetinaFactor() const { return _retinaFactor; }

		void setViewName(const std::string viewname) { _viewName = viewname; }

		/**
		* Get the opengl view port rectangle.
		*
		* @return Return the opengl view port rectangle.
		*/
		const Rect& getViewPortRect() const;


		/** return true if the key is being pressed at the time of calling.
		* @param nKey: a direct input key, such as DIK_SPACE. */
		bool IsKeyPressed(DWORD nKey);

		/**
		* Set the frame size of EGL view.
		*
		* @param width The width of the fram size.
		* @param height The height of the fram size.
		*/
		void setFrameSize(float width, float height);

#ifdef WIN32
		HWND getWin32Window() { return glfwGetWin32Window(_mainWindow); };
#endif /* (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) */
	protected:
		CParaEngineGLView();
		virtual ~CParaEngineGLView();

		bool initWithRect(const std::string& viewName, const Rect& rect, float frameZoomFactor);
		bool initWithFullScreen(const std::string& viewName);
		bool initWithFullscreen(const std::string& viewname, const GLFWvidmode &videoMode, GLFWmonitor *monitor);

		bool initGlew();

		void updateFrameSize();

		// GLFW callbacks
		void onGLFWError(int errorID, const char* errorDesc);
		void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify);
		void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y);
		void onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y);
		void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void onGLFWCharCallback(GLFWwindow* window, unsigned int character);
		void onGLFWWindowPosCallback(GLFWwindow* windows, int x, int y);
		void onGLFWframebuffersize(GLFWwindow* window, int w, int h);
		void onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height);
		void onGLFWWindowIconifyCallback(GLFWwindow* window, int iconified);

		/**
		* Get the frame size of EGL view.
		* In general, it returns the screen size since the EGL view is a fullscreen view.
		*
		* @return The frame size of EGL view.
		*/
		const Size& getFrameSize() const;

		

	protected:

		GLFWwindow* _mainWindow;
		GLFWmonitor* _monitor;

		bool _isRetinaEnabled;
		bool _isInRetinaMonitor;
		int  _retinaFactor;  // Should be 1 or 2

		bool _captured;
		bool _supportTouch;

		float _mouseX;
		float _mouseY;

		float _frameZoomFactor;

		std::string _viewName;

		// real screen size
		Size _screenSize;

		// resolution size, it is the size appropriate for the app resources.
		Size _designResolutionSize;

		// the view port size
		Rect _viewPortRect;

		float _scaleX;
		float _scaleY;

		friend class GLFWEventHandler;
	};

} // end namespace