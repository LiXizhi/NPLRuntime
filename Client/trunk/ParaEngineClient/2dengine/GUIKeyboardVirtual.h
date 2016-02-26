#pragma once

#define SAMPLE_BUFFER_SIZE 1024
#define KEY_PRESSED(x,y) (((x)[(y)]&0x80)!=0)

namespace ParaEngine
{
	class CGUIBase;

	/** base class for keyboard system. 
	*/
	class CGUIKeyboardVirtual
	{
	public:
		CGUIKeyboardVirtual();
		~CGUIKeyboardVirtual();

		/** whether to use window message for buffered mouse event. default to true, if false, directInput will be used. */
		virtual bool IsUseWindowsMessage();
		/** whether to use window message for buffered mouse event. default to true, if false, directInput will be used. */
		virtual void SetUseWindowsMessage(bool bUseWinMsg);

		/** push a standard windows key event to the buffer for processing in the next frame move. */
		virtual void PushKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
		/** push a standard windows key event to the buffer for processing in the next frame move. */
		virtual void PushKeyEvent(const MSG &msg);

		/** read hardware buffer and immediate keyboard states to internal data structure. */
		virtual void Update();

		/** return true if the key is being pressed at the time of calling.
		* @param nKey: a direct input key, such as DIK_SPACE. */
		virtual bool IsKeyPressed(DWORD nKey);
		virtual void SetKeyPressed(DWORD nKey, bool bPressed);
		virtual void Reset();

		virtual HRESULT ReadBufferedData();
		virtual HRESULT ReadImmediateData();
		
		virtual BYTE GetLastKeyState(int nIndex);
		virtual BYTE GetCurrentKeyState(int nIndex);
		virtual short GetKeyState(int nIndex);
		inline BYTE* GetKeyStates(){ return m_keystate; }

		DWORD GetElementsCount() const;
		void SetElementsCount(DWORD val);

		inline DIDEVICEOBJECTDATA* GetDeviceObjectData(){
			return m_didod;
		}
	public:
		static void SetCapture(CGUIBase* obj);
		static void ReleaseCapture();
		static void ReleaseCapture(CGUIBase* obj);
		static CGUIBase* GetCapture();

	protected:
		DWORD              m_dwElements;
		
		BYTE			   m_keystate[256];   // DirectInput keyboard state buffer 
		
		BYTE			   m_lastkeystate[256];   //last DirectInput keyboard state buffer 
		DIDEVICEOBJECTDATA m_didod[SAMPLE_BUFFER_SIZE];  // Receives buffered data 

		static CGUIBase *		m_objCaptured;
		/** whether to use window message for buffered mouse event. default to true, if false, directInput will be used. */
		bool					m_bUseWindowMessage;

		MSG						m_buffered_key_msgs[SAMPLE_BUFFER_SIZE / 2];
		int						m_buffered_key_msgs_count;
	};
}