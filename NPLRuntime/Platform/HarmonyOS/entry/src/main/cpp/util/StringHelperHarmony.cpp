#include "ParaEngine.h"
#include "util/StringHelper.h"

namespace ParaEngine
{
    // Clipboard functions - minimal stub implementation
    bool StringHelper::CopyTextToClipboard(const std::string& text)
    {
        // TODO: Implement clipboard functionality for HarmonyOS
        return false;
    }

    const char* StringHelper::GetTextFromClipboard()
    {
        // TODO: Implement clipboard functionality for HarmonyOS
        static const char* emptyStr = "";
        return emptyStr;
    }
}
