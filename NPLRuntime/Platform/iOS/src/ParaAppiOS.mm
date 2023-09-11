//-----------------------------------------------------------------------------
// Class: ParaAppiOS.mm
// Authors: big
// CreateDate: 2021.12.13
//-----------------------------------------------------------------------------

#import "KeyboardiOS.h"

#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"
#include "MidiPlayer/ParaEngineMediaPlayer.h"
#include "MidiMsg.h"

IParaEngineApp* CreateParaEngineApp()
{
    return new CParaEngineAppiOS();
}

namespace ParaEngine {
    CParaEngineAppiOS::CParaEngineAppiOS()
    {
        SetTouchInputting(true);
        CMidiMsg::GetSingleton().SetMediaPlayer(ParaEngineMediaPlayer::GetSingleton());
    }

    void CParaEngineAppiOS::GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
    {
        if (bInBackbuffer)
        {
            inout_x = (int)(inout_x / [UIScreen mainScreen].scale);
            inout_y = (int)(inout_y / [UIScreen mainScreen].scale);
        }

    }

    void CParaEngineAppiOS::ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
    {
        if (bInBackbuffer)
        {
            inout_x = (int)(inout_x * [UIScreen mainScreen].scale);
            inout_y = (int)(inout_y * [UIScreen mainScreen].scale);
        }
    }

    void CParaEngineAppiOS::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
    {
        //throw std::logic_error("The method or operation is not implemented.");
    }

    bool CParaEngineAppiOS::AppHasFocus()
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    void CParaEngineAppiOS::GetStats(string& output, DWORD dwFields)
    {
        //throw std::logic_error("The method or operation is not implemented.");
    }

    bool CParaEngineAppiOS::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    void CParaEngineAppiOS::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
    {
        //throw std::logic_error("The method or operation is not implemented.");
    }

    const char* CParaEngineAppiOS::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return "";
    }

    bool CParaEngineAppiOS::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    DWORD CParaEngineAppiOS::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return  0;
    }

    bool CParaEngineAppiOS::GetAutoLowerFrameRateWhenNotFocused()
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    void CParaEngineAppiOS::SetToggleSoundWhenNotFocused(bool bEnabled)
    {
        //    throw std::logic_error("The method or operation is not implemented.");
        
    }

    bool CParaEngineAppiOS::GetToggleSoundWhenNotFocused()
    {
        //throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    int CParaEngineAppiOS::Run(HINSTANCE hInstance)
    {
        return 0;
    }

    void CParaEngineAppiOS::setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom, const string& editParams)
    {
        const char *inputType = "text";
        int x = 0;
        this->GameToClient(x, ctrlBottom);
        
        CGUIEditBox *pGUI = dynamic_cast<CGUIEditBox*>((CGUIRoot::GetInstance()->GetUIKeyFocus()));

        if (pGUI && bOpen) {
            inputType = pGUI->GetInputType();
        }

        NSString *_editParams = [NSString stringWithUTF8String: editParams.c_str()];
        NSString *_inputType = [NSString stringWithUTF8String: inputType];

        [KeyboardiOSController setIMEKeyboardState:bOpen bMoveView:bMoveView ctrlBottom:ctrlBottom editParams:_editParams inputType:_inputType];
    }

    void CParaEngineAppiOS::Exit(int nReturnCode /*= 0*/)
	{
        exit(0);
    }

    void CParaEngineAppiOS::GetVisibleSize(Vector2* pOut) {
        //1、得到当前屏幕的尺寸：
        CGRect rect_screen = [[UIScreen mainScreen] bounds];
        CGSize size_screen = rect_screen.size;

        //2、获得scale：iPhone5和iPhone6是2，iPhone6Plus是3
        CGFloat scale_screen = [UIScreen mainScreen].scale;
        NSLog(@"scale_screen:%.f", scale_screen);

        //3.获取当前屏幕的分辨率
        CGFloat widthResolution = size_screen.width * scale_screen;
        CGFloat heightResolution = size_screen.height * scale_screen;
        
        pOut->x = widthResolution;
        pOut->y = heightResolution;
    }
}
