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

		
		virtual EKeyState GetKeyState(const EVirtualKey& key);
		inline EKeyState* GetKeyStates(){ return m_keystate; }

		DWORD GetElementsCount() const;
		void SetElementsCount(DWORD val);

	public:
		static void SetCapture(CGUIBase* obj);
		static void ReleaseCapture();
		static void ReleaseCapture(CGUIBase* obj);
		static CGUIBase* GetCapture();

	protected:
		DWORD              m_dwElements;
		EKeyState		   m_keystate[(int)EVirtualKey::COUNT];	
		static CGUIBase *		m_objCaptured;
	};
}