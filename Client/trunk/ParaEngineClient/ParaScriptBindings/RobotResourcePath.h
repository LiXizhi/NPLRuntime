#pragma once
// Local robot resources use UTF-8 names on every platform.
#include <fstream>
#include <string>
#include <codecvt>
#include <locale>
#include <limits>
#include <memory>
#include <vector>
#include <stdexcept>
#ifdef USE_MUJOCO
#include <mujoco/mujoco.h>
#endif

namespace ParaScripting {
inline std::wstring RobotWidePath(const std::string& path) {
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(path);
}
#ifdef USE_MUJOCO
inline std::string MuJoCoResourcePath(const std::string& filename) {
    static const int slot = []() {
        mjpResourceProvider provider;
        mjp_defaultResourceProvider(&provider);
        provider.prefix = "pararobot";
        provider.open = [](mjResource* resource) -> int {
            try {
                const std::string path = std::string(resource->name).substr(10);
#ifdef _WIN32
                std::ifstream file(RobotWidePath(path), std::ios::binary | std::ios::ate);
#else
                std::ifstream file(path, std::ios::binary | std::ios::ate);
#endif
                if (!file) return 0;
                const std::streamoff size = file.tellg();
                if (size < 0 || size > (std::numeric_limits<int>::max)()) return 0;
                std::unique_ptr<std::vector<char>> bytes(new std::vector<char>(static_cast<size_t>(size)));
                file.seekg(0);
                if (size && !file.read(bytes->data(), static_cast<std::streamsize>(size))) return 0;
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
