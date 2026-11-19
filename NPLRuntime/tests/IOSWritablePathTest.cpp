#include "ParaEngine.h"
#include "IO/FileUtils.h"
#include "ParaFileUtilsiOS.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <cstdio>
#include <stdexcept>

namespace fs = boost::filesystem;
using namespace ParaEngine;

static void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main()
{
    const fs::path root = fs::temp_directory_path() / fs::unique_path("npl-ios-path-%%%%-%%%%");
    int result = 0;
    try
    {
        auto* files = CParaFileUtils::GetInstance();
        Check(dynamic_cast<CParaFileUtilsiOS*>(files) != nullptr, "must test the real iOS implementation");
        const std::string platformDefault = files->GetWritablePath();
        CFileUtils::SetWritablePath("");
        Check(CFileUtils::GetWritablePath() == platformDefault, "default writable path changed");
        const std::string writable = (root / "override").string() + "/";
        CFileUtils::SetWritablePath(writable);
        Check(files->GetWritablePath() == platformDefault, "override must not replace the platform default");
        Check(files->GetFullPathForFilename("cache/asset.tmp") == writable + "cache/asset.tmp",
            "relative paths must use the override");
        const std::string absolute = (root / "absolute.tmp").string();
        Check(files->GetFullPathForFilename(absolute) == absolute, "absolute paths must stay unchanged");
        Check(files->MakeDirectoryFromFilePath("cache/asset.tmp"), "create directories under override");
        { std::ofstream out(writable + "cache/asset.tmp", std::ios::binary); out << "asset-data"; }
        Check(files->Exists("cache/asset.tmp"), "relative Exists must find downloaded data");
        Check(files->Exists(writable + "cache/asset.tmp"), "absolute Exists must still work");
        Check(files->Move("cache/asset.tmp", "cache/asset.bin"), "finalize downloaded asset by relative rename");
        Check(!files->Exists("cache/asset.tmp") && files->Exists("cache/asset.bin"), "rename result");
        std::ifstream input(files->GetFullPathForFilename("cache/asset.bin"), std::ios::binary);
        char content[10] = {};
        input.read(content, 10);
        Check(input.gcount() == 10 &&
            std::string(content, 10) == "asset-data", "read finalized asset from override");
        input.close();
        Check(files->Copy("cache/asset.bin", "cache/copy.bin", false), "copy under override");
        Check(files->MakeDirectoryFromFilePath("other/asset.bin"), "create second directory");
        Check(files->Move("cache/copy.bin", "other/asset.bin"), "move across directories under override");
        Check(files->Delete("other/asset.bin"), "delete under override");
        CFileUtils::SetWritablePath((root / "second").string() + "/");
        Check(!files->Exists("cache/asset.bin"), "path changes must take effect immediately");
        Check(fs::exists(writable + "cache/asset.bin"), "changing overrides must preserve existing data");
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "%s\n", error.what());
        result = 1;
    }
    CFileUtils::SetWritablePath("");
    fs::remove_all(root);
    return result;
}
