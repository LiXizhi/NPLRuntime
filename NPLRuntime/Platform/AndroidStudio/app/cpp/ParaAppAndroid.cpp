//-----------------------------------------------------------------------------
// ParaAppAndroid.cpp
// Authors: YuanQuanwei, big
// CreateDate: 2018.3.29
// ModifyDate: 2023.8.22
//-----------------------------------------------------------------------------

#include "ParaAppAndroid.h"
#include "jni/ParaEngineGLSurfaceView.h"
#include "jni/ParaEngineMediaPlayer.h"
#include "3dengine/AudioEngine2.h"
#include "MidiMsg.h"
#include "Framework/Common/Bridge/LuaJavaBridge.h"
#include "jni/ParaEngineHelper.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

using namespace ParaEngine;

IParaEngineApp* CreateParaEngineApp()
{
    return new CParaEngineAppAndroid();
}

static std::string errorLog;
static int errorLogCount = 0;
static bool isLogInit = false;
struct sigaction act_old;
static std::string writablePath;

static _Unwind_Reason_Code unwind_backtrace_callback(struct _Unwind_Context* context, void* arg) {
    uintptr_t pc = _Unwind_GetIP(context);

    Dl_info info;
    if (dladdr((void *)pc, &info) != 0) {
        const char *outputLine;
        const char *mangled = info.dli_sname;
        int status;
        char *demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
        if (status == 0) {
            outputLine = demangled;
            free(demangled);
        } else {
            outputLine = info.dli_sname;
        }

        if (errorLogCount < 100) {
            if (outputLine != nullptr && strcmp(outputLine, "unwind_backtrace()") != 0) {
                errorLog = errorLog + outputLine + "\n";
            }

            errorLogCount++;
        }
    } else {
        LOGD("Unable to retrieve function info.\n");
    }

    return _URC_NO_REASON;
}

ssize_t unwind_backtrace() {
    _Unwind_Reason_Code rc = _Unwind_Backtrace(unwind_backtrace_callback, nullptr);

    return rc == _URC_END_OF_STACK ? 0 : -1;
}

static void crash_handler_more(int sig, struct siginfo* info, void* buf) {
    sigaction(sig, &act_old, nullptr);

    errorLogCount = 0;
    errorLog = "";
    unwind_backtrace();

    LOGD("errorLog: %s", errorLog.c_str());
    namespace fs = boost::filesystem;

    std::string filename = writablePath + "/log.txt";

    fs::path filePath(filename);
    if (fs::exists(filePath)) {
        fs::ofstream fileStream(filePath, std::ios_base::app);

        if (fileStream.is_open()) {

            fileStream << errorLog << std::endl;
            fileStream.close();
        }
    }

    std::string sourcePath = filename;
    std::string destinationPath = writablePath + "/temp/log.crash.txt";

    fs::path source(sourcePath);
    fs::path destination(destinationPath);

    if (fs::exists(source) && fs::is_regular_file(source)) {
        fs::copy_file(source, destination, fs::copy_option::overwrite_if_exists);
    }
}

void initCrashHandler() {
    struct sigaction act{};
    act.sa_sigaction = crash_handler_more;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGKILL, &act, nullptr);
    sigaction(SIGINT, &act, nullptr);
    sigaction(SIGQUIT, &act, nullptr);
    sigaction(SIGILL, &act, nullptr);
    sigaction(SIGABRT, &act, nullptr);
    sigaction(SIGBUS, &act, nullptr);
    sigaction(SIGSEGV, &act, &act_old);
}

namespace ParaEngine {
    const std::string CParaEngineAppAndroid::classname = "com/tatfook/paracraft/ParaEngineActivity";

