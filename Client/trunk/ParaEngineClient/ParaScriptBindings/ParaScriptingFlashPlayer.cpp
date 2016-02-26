//----------------------------------------------------------------------
// ParaScriptingGUI
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2005.9.8
// Desc: 
// Containing the interface between script and the engine. 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#if defined (USE_FLASH_MANAGER) && defined(USE_DIRECTX_RENDERER)
#include "ParaWorldAsset.h"
#include "FlashTextureManager.h"
#include "ParaScriptingFlashPlayer.h"

using namespace ParaScripting;

bool ParaFlashPlayer::IsValid()
{
	return m_nFlashPlayerIndex >= 0 && (CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex) != 0);
}

const char* ParaFlashPlayer::GetFileName()
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->GetFileName();
	}
	return NULL;
}

bool ParaFlashPlayer::IsFree()
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->IsFree();
	}
	return false;
}

void ParaFlashPlayer::LoadMovie(const char* filename)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->LoadMovie(filename);
	}
}

void ParaFlashPlayer::UnloadMovie()
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->UnloadMovie();
	}
}

void ParaFlashPlayer::InvalidateDeviceObjects()
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->InvalidateDeviceObjects();
	}
}

void ParaFlashPlayer::GetTextureInfo(int widthIn, int heightIn, int& width, int& height)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		pFlashPlayer->GetTextureInfo(&widthIn, &heightIn);
	}
	width = widthIn;
	height = heightIn;
}

bool ParaFlashPlayer::IsWindowMode()
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->IsWindowMode();
	}
	return false;
}

void ParaFlashPlayer::SetWindowMode(bool bIsWindowMode)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		pFlashPlayer->SetWindowMode(bIsWindowMode);
	}
}

void ParaFlashPlayer::MoveWindow(int x, int y, int width, int height)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		pFlashPlayer->MoveWindow(x, y, width, height);
	}
}

int ParaFlashPlayer::SendMessage(unsigned int Msg, DWORD wParam, DWORD lParam)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		switch (Msg)
		{
		case 0:
		{
			/*
			* mouse click is simulated by sending WM_LBUTTONDOWN and WM_LBUTTONUP messages to the client window
			* Because the game engine app always captures the mouse, the single click function will actually
			* send a pair of Left button down and up message to the flash control.
			* The first click set the mouse focus and the second performs the actual click. Then the flash control will soon lose mouse capture to the game engine.
			*/

			/// this will set the focus
			pFlashPlayer->SetFocus();

			/// the first click set the focus from the game engine to the flash control
			// for unknown reason, PostMessage will crash the application. 
			pFlashPlayer->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(wParam, lParam));
			pFlashPlayer->SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(wParam, lParam));
			/// the second click performs the actual click. 
			pFlashPlayer->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(wParam, lParam));
			pFlashPlayer->SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(wParam, lParam));
			break;
		}
		case 1:
		{
			//pFlashPlayer->PostMessage(WM_MOUSEMOVE, 0, MAKELPARAM(wParam, lParam));
			break;
		}
		default:
			break;
		}
	}
	return 0;
}

void ParaFlashPlayer::PutFlashVars(const char* args)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		pFlashPlayer->PutFlashVars(args);
	}
}

const char* ParaFlashPlayer::CallFlashFunction(const char* sFunc)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		return pFlashPlayer->CallFlashFunction(sFunc);
	}
	return NULL;
}

void ParaFlashPlayer::SetFlashReturnValue(const char* sResult)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(m_nFlashPlayerIndex);
	if (pFlashPlayer)
	{
		pFlashPlayer->SetFlashReturnValue(sResult);
	}
}

#endif
