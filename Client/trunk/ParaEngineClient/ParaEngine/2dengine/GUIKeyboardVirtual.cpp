//-----------------------------------------------------------------------------
// Class: GUI keyboard
// Authors:	LiXizhi
// Emails:	
// Date: 2014.11.18
// Desc:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EventBinding.h"
#include "GUIKeyboardVirtual.h"

using namespace ParaEngine;

CGUIBase * CGUIKeyboardVirtual::m_objCaptured = NULL;

ParaEngine::CGUIKeyboardVirtual::CGUIKeyboardVirtual()
	:m_buffered_key_msgs_count(0)
{
	m_bUseWindowMessage = true;
	memset(m_didod, 0, sizeof(m_didod));
	memset(m_lastkeystate, 0, sizeof(m_lastkeystate));
	memset(m_keystate, 0, sizeof(m_keystate));
	Reset();
}

ParaEngine::CGUIKeyboardVirtual::~CGUIKeyboardVirtual()
{

}

bool CGUIKeyboardVirtual::IsKeyPressed(DWORD nKey)
{
	return ((m_keystate[nKey] & 0x80) != 0);
}

void ParaEngine::CGUIKeyboardVirtual::SetKeyPressed(DWORD nKey, bool bPressed)
{
	m_keystate[nKey] = bPressed ? 0x80 : 0;
}

void CGUIKeyboardVirtual::Reset()
{
	::memset(m_keystate, 0, sizeof(m_keystate));
	m_dwElements = 0;
}

void CGUIKeyboardVirtual::PushKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_buffered_key_msgs_count < SAMPLE_BUFFER_SIZE / 2) {
		m_buffered_key_msgs[m_buffered_key_msgs_count].lParam = lParam;
		m_buffered_key_msgs[m_buffered_key_msgs_count].wParam = wParam;
		m_buffered_key_msgs[m_buffered_key_msgs_count].time = GetTickCount();
		m_buffered_key_msgs[m_buffered_key_msgs_count].message = uMsg;
	}
	++m_buffered_key_msgs_count;
}

void CGUIKeyboardVirtual::PushKeyEvent(const MSG &msg)
{
	if (m_buffered_key_msgs_count < SAMPLE_BUFFER_SIZE / 2) {
		m_buffered_key_msgs[m_buffered_key_msgs_count] = msg;
	}
	++m_buffered_key_msgs_count;
}

CGUIBase* ParaEngine::CGUIKeyboardVirtual::GetCapture()
{
	return m_objCaptured;
}

HRESULT ParaEngine::CGUIKeyboardVirtual::ReadBufferedData()
{
	if (m_bUseWindowMessage)
	{
		/** we do not use Read Buffered Data, instead, mouse events are translated from windows messages.
		Only immediate button data are read from direct input. */
		m_dwElements = 0;
		//translating windows message into DirextMouse-like events, in order to maintain consistency of the interface
		for (int a = 0; a<m_buffered_key_msgs_count; a++) {
			m_didod[m_dwElements].dwOfs = 0;
			switch (m_buffered_key_msgs[a].message) {
			case WM_KEYDOWN:
				m_didod[m_dwElements].dwData = 0x80;
				m_didod[m_dwElements].dwOfs = CEventBinding::TranslateVKToDIK(m_buffered_key_msgs[a].wParam);
				break;
			case WM_KEYUP:
				m_didod[m_dwElements].dwData = 0;
				m_didod[m_dwElements].dwOfs = CEventBinding::TranslateVKToDIK(m_buffered_key_msgs[a].wParam);
				break;
			}
			if (m_didod[m_dwElements].dwOfs != 0)
			{
				m_didod[m_dwElements].dwTimeStamp = m_buffered_key_msgs[a].time;
				m_dwElements++;
			}
		}
		m_buffered_key_msgs_count = 0;
	}
	return S_OK;
}

void ParaEngine::CGUIKeyboardVirtual::Update()
{
	ReadBufferedData();
	ReadImmediateData();
}

HRESULT ParaEngine::CGUIKeyboardVirtual::ReadImmediateData()
{
	memcpy(m_lastkeystate, m_keystate, sizeof(m_lastkeystate));
	return S_OK;
}

BYTE ParaEngine::CGUIKeyboardVirtual::GetLastKeyState(int nIndex)
{
	return m_lastkeystate[nIndex];
}


BYTE ParaEngine::CGUIKeyboardVirtual::GetCurrentKeyState(int nIndex)
{
	return m_keystate[nIndex];
}

short ParaEngine::CGUIKeyboardVirtual::GetKeyState(int nIndex)
{
#ifdef WIN32
	return ::GetKeyState(nIndex);
#else
	return m_keystate[nIndex];
#endif
}

void ParaEngine::CGUIKeyboardVirtual::SetElementsCount(DWORD val)
{
	m_dwElements = val;
}

DWORD ParaEngine::CGUIKeyboardVirtual::GetElementsCount() const
{
	return m_dwElements;
}


void CGUIKeyboardVirtual::SetCapture(CGUIBase* obj)
{
	m_objCaptured = obj;
}
void CGUIKeyboardVirtual::ReleaseCapture(CGUIBase* obj)
{
	if (m_objCaptured == obj) {
		m_objCaptured = NULL;
	}
}
void CGUIKeyboardVirtual::ReleaseCapture()
{
	m_objCaptured = NULL;
}


bool CGUIKeyboardVirtual::IsUseWindowsMessage()
{
	return m_bUseWindowMessage;
}

void CGUIKeyboardVirtual::SetUseWindowsMessage(bool bUseWinMsg)
{
	m_bUseWindowMessage = bUseWinMsg;
}
