//-----------------------------------------------------------------------------
// PicoOpenXR.cpp
// Authors: big
// CreateDate: 2026.10.28
// Desc: OpenXR (GLES) session for PICO headsets.
// NOTE: do not include ParaEngine.h here, it pulls in the GLES2 headers of the engine.
//-----------------------------------------------------------------------------

#ifndef XR_USE_PLATFORM_ANDROID
#define XR_USE_PLATFORM_ANDROID
#endif
#ifndef XR_USE_GRAPHICS_API_OPENGL_ES
#define XR_USE_GRAPHICS_API_OPENGL_ES
#endif

#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <android/log.h>

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>

#include "PicoOpenXR.h"

#define XR_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngineXR", __VA_ARGS__))
#define XR_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ParaEngineXR", __VA_ARGS__))
#define XR_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ParaEngineXR", __VA_ARGS__))

#ifndef GL_FRAMEBUFFER_SRGB_EXT
#define GL_FRAMEBUFFER_SRGB_EXT 0x8DB9
#endif

namespace ParaEngine
{
	static bool XrCheck(XrInstance instance, XrResult result, const char* what)
	{
		if (XR_SUCCEEDED(result))
			return true;

		char resultName[XR_MAX_RESULT_STRING_SIZE] = "";
		if (instance != XR_NULL_HANDLE)
			xrResultToString(instance, result, resultName);
		XR_LOGE("%s failed: %d %s", what, (int)result, resultName);
		return false;
	}

	/** debug: each time "adb shell setprop debug.paracraft.xrdump <new value>" changes the value, the next rendered
	* frame is saved to <app files dir>/xr_frame.ppm (quarter size).
	* read it with "adb exec-out run-as <package> cat files/xr_frame.ppm > xr_frame.ppm". */
	static void DumpFrameIfRequested(int width, int height)
	{
		static int s_frame = 0;
		static char s_lastValue[PROP_VALUE_MAX] = "";
		if ((s_frame++ % 30) != 0)
			return;
		char value[PROP_VALUE_MAX] = "";
		__system_property_get("debug.paracraft.xrdump", value);
		if (strcmp(value, s_lastValue) == 0)
			return;
		bool firstCheck = (s_frame == 1);
		strcpy(s_lastValue, value);
		if (firstCheck || value[0] == '\0')
			return;

		char package[256] = "";
		FILE* cmdline = fopen("/proc/self/cmdline", "r");
		if (cmdline)
		{
			fread(package, 1, sizeof(package) - 1, cmdline);
			fclose(cmdline);
		}
		char path[512];
		snprintf(path, sizeof(path), "/data/data/%s/files/xr_frame.ppm", package);

		std::vector<unsigned char> pixels((size_t)width * height * 4);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

		const int step = 4;
		int outW = width / step, outH = height / step;
		FILE* file = fopen(path, "wb");
		if (!file)
		{
			XR_LOGE("can not write %s", path);
			return;
		}
		fprintf(file, "P6\n%d %d\n255\n", outW, outH);
		for (int y = outH - 1; y >= 0; --y)
		{
			for (int x = 0; x < outW; ++x)
			{
				const unsigned char* p = &pixels[((size_t)(y * step) * width + x * step) * 4];
				fwrite(p, 1, 3, file);
			}
		}
		fclose(file);
		XR_LOGI("frame saved to %s (%d x %d)", path, outW, outH);
	}

	static bool HasExtension(const std::vector<XrExtensionProperties>& extensions, const char* name)
	{
		for (auto& ext : extensions)
		{
			if (strcmp(ext.extensionName, name) == 0)
				return true;
		}
		return false;
	}

	CPicoOpenXR& CPicoOpenXR::GetInstance()
	{
		static CPicoOpenXR s_instance;
		return s_instance;
	}

