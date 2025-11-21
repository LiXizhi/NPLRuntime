#include "ParaEngine.h"
#include "util/CEditorHelper.h"

namespace ParaEngine
{
    namespace CEditorHelper
    {
        void OpenWithDefaultEditor(const char* filename, bool bWaitOnReturn)
        {
            // TODO: Implement file opening for HarmonyOS
        }

        bool CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn)
        {
            // TODO: Implement process creation for HarmonyOS
            return false;
        }

        bool ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
        {
            // TODO: Implement shell execute for HarmonyOS
            return false;
        }
    }
}
