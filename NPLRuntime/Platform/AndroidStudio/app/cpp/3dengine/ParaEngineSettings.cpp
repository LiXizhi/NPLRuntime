//-----------------------------------------------------------------------------
// ParaEngineSettings.cpp
// Authors:	big
// Emails: onedou@126.com
// CreateDate: 2020.6.11
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "3dengine/ParaEngineSettings.h"
#include "jni/ParaEngineHelper.h"
#include "jni/JniHelper.h"

namespace ParaEngine {
    const std::string& ParaEngineSettings::GetMachineID()
    {
        static std::string str = "";

        if (str.empty())
        {
            str = ParaEngineHelper::getMachineID();
        }

        return str;
    }

    bool ParaEngineSettings::GetUsbMode()
    {
        return JniHelper::callStaticBooleanMethod("com/tatfook/paracraft/ParaEngineActivity", "getUsbMode");
    }

    void ParaEngineSettings::SaveImageToGallery(const char* imageData)
    {
        if (imageData){
            JniHelper::callStaticVoidMethod("com/tatfook/paracraft/ParaEngineActivity","saveImageToGallery",imageData);
        }
    }
}