	CPicoOpenXR::CPicoOpenXR()
		: m_instance(XR_NULL_HANDLE)
		, m_systemId(XR_NULL_SYSTEM_ID)
		, m_session(XR_NULL_HANDLE)
		, m_appSpace(XR_NULL_HANDLE)
		, m_sessionState(XR_SESSION_STATE_UNKNOWN)
		, m_sessionRunning(false)
		, m_exitRequested(false)
		, m_hasBDController(false)
		, m_eyeWidth(0)
		, m_eyeHeight(0)
		, m_swapchain(XR_NULL_HANDLE)
		, m_swapchainFormat(0)
		, m_disableSRGBWrite(false)
		, m_depthBuffer(0)
		, m_actionSet(XR_NULL_HANDLE)
		, m_aimPoseAction(XR_NULL_HANDLE)
		, m_triggerAction(XR_NULL_HANDLE)
		, m_squeezeAction(XR_NULL_HANDLE)
		, m_thumbstickAction(XR_NULL_HANDLE)
		, m_thumbstickClickAction(XR_NULL_HANDLE)
		, m_primaryAction(XR_NULL_HANDLE)
		, m_secondaryAction(XR_NULL_HANDLE)
		, m_menuAction(XR_NULL_HANDLE)
	{
		m_handPath[0] = m_handPath[1] = XR_NULL_PATH;
		m_handSpace[0] = m_handSpace[1] = XR_NULL_HANDLE;
	}

	CPicoOpenXR::~CPicoOpenXR()
	{
	}

