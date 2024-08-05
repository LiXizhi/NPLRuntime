#pragma once
#include "Framework/InputSystem/VirtualKey.h"

#define L_PRESSED(x) (((x)->m_dims2.rgbButtons[0]&0x80)!=0)
#define R_PRESSED(x) (((x)->m_dims2.rgbButtons[1]&0x80)!=0)
#define M_PRESSED(x) (((x)->m_dims2.rgbButtons[2]&0x80)!=0)

#define SAMPLE_BUFFER_SIZE 1024

namespace ParaEngine
{
	struct MouseEventDeviceObject {
		DWORD       dwOfs;
		DWORD       dwData;
		DWORD       dwTimeStamp;
		DWORD       dwSequence;
		float x;
		float y;
	};

	/** base class for GUI mouse
	*/
	class CGUIMouseVirtual
	{
	public:
		CGUIMouseVirtual();
		virtual ~CGUIMouseVirtual();
	public:
		virtual HRESULT ReadBufferedData();
		virtual HRESULT ReadImmediateData();
		virtual void Update();
		/** reset mouse to make the mouse delta correct in the next frame. */
		virtual void ResetLastMouseState();

		virtual void Reset();
		virtual bool IsButtonDown(const EMouseButton nMouseButton);
		/** the current mouse wheel delta in steps, such as -2,-1,0,1,2 */
		virtual int	 GetMouseWheelDeltaSteps();
		/** the current mouse X delta in steps, such as -2,-1,0,1,2 */
		virtual int	 GetMouseXDeltaSteps();
		/** the current mouse Y delta in steps, such as -2,-1,0,1,2 */
		virtual int	 GetMouseYDeltaSteps();

		virtual bool IsLocked();
		/** true to lock the mouse at its current location*/
		virtual void SetLock(bool bLock);

		virtual void UpdateX(int delta);
		virtual void UpdateY(int delta);


		/** push a standard windows mouse event to the buffer for processing in the next frame move. */
		virtual void PushMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
		/** push a standard windows mouse event to the buffer for processing in the next frame move. */
		virtual void PushMouseEvent(const MSG &msg);
		/** push a standard windows mouse event to the buffer for processing in the next frame move. */
		virtual void PushMouseEvent(const DeviceMouseEventPtr& msg);

		virtual void SetCapture(CGUIBase* obj);
		virtual void ReleaseCapture(CGUIBase* obj);
		virtual CGUIBase* GetCapture();
		virtual void ReleaseCapture();

		/** whether to use window message for buffered mouse event. default to true, if false, directInput will be used. */
		virtual bool IsUseWindowsMessage();
		/** whether to use window message for buffered mouse event. default to true, if false, directInput will be used. */
		virtual void SetUseWindowsMessage(bool bUseWinMsg);

		/** true to show the cursor. only show if previous call is hide and vice versa. */
		virtual void ShowCursor(bool bShowCursor){};

		/** Force the device to show its cursor. this is different from ShowCursor in that it always calls the device API to set the cursor. */
		virtual void ForceShowCursor(bool bShow){};

		/** get the cursor size in pixels */
		virtual int GetCursorSize(){ return 32; };

		/** swap left/right button.*/
		virtual bool IsMouseButtonSwapped();
		virtual void SetMouseButtonSwapped(bool bSwapped);

		/** get current cursor position. */
		virtual void GetDeviceCursorPos(int& x, int&y);
		virtual void SetDeviceCursorPos(int x, int y);
		/** only change m_x, m_y, which is pretty fast. */
		virtual void SetMousePosition(int x, int y);

		/** check if there is any unprocessed buffered window mouse message. we call this function before Update() to check if there is any mouse event*/
		virtual int GetBufferedMessageCount();;
	protected:

		bool	m_bLock;
		/** whether to use window message for buffered mouse event. default to true, if false, directInput will be used. */
		bool	m_bUseWindowMessage;
		/** swap left/right button.*/
		bool	m_bSwapMouseButton;
		bool m_bLastMouseReset;
	public:
		MouseEventDeviceObject	m_didod[SAMPLE_BUFFER_SIZE];  // Receives buffered data 
		DWORD				m_dwElements;
		DIMOUSESTATE2		m_dims2;   // DirectInput Mouse state buffer 
		DIMOUSESTATE2		m_lastMouseState; 
		DIMOUSESTATE2		m_curMouseState;
		int					m_x, m_y;	//coordinate of the current mouse position

		MSG					m_buffered_mouse_msgs[SAMPLE_BUFFER_SIZE / 2];
		int					m_buffered_mouse_msgs_count;
		
		bool  m_isTouchInputting;
		
		CGUIBase *		m_objCaptured;
	};
}
