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
#ifdef PLATFORM_SDL2
#include "SDL2/SDL.h"
#endif

using namespace ParaEngine;

CGUIBase * CGUIKeyboardVirtual::m_objCaptured = NULL;

ParaEngine::CGUIKeyboardVirtual::CGUIKeyboardVirtual()
	:m_buffered_key_msgs_count(0), m_dwElements(0)
{
	memset(m_didod, 0, sizeof(m_didod));
	memset(m_lastkeystate, 0, sizeof(m_lastkeystate));
	memset(m_keystate, 0, sizeof(m_keystate));
	Reset();
}

ParaEngine::CGUIKeyboardVirtual::~CGUIKeyboardVirtual()
{

}

bool CGUIKeyboardVirtual::IsKeyPressed(const EVirtualKey& key)
{
	if (key == EVirtualKey::KEY_UNKNOWN || key == EVirtualKey::COUNT) return false;
	return m_keystate[(int)key] == EKeyState::PRESS;
}

void ParaEngine::CGUIKeyboardVirtual::SetKeyPressed(const EVirtualKey& key, bool bPressed)
{
	if (key == EVirtualKey::KEY_UNKNOWN || key == EVirtualKey::COUNT) return;
	m_keystate[(int)key] = bPressed ? EKeyState::PRESS : EKeyState::RELEASE;
}

void CGUIKeyboardVirtual::Reset()
{
	for (int i =0;i<(int)EVirtualKey::COUNT;i++)
	{
		m_keystate[i] = EKeyState::RELEASE;
	}
	m_dwElements = 0;
}

CGUIBase* ParaEngine::CGUIKeyboardVirtual::GetCapture()
{
	return m_objCaptured;
}

void ParaEngine::CGUIKeyboardVirtual::PushKeyEvent(const DeviceKeyEvent& e)
{
	if (m_buffered_key_msgs_count < SAMPLE_BUFFER_SIZE / 2) {
		m_buffered_key_msgs[m_buffered_key_msgs_count] = e;
	}
	++m_buffered_key_msgs_count;
}

void ParaEngine::CGUIKeyboardVirtual::Update()
{
	ReadBufferedData();
	ReadImmediateData();
}

EKeyState ParaEngine::CGUIKeyboardVirtual::GetKeyState(const EVirtualKey& key)
{
	assert(key != EVirtualKey::KEY_UNKNOWN && key != EVirtualKey::COUNT);
	return m_keystate[(int)key];
}

HRESULT ParaEngine::CGUIKeyboardVirtual::ReadBufferedData()
{
	/** we do not use Read Buffered Data, instead, mouse events are translated from windows messages.
	Only immediate button data are read from direct input. */
	m_dwElements = 0;
	//translating windows message into DirextMouse-like events, in order to maintain consistency of the interface
	for (int a = 0; a<m_buffered_key_msgs_count; a++) {
		m_didod[m_dwElements].dwOfs = 0;
		switch (m_buffered_key_msgs[a].m_state) {
		case EKeyState::PRESS:
			m_didod[m_dwElements].dwData = 0x80;
			m_didod[m_dwElements].dwOfs = (DWORD)m_buffered_key_msgs[a].m_virtualKey;
			break;
		case EKeyState::RELEASE:
			m_didod[m_dwElements].dwData = 0;
			m_didod[m_dwElements].dwOfs = (DWORD)m_buffered_key_msgs[a].m_virtualKey;
			break;
		}
		if (m_didod[m_dwElements].dwOfs != 0)
		{
			m_didod[m_dwElements].dwTimeStamp = (DWORD)m_buffered_key_msgs[a].m_nTime;
			m_dwElements++;
		}
	}
	m_buffered_key_msgs_count = 0;
	return S_OK;
}

HRESULT ParaEngine::CGUIKeyboardVirtual::ReadImmediateData()
{
	memcpy(m_lastkeystate, m_keystate, sizeof(m_lastkeystate));
#ifdef PLATFORM_SDL2
	SDL_Keymod keymod = SDL_GetModState();
	m_lastkeystate[(int)EVirtualKey::KEY_NUMLOCK] = (keymod & KMOD_NUM) == 0 ? EKeyState::RELEASE : EKeyState::PRESS;
	m_lastkeystate[(int)EVirtualKey::KEY_SCROLL] = (keymod & KMOD_SCROLL) == 0 ? EKeyState::RELEASE : EKeyState::PRESS;
	m_lastkeystate[(int)EVirtualKey::KEY_CAPITAL] = (keymod & KMOD_CAPS) == 0 ? EKeyState::RELEASE : EKeyState::PRESS;
#endif
	return S_OK;
}

EKeyState ParaEngine::CGUIKeyboardVirtual::GetLastKeyState(int nIndex)
{
	return m_lastkeystate[nIndex];
}

EKeyState ParaEngine::CGUIKeyboardVirtual::GetCurrentKeyState(int nIndex)
{
	return m_keystate[nIndex];
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