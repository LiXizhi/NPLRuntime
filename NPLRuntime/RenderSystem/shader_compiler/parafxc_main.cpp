#include "FxCompiler.h"
#include "FxParserV2.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace ParaEngine;

namespace
{
	bool ReadFileToString(const fs::path& filePath, std::string& outText)
	{
		std::ifstream ifs(filePath, std::ios::binary);
		if (!ifs)
			return false;
		std::ostringstream oss;
		oss << ifs.rdbuf();
		outText = oss.str();
		return true;
	}

	bool WriteBytesToFile(const fs::path& filePath, const std::vector<uint8_t>& bytes)
	{
		if (filePath.has_parent_path())
		{
			std::error_code ec;
			fs::create_directories(filePath.parent_path(), ec);
		}
		std::ofstream ofs(filePath, std::ios::binary);
		if (!ofs)
			return false;
		if (!bytes.empty())
		{
			ofs.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
		}
		return ofs.good();
	}

	FxIncludeCallback MakeDiskIncludeResolver(const fs::path& baseDir)
	{
		return [baseDir](const std::string& includeName, const std::string& /*parentSourceName*/, std::string& outContent) -> bool {
			fs::path candidate = baseDir / includeName;
			if (ReadFileToString(candidate, outContent))
				return true;
			return false;
		};
	}

	const char* TargetName(EShaderTarget t)
	{
		switch (t)
		{
		case EShaderTarget::DXBC_SM40_93: return "DXBC_SM40_93";
		case EShaderTarget::DXBC_SM40: return "DXBC_SM40";
		case EShaderTarget::DXBC_SM50: return "DXBC_SM50";
		case EShaderTarget::DXIL_SM60: return "DXIL_SM60";
		case EShaderTarget::SPIRV_1_0: return "SPIRV_1_0";
		case EShaderTarget::MSL_2_0: return "MSL_2_0";
		case EShaderTarget::GLSL_120: return "GLSL_120";
		case EShaderTarget::GLSL_330: return "GLSL_330";
		case EShaderTarget::ESSL_100: return "ESSL_100";
		case EShaderTarget::ESSL_300: return "ESSL_300";
		case EShaderTarget::WGSL_1_0: return "WGSL_1_0";
		default: return "UNKNOWN";
		}
	}

	int VerifyDirectory(const fs::path& dirPath)
	{
		if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
		{
			std::cerr << "[parafxc] Directory not found: " << dirPath.string() << "\n";
			return 2;
		}

		std::vector<fs::path> fxFiles;
		for (const auto& entry : fs::directory_iterator(dirPath))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".fx")
			{
				fxFiles.push_back(entry.path());
			}
		}
		std::sort(fxFiles.begin(), fxFiles.end());

		std::cout << "[parafxc] Verifying " << fxFiles.size() << " .fx files in " << dirPath.string() << "...\n";

		std::vector<EShaderTarget> targets;
#ifdef _WIN32
		targets.push_back(EShaderTarget::DXBC_SM40);
		targets.push_back(EShaderTarget::DXBC_SM50);
		targets.push_back(EShaderTarget::DXIL_SM60);