    CParaEngineAppAndroid::CParaEngineAppAndroid() {
        SetTouchInputting(true);
        CMidiMsg::GetSingleton().SetMediaPlayer(ParaEngineMediaPlayer::GetSingleton());

        initCrashHandler();

        if (!isLogInit) {
            writablePath = ParaEngineHelper::getWritablePath();
            namespace fs = boost::filesystem;
            std::string filename = writablePath + "/log.txt";

            // remove old log file.
            fs::path filePath(filename);
            if (fs::exists(filePath)) {
                fs::remove(filePath);
            }

            // create new log file.
            fs::ofstream newFile(filename);

            if (newFile.is_open()) {
                newFile.close();
            }
        }
    }

    void CParaEngineAppAndroid::GameToClient(int &inout_x,
                                             int &inout_y,
                                             bool bInBackbuffer) {
        // throw std::logic_error("The method or operation is not implemented.");
    }

    void CParaEngineAppAndroid::ClientToGame(int &inout_x,
                                             int &inout_y,
                                             bool bInBackbuffer) {
        // throw std::logic_error("The method or operation is not implemented.");
    }

    bool CParaEngineAppAndroid::AppHasFocus() {
        // throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    void CParaEngineAppAndroid::GetStats(string &output, DWORD dwFields) {
        //throw std::logic_error("The method or operation is not implemented.");
    }

    bool CParaEngineAppAndroid::WriteRegStr(const string &root_key,
                                            const string &sSubKey,
                                            const string &name,
                                            const string &value) {
        // throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    void CParaEngineAppAndroid::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled) {
        // throw std::logic_error("The method or operation is not implemented.");
    }

    const char *CParaEngineAppAndroid::ReadRegStr(const string &root_key,
                                                  const string &sSubKey,
                                                  const string &name) {
        //throw std::logic_error("The method or operation is not implemented.");
        return "";
    }

    bool CParaEngineAppAndroid::WriteRegDWORD(const string &root_key,
                                              const string &sSubKey,
                                              const string &name,
                                              DWORD value) {
        //throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    DWORD CParaEngineAppAndroid::ReadRegDWORD(const string &root_key,
                                              const string &sSubKey,
                                              const string &name) {
        // throw std::logic_error("The method or operation is not implemented.");
        return 0;
    }

    bool CParaEngineAppAndroid::GetAutoLowerFrameRateWhenNotFocused() {
        // throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    void CParaEngineAppAndroid::SetToggleSoundWhenNotFocused(bool bEnabled) {
        // throw std::logic_error("The method or operation is not implemented.");
    }

    bool CParaEngineAppAndroid::GetToggleSoundWhenNotFocused() {
        // throw std::logic_error("The method or operation is not implemented.");
        return true;
    }

    HRESULT CParaEngineAppAndroid::DoWork() {
        return CParaEngineAppBase::DoWork();
    }

    void CParaEngineAppAndroid::GetScreenResolution(Vector2 *pOut) {
        // should return full render window
        pOut->x = CGlobals::GetApp()->GetRenderWindow()->GetWidth();
        pOut->y = CGlobals::GetApp()->GetRenderWindow()->GetHeight();
    }

    void CParaEngineAppAndroid::setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom, const string& editParams) {
        int x = 0;
        this->GameToClient(x, ctrlBottom);
        ParaEngineGLSurfaceView::setIMEKeyboardState(bOpen, bMoveView, ctrlBottom,editParams);
    }

    void CParaEngineAppAndroid::OnPause()
    {
        CAudioEngine2::GetInstance()->PauseAll();
        CParaEngineAppBase::OnPause();
    }

    void CParaEngineAppAndroid::OnResume()
    {
        CAudioEngine2::GetInstance()->ResumeAll();
        CParaEngineAppBase::OnResume();
    }

    void CParaEngineAppAndroid::Exit(int nReturnCode)
    {
        JniHelper::callStaticVoidMethod(classname, "onExit");
    }

    void CParaEngineAppAndroid::GetVisibleSize(Vector2* pOut) {
        pOut->x = 0;
        pOut->y = 0;
    };
}
