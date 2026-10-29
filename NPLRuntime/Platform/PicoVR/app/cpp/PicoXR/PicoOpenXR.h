//-----------------------------------------------------------------------------
// PicoOpenXR.h
// Authors: big
// CreateDate: 2026.10.28
// Desc: OpenXR (GLES) session for PICO headsets. Renders the engine side by side
// into one swapchain image and submits it as a stereo projection layer.
//-----------------------------------------------------------------------------

#pragma once

#include <openxr/openxr.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace ParaEngine
{
	/** per-frame head, eye and controller state. Same conventions as WebXR:
	* local reference space, right-handed, y up, quaternions as (x, y, z, w). */
	struct PicoXRFrameData
	{
		int time; // predicted display time in ms
		int viewCount;
		float viewport[2][4]; // x, y, width, height inside the render window
		float eyePosition[2][3];
		float eyeOrientation[2][4];
		float eyeFov[2][4]; // angleLeft, angleRight, angleUp, angleDown in radians
		bool handActive[2]; // 0 left, 1 right
		float handPosition[2][3]; // aim (target ray) pose
		float handOrientation[2][4];
		// xr-standard gamepad layout: trigger, squeeze, touchpad, thumbstick, A/X, B/Y, menu
		int handKey[2][7];
		// touchpad x, y, thumbstick x, y. y is positive downward like WebXR gamepads.
		float handAxes[2][4];
	};

	class CPicoOpenXR
	{
	public:
		/** draw one engine frame. data is null when the headset does not show this frame (session idle, etc).
		* return false if nothing was drawn. */
		typedef std::function<bool(const PicoXRFrameData* data)> RenderCallback;

		static CPicoOpenXR& GetInstance();

		/** create instance, session and swapchain. must be called on the GL thread with an OpenGL ES 3 context current.
		* @param javaVM: JavaVM*
		* @param activity: jobject of the android activity */
		bool Init(void* javaVM, void* activity);
		void Shutdown();

		bool IsInited() const { return m_session != XR_NULL_HANDLE; }
		bool IsSessionRunning() const { return m_sessionRunning; }
		bool IsExitRequested() const { return m_exitRequested; }

		/** size of the side by side render target: two eyes horizontally */
		int GetRenderWidth() const { return m_eyeWidth * 2; }
		int GetRenderHeight() const { return m_eyeHeight; }

		/** poll XR events and tick the engine once through renderFunc.
		* returns true if a frame was submitted to the headset. */
		bool Frame(const RenderCallback& renderFunc);

	private:
		CPicoOpenXR();
		~CPicoOpenXR();

		bool CreateSwapchain();
		bool CreateActions();
		void PollEvents();
		bool RenderViews(XrTime displayTime, const RenderCallback& renderFunc, void* projectionViews);
		void UpdateControllers(XrTime displayTime, PicoXRFrameData& data);

	private:
		XrInstance m_instance;
		XrSystemId m_systemId;
		XrSession m_session;
		XrSpace m_appSpace;
		XrSessionState m_sessionState;
		bool m_sessionRunning;
		bool m_exitRequested;
		bool m_hasBDController;

		int m_eyeWidth;
		int m_eyeHeight;
		XrSwapchain m_swapchain;
		int64_t m_swapchainFormat;
		bool m_disableSRGBWrite;
		std::vector<uint32_t> m_swapchainTextures;
		std::vector<uint32_t> m_framebuffers;
		uint32_t m_depthBuffer;

		XrActionSet m_actionSet;
		XrAction m_aimPoseAction;
		XrAction m_triggerAction;
		XrAction m_squeezeAction;
		XrAction m_thumbstickAction;
		XrAction m_thumbstickClickAction;
		XrAction m_primaryAction;
		XrAction m_secondaryAction;
		XrAction m_menuAction;
		XrPath m_handPath[2];
		XrSpace m_handSpace[2];
	};
}