#endif
		targets.push_back(EShaderTarget::SPIRV_1_0);
		targets.push_back(EShaderTarget::MSL_2_0);
		targets.push_back(EShaderTarget::WGSL_1_0);
		targets.push_back(EShaderTarget::GLSL_120);
		targets.push_back(EShaderTarget::GLSL_330);
		targets.push_back(EShaderTarget::ESSL_100);
		targets.push_back(EShaderTarget::ESSL_300);

		int totalFiles = static_cast<int>(fxFiles.size());
		int passedFiles = 0;
		int failedFiles = 0;
		int fl93CompatibleCount = 0;

		for (const auto& fxPath : fxFiles)
		{
			std::string rawSrc;
			if (!ReadFileToString(fxPath, rawSrc))
			{
				std::cerr << "[FAIL] " << fxPath.filename().string() << ": cannot read file\n";
				++failedFiles;
				continue;
			}

			auto resolver = MakeDiskIncludeResolver(fxPath.parent_path());
			bool fileOk = true;
			std::string failReason;

			FxParsedDocument doc;
			std::string parseErr;
			if (!CFxParserV2::Parse(rawSrc, fxPath.filename().string(), doc, parseErr, resolver))
			{
				std::cerr << "[FAIL] " << fxPath.filename().string() << " (Parse): " << parseErr << "\n";
				++failedFiles;
				continue;
			}

			for (EShaderTarget target : targets)
			{
				FxCompiledEffect compiled;
				std::string compErr;
				if (!CFxCompiler::CompileParsed(doc, target, compiled, compErr))
				{
					fileOk = false;
					failReason += std::string("  [") + TargetName(target) + "] " + compErr + "\n";
				}
			}

#ifdef _WIN32
			{
				FxCompiledEffect fl93Effect;
				std::string fl93Err;
				if (CFxCompiler::CompileParsed(doc, EShaderTarget::DXBC_SM40_93, fl93Effect, fl93Err))
				{
					++fl93CompatibleCount;
				}
			}
#endif

			if (fileOk)
			{
				std::vector<uint8_t> pfxBytes;
				std::string pfxErr;
				if (!CFxCompiler::CompileMultiTargetPFX(rawSrc, fxPath.filename().string(), targets, pfxBytes, pfxErr, resolver))
				{
					fileOk = false;
					failReason += "  [PFX Serialize] " + pfxErr + "\n";
				}
				else
				{
					for (EShaderTarget target : targets)
					{
						FxCompiledEffect roundtrip;
						if (!CFxCompiler::ExtractTargetFromPFX(pfxBytes.data(), pfxBytes.size(), target, roundtrip))
						{
							fileOk = false;
							failReason += std::string("  [PFX Extract ") + TargetName(target) + "] failed\n";
						}
					}
				}
			}

			if (fileOk)
			{
				++passedFiles;
				size_t totalPasses = 0;
				for (const auto& t : doc.techniques)
					totalPasses += t.passes.size();
				std::cout << "[PASS] " << fxPath.filename().string()
					<< " (techs=" << doc.techniques.size()
					<< ", passes=" << totalPasses
					<< ", uniforms=" << doc.params.size()
					<< ", samplers=" << doc.samplers.size() << ")\n";
			}
			else
			{
				++failedFiles;
				std::cerr << "[FAIL] " << fxPath.filename().string() << ":\n" << failReason;
			}
		}

		std::cout << "\n[parafxc] Summary: " << passedFiles << "/" << totalFiles << " passed, "
			<< failedFiles << " failed";
#ifdef _WIN32
		std::cout << " (FL 9_3 compatible: " << fl93CompatibleCount << "/" << totalFiles << ")";
