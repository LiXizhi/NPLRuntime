//-----------------------------------------------------------------------------
// SerialPortAndroid.h
// Authors: big
// CreateDate: 2023.10.16
// ModifyDate: 2023.10.17
//-----------------------------------------------------------------------------

#include "SerialPortAndroid.h"
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "jni/JniHelper.h"
#include "NPLTable.h"
#include "NPLHelper.h"

namespace ParaEngine {
    const std::string SerialPortAndroid::classname = "com/tatfook/paracraft/utils/USBSerialTransferUtil";
    const std::string SerialPortAndroid::serialName = "SerialPort";
    std::string SerialPortAndroid::sCallback = "";

    void SerialPortAndroid::open(const std::string &filename, const std::string &callback) {
        sCallback = callback;
        JniHelper::callStaticVoidMethod(classname, "nativeOpen");
    }

    void SerialPortAndroid::send(const std::string &data) {
        JniHelper::callStaticVoidMethod(classname, "nativeSend", data);
    }

    void SerialPortAndroid::close() {
        JniHelper::callStaticVoidMethod(classname, "nativeClose");
    }

    std::string SerialPortAndroid::GetPortNames() {
        return serialName;
    }

    extern "C" {
        JNIEXPORT void Java_com_tatfook_paracraft_utils_USBSerialTransferUtil_nativeRead(JNIEnv *env, jclass, jstring j_data) {
            std::string data = JniHelper::jstring2string(j_data);

            NPL::NPLObjectProxy msg;
            msg["filename"] = SerialPortAndroid::serialName;
            msg["data"] = data;

            std::string sMsg;
            NPL::NPLHelper::NPLTableToString("msg", msg, sMsg);
            ParaEngine::CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Activate_async(SerialPortAndroid::sCallback, sMsg.c_str(), (int)sMsg.size());
        }
    }
}
