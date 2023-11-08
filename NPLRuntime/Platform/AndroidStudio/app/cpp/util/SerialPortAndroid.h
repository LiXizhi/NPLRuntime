//-----------------------------------------------------------------------------
// SerialPortAndroid.h
// Authors: big
// CreateDate: 2023.10.16
// ModifyDate: 2023.10.17
//-----------------------------------------------------------------------------

#pragma once
#include <jni.h>
#include <string>

namespace ParaEngine {
    class SerialPortAndroid {
    public:
        void open(const std::string &filename, const std::string &callback);
        void send(const std::string &data);
        void close();
        std::string GetPortNames();

        static const std::string classname;
        static const std::string serialName;
        static std::string sCallback;
        static SerialPortAndroid &GetInstance() {
            static SerialPortAndroid serialPortAndroid;
            return serialPortAndroid;
        }
    };
}