#endif
		std::cout << "\n";

		return (failedFiles == 0 && totalFiles > 0) ? 0 : 1;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage:\n"
			<< "  parafxc --verify-dir <shaders/fx>\n"
			<< "  parafxc <input.fx> -o <output.pfx>\n";
		return 1;
	}

	std::string arg1 = argv[1];
	if (arg1 == "--verify-dir" && argc >= 3)
	{
		return VerifyDirectory(fs::path(argv[2]));
	}
	if (arg1 == "--dump-target" && argc >= 4)
	{
		std::string tname = argv[2];
		EShaderTarget tgt = EShaderTarget::MSL_2_0;
		if (tname == "wgsl" || tname == "WGSL_1_0") tgt = EShaderTarget::WGSL_1_0;
		else if (tname == "msl" || tname == "MSL_2_0") tgt = EShaderTarget::MSL_2_0;
		else if (tname == "glsl330" || tname == "GLSL_330") tgt = EShaderTarget::GLSL_330;
		else if (tname == "glsl120" || tname == "GLSL_120") tgt = EShaderTarget::GLSL_120;
		else if (tname == "essl300" || tname == "ESSL_300") tgt = EShaderTarget::ESSL_300;
		else if (tname == "essl100" || tname == "ESSL_100") tgt = EShaderTarget::ESSL_100;

		fs::path inFx(argv[3]);
		std::string rawSrc;
		if (!ReadFileToString(inFx, rawSrc))
		{
			std::cerr << "[parafxc] Failed to read " << inFx.string() << "\n";
			return 1;
		}
		FxCompiledEffect eff;
		std::string err;
		if (!CFxCompiler::Compile(rawSrc, inFx.filename().string(), tgt, eff, err, MakeDiskIncludeResolver(inFx.parent_path())))
		{
			std::cerr << "[parafxc] Compile failed: " << err << "\n";
			return 1;
		}
		for (const auto& tech : eff.techniques)
		{
			for (const auto& pass : tech.passes)
			{
				std::cout << "// === Technique: " << tech.name << " | Pass: " << pass.name << " ===\n";
				if (!pass.vs.bytecode.empty())
				{
					std::cout << "// --- VS (" << pass.vs.entryPoint << ", cbuffer=" << pass.vs.cbufferSize << "B) ---\n";
					std::cout.write(reinterpret_cast<const char*>(pass.vs.bytecode.data()), static_cast<std::streamsize>(pass.vs.bytecode.size()));
					std::cout << "\n";
				}
				if (!pass.ps.bytecode.empty())
				{
					std::cout << "// --- PS (" << pass.ps.entryPoint << ", cbuffer=" << pass.ps.cbufferSize << "B) ---\n";
					std::cout.write(reinterpret_cast<const char*>(pass.ps.bytecode.data()), static_cast<std::streamsize>(pass.ps.bytecode.size()));
					std::cout << "\n";
				}
			}
		}
		return 0;
	}

	fs::path inputPath;
	fs::path outputPath;
	for (int i = 1; i < argc; ++i)
	{
		std::string a = argv[i];
		if (a == "-o" && i + 1 < argc)
		{
			outputPath = argv[++i];
		}
		else if (!a.empty() && a[0] != '-')
		{
			inputPath = a;
		}
	}

	if (inputPath.empty())
	{
		std::cerr << "[parafxc] Missing input .fx file.\n";
		return 1;
	}
	if (outputPath.empty())
	{
		outputPath = inputPath;
		outputPath.replace_extension(".pfx");
	}

	std::string rawSrc;
	if (!ReadFileToString(inputPath, rawSrc))
	{
		std::cerr << "[parafxc] Failed to read: " << inputPath.string() << "\n";
		return 1;
	}

	std::vector<EShaderTarget> targets;
#ifdef _WIN32
	targets.push_back(EShaderTarget::DXBC_SM40);
	targets.push_back(EShaderTarget::DXBC_SM50);
	targets.push_back(EShaderTarget::DXIL_SM60);
#endif
	targets.push_back(EShaderTarget::SPIRV_1_0);
	targets.push_back(EShaderTarget::MSL_2_0);
	targets.push_back(EShaderTarget::WGSL_1_0);
	targets.push_back(EShaderTarget::GLSL_120);
	targets.push_back(EShaderTarget::GLSL_330);
	targets.push_back(EShaderTarget::ESSL_100);
	targets.push_back(EShaderTarget::ESSL_300);

	std::vector<uint8_t> pfxBytes;
	std::string err;
	if (!CFxCompiler::CompileMultiTargetPFX(rawSrc, inputPath.filename().string(), targets, pfxBytes, err, MakeDiskIncludeResolver(inputPath.parent_path())))
	{
		std::cerr << "[parafxc] Compile failed for " << inputPath.string() << ":\n" << err << "\n";
		return 1;
	}

	if (!WriteBytesToFile(outputPath, pfxBytes))
	{
		std::cerr << "[parafxc] Failed to write: " << outputPath.string() << "\n";
		return 1;
	}

	std::cout << "[parafxc] Wrote " << outputPath.string() << " (" << pfxBytes.size() << " bytes)\n";
	return 0;
}
