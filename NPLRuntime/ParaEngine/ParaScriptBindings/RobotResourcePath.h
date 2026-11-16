#pragma once
// Local robot resources use UTF-8 names on every platform.
// NOTE: Keep ParaEngine.h out of this header so MuJoCoSimulation.cpp can enable std::byte=1
// for <mujoco/mjspec.h> without pulling in Windows rpcndr.h byte typedefs.
#if defined(_WIN32) && !defined(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif
#include <cstring>
#include <fstream>
#include <string>
#ifdef _WIN32
#include <codecvt>
#include <locale>
#endif
#include <limits>
#include <memory>
#include <vector>
#include <stdexcept>
#if defined(USE_MUJOCO) && !defined(NPL_ROBOT_NO_MUJOCO_HEADER)
#include <mujoco/mujoco.h>
#endif

namespace ParaScripting {
using RobotResourceFallbackFn = bool(*)(const std::string& path, std::vector<char>& out);

inline RobotResourceFallbackFn& GetRobotResourceFallbackReader() {
    static RobotResourceFallbackFn fn = nullptr;
    return fn;
}

#ifdef _WIN32
inline std::wstring RobotWidePath(const std::string& path) {
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(path);
}
#endif

inline bool RobotFileExistsDirect(const std::string& path) {
#ifdef _WIN32
    std::ifstream file(RobotWidePath(path), std::ios::binary);
#else
    std::ifstream file(path, std::ios::binary);
#endif
    return static_cast<bool>(file);
}

inline bool RobotReadResourceBytes(const std::string& path, std::vector<char>& out) {
    try {
#ifdef _WIN32
        std::ifstream file(RobotWidePath(path), std::ios::binary | std::ios::ate);
#else
        std::ifstream file(path, std::ios::binary | std::ios::ate);
#endif
        if (file) {
            const std::streamoff size = file.tellg();
            if (size < 0 || size > (std::numeric_limits<int>::max)()) return false;
            out.resize(static_cast<size_t>(size));
            file.seekg(0);
            if (size && !file.read(out.data(), static_cast<std::streamsize>(size))) return false;
            return true;
        }
        RobotResourceFallbackFn fallback = GetRobotResourceFallbackReader();
        if (fallback && fallback(path, out)) {
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

#if defined(USE_MUJOCO) && !defined(NPL_ROBOT_NO_MUJOCO_HEADER)
inline std::string MuJoCoResourcePath(const std::string& filename) {
    static const int slot = []() {
        mjpResourceProvider provider;
        mjp_defaultResourceProvider(&provider);
        provider.prefix = "pararobot";
        provider.open = [](mjResource* resource) -> int {
            try {
                const std::string path = std::string(resource->name).substr(10);
                std::unique_ptr<std::vector<char>> bytes(new std::vector<char>());
                if (!RobotReadResourceBytes(path, *bytes)) return 0;
                resource->data = bytes.release();
                return 1;
            } catch (...) { return 0; }
        };
        provider.read = [](mjResource* resource, const void** buffer) -> int {
            auto* bytes = static_cast<std::vector<char>*>(resource->data);
            if (!bytes) return -1;
            *buffer = bytes->data();
            return static_cast<int>(bytes->size());
        };
        provider.close = [](mjResource* resource) {
            delete static_cast<std::vector<char>*>(resource->data);
            resource->data = nullptr;
        };
        return mjp_registerResourceProvider(&provider);
    }();
    if (slot <= 0) throw std::runtime_error("cannot register robot resource provider");
    return "pararobot:" + filename;
}
#endif
}
