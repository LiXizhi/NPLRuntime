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
{
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




EKeyState ParaEngine::CGUIKeyboardVirtual::GetKeyState(const EVirtualKey& key)
{
	assert(key != EVirtualKey::KEY_UNKNOWN && key != EVirtualKey::COUNT);
	return m_keystate[(int)key];
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