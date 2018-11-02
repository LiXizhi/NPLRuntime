#pragma once
#include "Framework/InputSystem/VirtualKey.h"
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

		/** return true if the key is being pressed at the time of calling.
		* @param nKey: a direct input key, such as DIK_SPACE. */
		virtual bool IsKeyPressed(const EVirtualKey& key);
		virtual void SetKeyPressed(const EVirtualKey& key, bool bPressed);
		virtual void Reset();
		
		/** push a standard key event to the buffer for processing in the next frame move. */
		virtual void PushKeyEvent(const DeviceKeyEvent& msg);
		/** read hardware buffer and immediate keyboard states to internal data structure. */
		virtual void Update();

		virtual EKeyState GetKeyState(const EVirtualKey& key);
		inline EKeyState* GetKeyStates(){ return m_keystate; }

		virtual HRESULT ReadBufferedData();
		virtual HRESULT ReadImmediateData();

		virtual EKeyState GetLastKeyState(int nIndex);
		virtual EKeyState GetCurrentKeyState(int nIndex);
		
		DWORD GetElementsCount() const;
		void SetElementsCount(DWORD val);

		inline DIDEVICEOBJECTDATA* GetDeviceObjectData() {
			return m_didod;
		}

	public:
		static void SetCapture(CGUIBase* obj);
		static void ReleaseCapture();
		static void ReleaseCapture(CGUIBase* obj);
		static CGUIBase* GetCapture();

	protected:
		DWORD              m_dwElements;
		EKeyState		   m_keystate[(int)EVirtualKey::COUNT];	
		// last DirectInput keyboard state buffer 
		EKeyState		   m_lastkeystate[(int)EVirtualKey::COUNT];   
		DIDEVICEOBJECTDATA m_didod[SAMPLE_BUFFER_SIZE];  // Receives buffered data 

		DeviceKeyEvent	m_buffered_key_msgs[SAMPLE_BUFFER_SIZE / 2];
		int					m_buffered_key_msgs_count;

		static CGUIBase *		m_objCaptured;
	};
}