	bool CPicoOpenXR::Init(void* javaVM, void* activity)
	{
		if (IsInited())
			return true;

		if (javaVM == nullptr || activity == nullptr)
		{
			XR_LOGE("Init: JavaVM or activity is null");
			return false;
		}

		m_exitRequested = false;

		// the android loader must be initialized before any other call
		PFN_xrInitializeLoaderKHR initializeLoader = nullptr;
		if (XR_SUCCEEDED(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&initializeLoader)) && initializeLoader)
		{
			XrLoaderInitInfoAndroidKHR loaderInfo = { XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
			loaderInfo.applicationVM = javaVM;
			loaderInfo.applicationContext = activity;
			initializeLoader((const XrLoaderInitInfoBaseHeaderKHR*)&loaderInfo);
		}

		uint32_t extensionCount = 0;
		if (!XrCheck(XR_NULL_HANDLE, xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "xrEnumerateInstanceExtensionProperties"))
			return false;
		std::vector<XrExtensionProperties> extensions(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
		xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensions.data());

		if (!HasExtension(extensions, XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME))
		{
			XR_LOGE("runtime does not support %s", XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);
			return false;
		}

		std::vector<const char*> enabledExtensions;
		enabledExtensions.push_back(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);
		enabledExtensions.push_back(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
		m_hasBDController = HasExtension(extensions, XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME);
		if (m_hasBDController)
			enabledExtensions.push_back(XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME);

		XrInstanceCreateInfoAndroidKHR androidInfo = { XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
		androidInfo.applicationVM = javaVM;
		androidInfo.applicationActivity = activity;

		XrInstanceCreateInfo createInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
		createInfo.next = &androidInfo;
		strncpy(createInfo.applicationInfo.applicationName, "Paracraft", XR_MAX_APPLICATION_NAME_SIZE - 1);
		strncpy(createInfo.applicationInfo.engineName, "ParaEngine", XR_MAX_ENGINE_NAME_SIZE - 1);
		createInfo.applicationInfo.applicationVersion = 1;
		createInfo.applicationInfo.engineVersion = 1;
		// PICO runtimes before OS 5.x only expose OpenXR 1.0
		createInfo.applicationInfo.apiVersion = XR_API_VERSION_1_0;
		createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		createInfo.enabledExtensionNames = enabledExtensions.data();

		if (!XrCheck(XR_NULL_HANDLE, xrCreateInstance(&createInfo, &m_instance), "xrCreateInstance"))
			return false;

		XrInstanceProperties instanceProps = { XR_TYPE_INSTANCE_PROPERTIES };
		if (XR_SUCCEEDED(xrGetInstanceProperties(m_instance, &instanceProps)))
		{
			XR_LOGI("OpenXR runtime: %s %d.%d.%d", instanceProps.runtimeName,
				XR_VERSION_MAJOR(instanceProps.runtimeVersion), XR_VERSION_MINOR(instanceProps.runtimeVersion), XR_VERSION_PATCH(instanceProps.runtimeVersion));
		}

		XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
		systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
		if (!XrCheck(m_instance, xrGetSystem(m_instance, &systemInfo, &m_systemId), "xrGetSystem"))
		{
			Shutdown();
			return false;
		}

		// the runtime requires this call before xrCreateSession
		PFN_xrGetOpenGLESGraphicsRequirementsKHR getGraphicsRequirements = nullptr;
		xrGetInstanceProcAddr(m_instance, "xrGetOpenGLESGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&getGraphicsRequirements);
		if (getGraphicsRequirements)
		{
			XrGraphicsRequirementsOpenGLESKHR requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR };
			getGraphicsRequirements(m_instance, m_systemId, &requirements);

			GLint major = 0, minor = 0;
			glGetIntegerv(GL_MAJOR_VERSION, &major);
			glGetIntegerv(GL_MINOR_VERSION, &minor);
			XrVersion glesVersion = XR_MAKE_VERSION(major, minor, 0);
			if (glesVersion < requirements.minApiVersionSupported)
			{
				XR_LOGE("OpenGL ES %d.%d is lower than the runtime requires(%d.%d)", major, minor,
					XR_VERSION_MAJOR(requirements.minApiVersionSupported), XR_VERSION_MINOR(requirements.minApiVersionSupported));
				Shutdown();
				return false;
			}
		}

		uint32_t viewCount = 0;
		xrEnumerateViewConfigurationViews(m_instance, m_systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
		std::vector<XrViewConfigurationView> configViews(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
		xrEnumerateViewConfigurationViews(m_instance, m_systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, configViews.data());
		if (viewCount != 2)
		{
			XR_LOGE("stereo view configuration expects 2 views, got %d", viewCount);
			Shutdown();
			return false;
		}
		m_eyeWidth = (int)configViews[0].recommendedImageRectWidth;
		m_eyeHeight = (int)configViews[0].recommendedImageRectHeight;
		XR_LOGI("recommended eye size: %d x %d", m_eyeWidth, m_eyeHeight);

		// bind the current GL context (owned by GLSurfaceView) to the session
		EGLDisplay display = eglGetCurrentDisplay();
		EGLContext context = eglGetCurrentContext();
		EGLint configId = 0;
		eglQueryContext(display, context, EGL_CONFIG_ID, &configId);
		EGLint configAttribs[] = { EGL_CONFIG_ID, configId, EGL_NONE };
		EGLConfig config = nullptr;
		EGLint numConfigs = 0;
		eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

		XrGraphicsBindingOpenGLESAndroidKHR graphicsBinding = { XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR };
		graphicsBinding.display = display;
		graphicsBinding.config = config;
		graphicsBinding.context = context;

		XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
		sessionInfo.next = &graphicsBinding;
		sessionInfo.systemId = m_systemId;
		if (!XrCheck(m_instance, xrCreateSession(m_instance, &sessionInfo, &m_session), "xrCreateSession"))
		{
			Shutdown();
			return false;
		}

		// same reference space as WebXR 'local'
		XrReferenceSpaceCreateInfo spaceInfo = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
		spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
		spaceInfo.poseInReferenceSpace.orientation.w = 1.f;
		if (!XrCheck(m_instance, xrCreateReferenceSpace(m_session, &spaceInfo, &m_appSpace), "xrCreateReferenceSpace"))
		{
			Shutdown();
			return false;
		}

		if (!CreateSwapchain() || !CreateActions())
		{
			Shutdown();
			return false;
		}

		// frame pacing is done by xrWaitFrame, the GLSurfaceView window is not visible in the headset.
		eglSwapInterval(display, 0);

		XR_LOGI("OpenXR initialized");
		return true;
	}

	bool CPicoOpenXR::CreateSwapchain()
	{
		uint32_t formatCount = 0;
		xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr);
		std::vector<int64_t> formats(formatCount);
		xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data());

		// The engine writes gamma space colors. With an sRGB swapchain we must turn off the linear to sRGB
		// conversion on write, otherwise use a plain RGBA8 swapchain.
		const char* glExtensions = (const char*)glGetString(GL_EXTENSIONS);
		bool hasSRGBWriteControl = glExtensions && strstr(glExtensions, "GL_EXT_sRGB_write_control") != nullptr;

		m_swapchainFormat = 0;
		m_disableSRGBWrite = false;
		for (auto format : formats)
		{
			if (format == GL_SRGB8_ALPHA8 && hasSRGBWriteControl)
			{
				m_swapchainFormat = format;
				m_disableSRGBWrite = true;
				break;
			}
		}
		if (m_swapchainFormat == 0)
		{
			for (auto format : formats)
			{
				if (format == GL_RGBA8)
				{
					m_swapchainFormat = format;
					break;
				}
			}
		}
		if (m_swapchainFormat == 0 && !formats.empty())
			m_swapchainFormat = formats[0];
		XR_LOGI("swapchain format: 0x%x", (int)m_swapchainFormat);

		XrSwapchainCreateInfo swapchainInfo = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchainInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
		swapchainInfo.format = m_swapchainFormat;
		swapchainInfo.sampleCount = 1;
		swapchainInfo.width = (uint32_t)GetRenderWidth();
		swapchainInfo.height = (uint32_t)GetRenderHeight();
		swapchainInfo.faceCount = 1;
		swapchainInfo.arraySize = 1;
		swapchainInfo.mipCount = 1;
		if (!XrCheck(m_instance, xrCreateSwapchain(m_session, &swapchainInfo, &m_swapchain), "xrCreateSwapchain"))
			return false;

		uint32_t imageCount = 0;
		xrEnumerateSwapchainImages(m_swapchain, 0, &imageCount, nullptr);
		std::vector<XrSwapchainImageOpenGLESKHR> images(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR });
		xrEnumerateSwapchainImages(m_swapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)images.data());

		glGenRenderbuffers(1, &m_depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, GetRenderWidth(), GetRenderHeight());
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		m_swapchainTextures.resize(imageCount);
		m_framebuffers.resize(imageCount);
		glGenFramebuffers((GLsizei)imageCount, m_framebuffers.data());
		for (uint32_t i = 0; i < imageCount; ++i)
		{
			m_swapchainTextures[i] = images[i].image;
			glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, images[i].image, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				XR_LOGE("swapchain framebuffer %d is incomplete: 0x%x", i, status);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return false;
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		XR_LOGI("swapchain created: %d x %d, %d images", GetRenderWidth(), GetRenderHeight(), imageCount);
		return true;
	}

	bool CPicoOpenXR::CreateActions()
	{
		XrActionSetCreateInfo actionSetInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
		strcpy(actionSetInfo.actionSetName, "paracraft");
		strcpy(actionSetInfo.localizedActionSetName, "Paracraft");
		if (!XrCheck(m_instance, xrCreateActionSet(m_instance, &actionSetInfo, &m_actionSet), "xrCreateActionSet"))
			return false;

		xrStringToPath(m_instance, "/user/hand/left", &m_handPath[0]);
		xrStringToPath(m_instance, "/user/hand/right", &m_handPath[1]);

		auto createAction = [this](XrAction& action, const char* name, const char* localizedName, XrActionType type) {
			XrActionCreateInfo actionInfo = { XR_TYPE_ACTION_CREATE_INFO };
			actionInfo.actionType = type;
			strcpy(actionInfo.actionName, name);
			strcpy(actionInfo.localizedActionName, localizedName);
			actionInfo.countSubactionPaths = 2;
			actionInfo.subactionPaths = m_handPath;
			return XrCheck(m_instance, xrCreateAction(m_actionSet, &actionInfo, &action), name);
		};

		if (!createAction(m_aimPoseAction, "aim_pose", "Aim Pose", XR_ACTION_TYPE_POSE_INPUT) ||
			!createAction(m_triggerAction, "trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT) ||
			!createAction(m_squeezeAction, "squeeze", "Squeeze", XR_ACTION_TYPE_FLOAT_INPUT) ||
			!createAction(m_thumbstickAction, "thumbstick", "Thumbstick", XR_ACTION_TYPE_VECTOR2F_INPUT) ||
			!createAction(m_thumbstickClickAction, "thumbstick_click", "Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT) ||
			!createAction(m_primaryAction, "primary_button", "A/X Button", XR_ACTION_TYPE_BOOLEAN_INPUT) ||
			!createAction(m_secondaryAction, "secondary_button", "B/Y Button", XR_ACTION_TYPE_BOOLEAN_INPUT) ||
			!createAction(m_menuAction, "menu", "Menu", XR_ACTION_TYPE_BOOLEAN_INPUT))
			return false;

		struct Binding { XrAction action; const char* path; };
		// a profile whose paths the runtime does not know is rejected as a whole, the others still work.
		auto suggest = [this](const char* profile, const std::vector<Binding>& bindings) {
			XrPath profilePath;
			xrStringToPath(m_instance, profile, &profilePath);
			std::vector<XrActionSuggestedBinding> suggested;
			for (auto& binding : bindings)
			{
				XrPath path;
				if (XR_SUCCEEDED(xrStringToPath(m_instance, binding.path, &path)))
					suggested.push_back({ binding.action, path });
			}
			XrInteractionProfileSuggestedBinding suggestedInfo = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
			suggestedInfo.interactionProfile = profilePath;
			suggestedInfo.countSuggestedBindings = (uint32_t)suggested.size();
			suggestedInfo.suggestedBindings = suggested.data();
			if (XR_FAILED(xrSuggestInteractionProfileBindings(m_instance, &suggestedInfo)))
				XR_LOGW("interaction profile %s is not supported", profile);
		};

		std::vector<Binding> touchBindings = {
			{ m_aimPoseAction, "/user/hand/left/input/aim/pose" },
			{ m_aimPoseAction, "/user/hand/right/input/aim/pose" },
			{ m_triggerAction, "/user/hand/left/input/trigger/value" },
			{ m_triggerAction, "/user/hand/right/input/trigger/value" },
			{ m_squeezeAction, "/user/hand/left/input/squeeze/value" },
			{ m_squeezeAction, "/user/hand/right/input/squeeze/value" },
			{ m_thumbstickAction, "/user/hand/left/input/thumbstick" },
			{ m_thumbstickAction, "/user/hand/right/input/thumbstick" },
			{ m_thumbstickClickAction, "/user/hand/left/input/thumbstick/click" },
			{ m_thumbstickClickAction, "/user/hand/right/input/thumbstick/click" },
			{ m_primaryAction, "/user/hand/left/input/x/click" },
			{ m_primaryAction, "/user/hand/right/input/a/click" },
			{ m_secondaryAction, "/user/hand/left/input/y/click" },
			{ m_secondaryAction, "/user/hand/right/input/b/click" },
			{ m_menuAction, "/user/hand/left/input/menu/click" },
		};

		suggest("/interaction_profiles/khr/simple_controller", {
			{ m_aimPoseAction, "/user/hand/left/input/aim/pose" },
			{ m_aimPoseAction, "/user/hand/right/input/aim/pose" },
			{ m_triggerAction, "/user/hand/left/input/select/click" },
			{ m_triggerAction, "/user/hand/right/input/select/click" },
			{ m_menuAction, "/user/hand/left/input/menu/click" },
			{ m_menuAction, "/user/hand/right/input/menu/click" },
		});
		suggest("/interaction_profiles/oculus/touch_controller", touchBindings);
		if (m_hasBDController)
		{
			// PICO 4 has the same layout as touch controllers
			suggest("/interaction_profiles/bytedance/pico4_controller", touchBindings);
		}

		for (int hand = 0; hand < 2; ++hand)
		{
			XrActionSpaceCreateInfo spaceInfo = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			spaceInfo.action = m_aimPoseAction;
			spaceInfo.subactionPath = m_handPath[hand];
			spaceInfo.poseInActionSpace.orientation.w = 1.f;
			if (!XrCheck(m_instance, xrCreateActionSpace(m_session, &spaceInfo, &m_handSpace[hand]), "xrCreateActionSpace"))
				return false;
		}

		XrSessionActionSetsAttachInfo attachInfo = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
		attachInfo.countActionSets = 1;
		attachInfo.actionSets = &m_actionSet;
		return XrCheck(m_instance, xrAttachSessionActionSets(m_session, &attachInfo), "xrAttachSessionActionSets");
	}

	void CPicoOpenXR::Shutdown()
	{
		if (!m_framebuffers.empty())
		{
			glDeleteFramebuffers((GLsizei)m_framebuffers.size(), m_framebuffers.data());
			m_framebuffers.clear();
		}
		m_swapchainTextures.clear();
		if (m_depthBuffer != 0)
		{
			glDeleteRenderbuffers(1, &m_depthBuffer);
			m_depthBuffer = 0;
		}

		for (int hand = 0; hand < 2; ++hand)
		{
			if (m_handSpace[hand] != XR_NULL_HANDLE)
			{
				xrDestroySpace(m_handSpace[hand]);
				m_handSpace[hand] = XR_NULL_HANDLE;
			}
		}
		// destroying the action set destroys its actions
		if (m_actionSet != XR_NULL_HANDLE)
		{
			xrDestroyActionSet(m_actionSet);
			m_actionSet = XR_NULL_HANDLE;
		}
		m_aimPoseAction = m_triggerAction = m_squeezeAction = m_thumbstickAction = XR_NULL_HANDLE;
		m_thumbstickClickAction = m_primaryAction = m_secondaryAction = m_menuAction = XR_NULL_HANDLE;

		if (m_swapchain != XR_NULL_HANDLE)
		{
			xrDestroySwapchain(m_swapchain);
			m_swapchain = XR_NULL_HANDLE;
		}
		if (m_appSpace != XR_NULL_HANDLE)
		{
			xrDestroySpace(m_appSpace);
			m_appSpace = XR_NULL_HANDLE;
		}
		if (m_session != XR_NULL_HANDLE)
		{
			xrDestroySession(m_session);
			m_session = XR_NULL_HANDLE;
		}
		if (m_instance != XR_NULL_HANDLE)
		{
			xrDestroyInstance(m_instance);
			m_instance = XR_NULL_HANDLE;
		}
		m_systemId = XR_NULL_SYSTEM_ID;
		m_sessionState = XR_SESSION_STATE_UNKNOWN;
		m_sessionRunning = false;
	}

	void CPicoOpenXR::PollEvents()
	{
		XrEventDataBuffer event = { XR_TYPE_EVENT_DATA_BUFFER };
		while (xrPollEvent(m_instance, &event) == XR_SUCCESS)
		{
			switch (event.type)
			{
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
			{
				auto stateEvent = (const XrEventDataSessionStateChanged*)&event;
				m_sessionState = stateEvent->state;
				XR_LOGI("session state: %d", (int)m_sessionState);

				if (m_sessionState == XR_SESSION_STATE_READY)
				{
					XrSessionBeginInfo beginInfo = { XR_TYPE_SESSION_BEGIN_INFO };
					beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
					m_sessionRunning = XrCheck(m_instance, xrBeginSession(m_session, &beginInfo), "xrBeginSession");
				}
				else if (m_sessionState == XR_SESSION_STATE_STOPPING)
				{
					xrEndSession(m_session);
					m_sessionRunning = false;
				}
				else if (m_sessionState == XR_SESSION_STATE_EXITING || m_sessionState == XR_SESSION_STATE_LOSS_PENDING)
				{
					m_sessionRunning = false;
					m_exitRequested = true;
				}
				break;
			}
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
				m_sessionRunning = false;
				m_exitRequested = true;
				break;
			default:
				break;
			}
			event = { XR_TYPE_EVENT_DATA_BUFFER };
		}
	}

	bool CPicoOpenXR::Frame(const RenderCallback& renderFunc)
	{
		PollEvents();

		if (!m_sessionRunning)
		{
			// keep NPL and game logic running while the headset is idle, but do not spin.
			renderFunc(nullptr);
			usleep(10000);
			return false;
		}

		XrFrameWaitInfo waitInfo = { XR_TYPE_FRAME_WAIT_INFO };
		XrFrameState frameState = { XR_TYPE_FRAME_STATE };
		if (!XrCheck(m_instance, xrWaitFrame(m_session, &waitInfo, &frameState), "xrWaitFrame"))
			return false;

		XrFrameBeginInfo beginInfo = { XR_TYPE_FRAME_BEGIN_INFO };
		if (!XrCheck(m_instance, xrBeginFrame(m_session, &beginInfo), "xrBeginFrame"))
			return false;

		XrCompositionLayerProjectionView projectionViews[2];
		XrCompositionLayerProjection layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		const XrCompositionLayerBaseHeader* layers[1] = { (const XrCompositionLayerBaseHeader*)&layer };
		uint32_t layerCount = 0;

		if (frameState.shouldRender)
		{
			if (RenderViews(frameState.predictedDisplayTime, renderFunc, projectionViews))
			{
				layer.space = m_appSpace;
				layer.viewCount = 2;
				layer.views = projectionViews;
				layerCount = 1;
			}
		}
		else
		{
			renderFunc(nullptr);
		}

		XrFrameEndInfo endInfo = { XR_TYPE_FRAME_END_INFO };
		endInfo.displayTime = frameState.predictedDisplayTime;
		endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
		endInfo.layerCount = layerCount;
		endInfo.layers = layers;
		XrCheck(m_instance, xrEndFrame(m_session, &endInfo), "xrEndFrame");
		return layerCount > 0;
	}

	bool CPicoOpenXR::RenderViews(XrTime displayTime, const RenderCallback& renderFunc, void* pProjectionViews)
	{
		auto projectionViews = (XrCompositionLayerProjectionView*)pProjectionViews;

		XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
		locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		locateInfo.displayTime = displayTime;
		locateInfo.space = m_appSpace;
		XrViewState viewState = { XR_TYPE_VIEW_STATE };
		XrView views[2] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };
		uint32_t viewCount = 0;
		if (XR_FAILED(xrLocateViews(m_session, &locateInfo, &viewState, 2, &viewCount, views)) ||
			(viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0)
		{
			renderFunc(nullptr);
			return false;
		}

		PicoXRFrameData data;
		memset(&data, 0, sizeof(data));
		data.time = (int)(displayTime / 1000000);
		data.viewCount = 2;
		for (int eye = 0; eye < 2; ++eye)
		{
			const XrPosef& pose = views[eye].pose;
			data.viewport[eye][0] = (float)(eye * m_eyeWidth);
			data.viewport[eye][1] = 0.f;
			data.viewport[eye][2] = (float)m_eyeWidth;
			data.viewport[eye][3] = (float)m_eyeHeight;
			data.eyePosition[eye][0] = pose.position.x;
			data.eyePosition[eye][1] = pose.position.y;
			data.eyePosition[eye][2] = pose.position.z;
			data.eyeOrientation[eye][0] = pose.orientation.x;
			data.eyeOrientation[eye][1] = pose.orientation.y;
			data.eyeOrientation[eye][2] = pose.orientation.z;
			data.eyeOrientation[eye][3] = pose.orientation.w;
			data.eyeFov[eye][0] = views[eye].fov.angleLeft;
			data.eyeFov[eye][1] = views[eye].fov.angleRight;
			data.eyeFov[eye][2] = views[eye].fov.angleUp;
			data.eyeFov[eye][3] = views[eye].fov.angleDown;
		}
		UpdateControllers(displayTime, data);

		uint32_t imageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		if (!XrCheck(m_instance, xrAcquireSwapchainImage(m_swapchain, &acquireInfo, &imageIndex), "xrAcquireSwapchainImage"))
		{
			renderFunc(nullptr);
			return false;
		}
		XrSwapchainImageWaitInfo imageWaitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		imageWaitInfo.timeout = XR_INFINITE_DURATION;
		xrWaitSwapchainImage(m_swapchain, &imageWaitInfo);

		// the engine renders into whatever framebuffer is bound, render targets restore it afterwards.
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[imageIndex]);
		glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
		if (m_disableSRGBWrite)
			glDisable(GL_FRAMEBUFFER_SRGB_EXT);

		bool rendered = renderFunc(&data);

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[imageIndex]);
		DumpFrameIfRequested(GetRenderWidth(), GetRenderHeight());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		XrSwapchainImageReleaseInfo releaseInfo = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		xrReleaseSwapchainImage(m_swapchain, &releaseInfo);

		for (int eye = 0; eye < 2; ++eye)
		{
			projectionViews[eye] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
			projectionViews[eye].pose = views[eye].pose;
			projectionViews[eye].fov = views[eye].fov;
			projectionViews[eye].subImage.swapchain = m_swapchain;
			projectionViews[eye].subImage.imageRect.offset.x = eye * m_eyeWidth;
			projectionViews[eye].subImage.imageRect.offset.y = 0;
			projectionViews[eye].subImage.imageRect.extent.width = m_eyeWidth;
			projectionViews[eye].subImage.imageRect.extent.height = m_eyeHeight;
			projectionViews[eye].subImage.imageArrayIndex = 0;
		}
		return rendered;
	}

	void CPicoOpenXR::UpdateControllers(XrTime displayTime, PicoXRFrameData& data)
	{
		XrActiveActionSet activeActionSet = { m_actionSet, XR_NULL_PATH };
		XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
		syncInfo.countActiveActionSets = 1;
		syncInfo.activeActionSets = &activeActionSet;
		// returns XR_SESSION_NOT_FOCUSED while the system menu is shown, all actions are inactive then.
		xrSyncActions(m_session, &syncInfo);

		for (int hand = 0; hand < 2; ++hand)
		{
			XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
			getInfo.subactionPath = m_handPath[hand];

			getInfo.action = m_aimPoseAction;
			XrActionStatePose poseState = { XR_TYPE_ACTION_STATE_POSE };
			xrGetActionStatePose(m_session, &getInfo, &poseState);
			if (poseState.isActive)
			{
				XrSpaceLocation location = { XR_TYPE_SPACE_LOCATION };
				if (XR_SUCCEEDED(xrLocateSpace(m_handSpace[hand], m_appSpace, displayTime, &location)) &&
					(location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
				{
					data.handActive[hand] = true;
					data.handPosition[hand][0] = location.pose.position.x;
					data.handPosition[hand][1] = location.pose.position.y;
					data.handPosition[hand][2] = location.pose.position.z;
					data.handOrientation[hand][0] = location.pose.orientation.x;
					data.handOrientation[hand][1] = location.pose.orientation.y;
					data.handOrientation[hand][2] = location.pose.orientation.z;
					data.handOrientation[hand][3] = location.pose.orientation.w;
				}
			}

			auto getFloat = [&](XrAction action) {
				getInfo.action = action;
				XrActionStateFloat state = { XR_TYPE_ACTION_STATE_FLOAT };
				xrGetActionStateFloat(m_session, &getInfo, &state);
				return state.isActive ? state.currentState : 0.f;
			};
			auto getBool = [&](XrAction action) {
				getInfo.action = action;
				XrActionStateBoolean state = { XR_TYPE_ACTION_STATE_BOOLEAN };
				xrGetActionStateBoolean(m_session, &getInfo, &state);
				return (state.isActive && state.currentState) ? 1 : 0;
			};

			data.handKey[hand][0] = getFloat(m_triggerAction) > 0.5f ? 1 : 0;
			data.handKey[hand][1] = getFloat(m_squeezeAction) > 0.5f ? 1 : 0;
			data.handKey[hand][2] = 0; // no touchpad
			data.handKey[hand][3] = getBool(m_thumbstickClickAction);
			data.handKey[hand][4] = getBool(m_primaryAction);
			data.handKey[hand][5] = getBool(m_secondaryAction);
			data.handKey[hand][6] = getBool(m_menuAction);

			getInfo.action = m_thumbstickAction;
			XrActionStateVector2f stick = { XR_TYPE_ACTION_STATE_VECTOR2F };
			xrGetActionStateVector2f(m_session, &getInfo, &stick);
			if (stick.isActive)
			{
				data.handAxes[hand][2] = stick.currentState.x;
				// OpenXR thumbstick y is positive upward, WebXR gamepad y is positive downward
				data.handAxes[hand][3] = -stick.currentState.y;
			}
		}
	}
}
