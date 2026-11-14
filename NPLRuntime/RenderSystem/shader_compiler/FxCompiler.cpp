#include "FxCompiler.h"
#include "SpirvCodeGen.h"
#include "MslCodeGen.h"
#include "WgslCodeGen.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <sstream>

#include "hlsl2glsl.h"
#include "hlslCrossCompiler.h"
#include "hlslLinker.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <filesystem>
#endif

namespace ParaEngine
{
	namespace
	{
		std::mutex g_hlsl2glslMutex;

		// Binary serialization helpers
		void WriteU8(std::vector<uint8_t>& b, uint8_t v) { b.push_back(v); }
		void WriteU16(std::vector<uint8_t>& b, uint16_t v)
		{
			uint8_t p[2];
			std::memcpy(p, &v, 2);
			b.insert(b.end(), p, p + 2);
		}
		void WriteU32(std::vector<uint8_t>& b, uint32_t v)
		{
			uint8_t p[4];
			std::memcpy(p, &v, 4);
			b.insert(b.end(), p, p + 4);
		}
		void WriteF32(std::vector<uint8_t>& b, float v)
		{
			uint8_t p[4];
			std::memcpy(p, &v, 4);
			b.insert(b.end(), p, p + 4);
		}
		void WriteStr(std::vector<uint8_t>& b, const std::string& s)
		{
			WriteU32(b, static_cast<uint32_t>(s.size()));
			b.insert(b.end(), s.begin(), s.end());
		}

		struct BinaryReader
		{
			const uint8_t* ptr = nullptr;
			size_t size = 0;
			size_t pos = 0;

			bool ReadU8(uint8_t& v)
			{
				if (pos + 1 > size) return false;
				v = ptr[pos++];
				return true;
			}
			bool ReadU16(uint16_t& v)
			{
				if (pos + 2 > size) return false;
				std::memcpy(&v, ptr + pos, 2);
				pos += 2;
				return true;
			}
			bool ReadU32(uint32_t& v)
			{
				if (pos + 4 > size) return false;
				std::memcpy(&v, ptr + pos, 4);
				pos += 4;
				return true;
			}
			bool ReadF32(float& v)
			{
				if (pos + 4 > size) return false;
				std::memcpy(&v, ptr + pos, 4);
				pos += 4;
				return true;
			}
			bool ReadStr(std::string& s)
			{
				uint32_t len = 0;
				if (!ReadU32(len) || pos + len > size) return false;
				s.assign(reinterpret_cast<const char*>(ptr + pos), len);
				pos += len;
				return true;
			}
		};

		void WriteStage(std::vector<uint8_t>& b, const FxCompiledStage& st)
		{
			WriteU8(b, st.valid ? 1 : 0);
			WriteU8(b, static_cast<uint8_t>(st.target));
			WriteStr(b, st.entryPoint);
			WriteStr(b, st.profile);
			WriteU32(b, static_cast<uint32_t>(st.bytecode.size()));
			b.insert(b.end(), st.bytecode.begin(), st.bytecode.end());
			WriteU32(b, st.cbufferSize);
			WriteU32(b, static_cast<uint32_t>(st.cbufferMembers.size()));
			for (const auto& m : st.cbufferMembers)
			{
				WriteStr(b, m.name);
				WriteU32(b, m.byteOffset);
				WriteU32(b, m.byteSize);
				WriteU16(b, m.rows);
				WriteU16(b, m.cols);
				WriteU16(b, m.elements);
				WriteU8(b, m.baseType);
			}
			WriteU32(b, static_cast<uint32_t>(st.textures.size()));
			for (const auto& r : st.textures)
			{
				WriteStr(b, r.name);
				WriteU32(b, r.bindPoint);
				WriteU32(b, r.bindCount);
			}
			WriteU32(b, static_cast<uint32_t>(st.samplers.size()));
			for (const auto& r : st.samplers)
			{
				WriteStr(b, r.name);
				WriteU32(b, r.bindPoint);
				WriteU32(b, r.bindCount);
			}
		}

		bool ReadStage(BinaryReader& r, FxCompiledStage& st)
		{
			uint8_t valid = 0, tgt = 0;
			if (!r.ReadU8(valid) || !r.ReadU8(tgt) || !r.ReadStr(st.entryPoint) || !r.ReadStr(st.profile))
				return false;
			st.valid = (valid != 0);
			st.target = static_cast<EShaderTarget>(tgt);
			uint32_t bcLen = 0;
			if (!r.ReadU32(bcLen) || r.pos + bcLen > r.size)
				return false;
			st.bytecode.assign(r.ptr + r.pos, r.ptr + r.pos + bcLen);
			r.pos += bcLen;
			if (!r.ReadU32(st.cbufferSize))
				return false;
			uint32_t memCount = 0;
			if (!r.ReadU32(memCount))
				return false;
			st.cbufferMembers.resize(memCount);
			for (uint32_t i = 0; i < memCount; ++i)
			{
				auto& m = st.cbufferMembers[i];
				if (!r.ReadStr(m.name) || !r.ReadU32(m.byteOffset) || !r.ReadU32(m.byteSize) ||
					!r.ReadU16(m.rows) || !r.ReadU16(m.cols) || !r.ReadU16(m.elements) || !r.ReadU8(m.baseType))
					return false;
			}
			uint32_t texCount = 0;
			if (!r.ReadU32(texCount)) return false;
			st.textures.resize(texCount);
			for (uint32_t i = 0; i < texCount; ++i)
			{
				if (!r.ReadStr(st.textures[i].name) || !r.ReadU32(st.textures[i].bindPoint) || !r.ReadU32(st.textures[i].bindCount))
					return false;
			}
			uint32_t sampCount = 0;
			if (!r.ReadU32(sampCount)) return false;
			st.samplers.resize(sampCount);
			for (uint32_t i = 0; i < sampCount; ++i)
			{
				if (!r.ReadStr(st.samplers[i].name) || !r.ReadU32(st.samplers[i].bindPoint) || !r.ReadU32(st.samplers[i].bindCount))
					return false;
			}
			return true;
		}

		bool TranslateStageGLSL(
			const std::string& hlslSource,
			const std::string& entryPoint,
			bool isVertex,
			EShaderTarget target,
			FxCompiledStage& outStage,
			std::vector<FxUniformParam>& ioDiscoveredParams,
			std::string& outError)
		{
			std::lock_guard<std::mutex> lock(g_hlsl2glslMutex);

			ETargetVersion targetVer = ETargetGLSL_120;
			if (target == EShaderTarget::ESSL_100) targetVer = ETargetGLSL_ES_100;
			else if (target == EShaderTarget::ESSL_300) targetVer = ETargetGLSL_ES_300;
			else if (target == EShaderTarget::GLSL_330) targetVer = ETargetGLSL_140;

			Hlsl2Glsl_Initialize();
			ShHandle parser = Hlsl2Glsl_ConstructCompiler(isVertex ? EShLangVertex : EShLangFragment);
			if (!parser)
			{
				Hlsl2Glsl_Shutdown();
				outError = "Hlsl2Glsl_ConstructCompiler failed";
				return false;
			}

			auto* linker = parser->GetLinker();
			if (linker)
			{
				linker->setUserAttribName(EAttrSemPosition, "a_position");
				linker->setUserAttribName(EAttrSemNormal, "a_normal");
				linker->setUserAttribName(EAttrSemColor0, "a_color");
				linker->setUserAttribName(EAttrSemColor1, "a_color2");
				linker->setUserAttribName(EAttrSemTex0, "a_texCoord");
				linker->setUserAttribName(EAttrSemTex1, "a_texCoord1");
				linker->setUserAttribName(EAttrSemTex2, "a_texCoord2");
				linker->setUserAttribName(EAttrSemTex3, "a_texCoord3");
				linker->setUserAttribName(EAttrSemBlendWeight, "a_blendWeight");
				linker->setUserAttribName(EAttrSemBlendIndices, "a_blendIndex");
			}

			int opt = ETranslateOpBGRAVertexColor;
			if (!Hlsl2Glsl_Parse(parser, hlslSource.c_str(), targetVer, nullptr, opt))
			{
				const char* log = Hlsl2Glsl_GetInfoLog(parser);
				outError = log ? log : "Hlsl2Glsl_Parse failed";
				Hlsl2Glsl_DestructCompiler(parser);
				Hlsl2Glsl_Shutdown();
				return false;
			}

			if (!Hlsl2Glsl_Translate(parser, entryPoint.c_str(), targetVer, opt))
			{
				const char* log = Hlsl2Glsl_GetInfoLog(parser);
				outError = log ? log : "Hlsl2Glsl_Translate failed";
				Hlsl2Glsl_DestructCompiler(parser);
				Hlsl2Glsl_Shutdown();
				return false;
			}

			const char* rawGlsl = Hlsl2Glsl_GetShader(parser);
			std::string glslText = rawGlsl ? rawGlsl : "";

			// Strip #line and #version directives (matching GLProgram expectations)
			std::istringstream iss(glslText);
			std::ostringstream oss;
			std::string line;
			while (std::getline(iss, line))
			{
				if (line.find("#line ") != std::string::npos || line.find("#version ") != std::string::npos)
					continue;
				oss << line << "\n";
			}
			std::string finalGlsl = oss.str();

			// Reflect uniforms from hlsl2glsl
			int uniCount = Hlsl2Glsl_GetUniformCount(parser);
			if (uniCount > 0)
			{
				const ShUniformInfo* uni = Hlsl2Glsl_GetUniformInfo(parser);
				for (int i = 0; i < uniCount; ++i)
				{
					std::string uname = uni[i].name ? uni[i].name : "";
					std::string usem = uni[i].semantic ? uni[i].semantic : "";
					if (uname.empty())
						continue;

					auto existing = std::find_if(ioDiscoveredParams.begin(), ioDiscoveredParams.end(),
						[&](const FxUniformParam& p) { return p.name == uname; });
					if (existing == ioDiscoveredParams.end())
					{
						FxUniformParam p;
						p.name = uname;
						p.semantic = usem;
						p.elementCount = uni[i].arraySize > 0 ? uni[i].arraySize : 1;
						p.isBool = (uni[i].type == EShTypeBool || uni[i].type == EShTypeBVec2 || uni[i].type == EShTypeBVec3 || uni[i].type == EShTypeBVec4);
						p.isInt = (uni[i].type == EShTypeInt || uni[i].type == EShTypeIVec2 || uni[i].type == EShTypeIVec3 || uni[i].type == EShTypeIVec4);
						ioDiscoveredParams.push_back(std::move(p));
					}
					else if (existing->semantic.empty() && !usem.empty())
					{
						existing->semantic = usem;
					}
				}
			}

			Hlsl2Glsl_DestructCompiler(parser);
			Hlsl2Glsl_Shutdown();

			outStage.valid = true;
			outStage.target = target;
			outStage.entryPoint = entryPoint;
			outStage.profile = isVertex ? "glsl_vs" : "glsl_ps";
			outStage.bytecode.assign(finalGlsl.begin(), finalGlsl.end());
			outStage.bytecode.push_back(0); // Null-terminate for direct const char* usage
			return true;
		}

#ifdef _WIN32
		using PFN_D3DCompile = HRESULT(WINAPI*)(
			LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
		using PFN_D3DReflect = HRESULT(WINAPI*)(
			LPCVOID, SIZE_T, REFIID, void**);

		struct D3DCompilerLoader
		{
			HMODULE hMod = nullptr;
			PFN_D3DCompile pfnCompile = nullptr;
			PFN_D3DReflect pfnReflect = nullptr;

			D3DCompilerLoader()
			{
				const char* dllNames[] = { "d3dcompiler_47.dll", "d3dcompiler_46.dll", "d3dcompiler_43.dll" };
				for (const char* name : dllNames)
				{
					hMod = ::LoadLibraryA(name);
					if (hMod)
						break;
				}
				if (hMod)
				{
					pfnCompile = reinterpret_cast<PFN_D3DCompile>(::GetProcAddress(hMod, "D3DCompile"));
					pfnReflect = reinterpret_cast<PFN_D3DReflect>(::GetProcAddress(hMod, "D3DReflect"));
				}
			}
		};

		static D3DCompilerLoader& GetD3DCompiler()
		{
			static D3DCompilerLoader s_loader;
			return s_loader;
		}

		bool CompileStageDXBC(
			const std::string& hlslSource,
			const std::string& sourceName,
			const std::string& entryPoint,
			bool isVertex,
			EShaderTarget target,
			FxCompiledStage& outStage,
			std::string& outError)
		{
			auto& d3dc = GetD3DCompiler();
			if (!d3dc.pfnCompile || !d3dc.pfnReflect)
			{
				outError = "d3dcompiler_47.dll not found on system";
				return false;
			}

			const char* targetProfile = isVertex ? "vs_4_0" : "ps_4_0";
			const D3D_SHADER_MACRO fl93Macros[] = { { "PARA_FL_9_3", "1" }, { nullptr, nullptr } };
			const D3D_SHADER_MACRO* pMacros = nullptr;
			UINT flags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL1;
			if (target == EShaderTarget::DXBC_SM40_93)
			{
				targetProfile = isVertex ? "vs_4_0_level_9_3" : "ps_4_0_level_9_3";
				pMacros = fl93Macros;
				flags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;
			}
			else if (target == EShaderTarget::DXBC_SM50)
			{
				targetProfile = isVertex ? "vs_5_0" : "ps_5_0";
			}

			ID3DBlob* codeBlob = nullptr;
			ID3DBlob* errBlob = nullptr;
			HRESULT hr = d3dc.pfnCompile(
				hlslSource.data(),
				hlslSource.size(),
				sourceName.c_str(),
				pMacros,
				nullptr,
				entryPoint.c_str(),
				targetProfile,
				flags,
				0,
				&codeBlob,
				&errBlob);

			// If level_9_3 fails on an SM3/SM4 shader (e.g. >8 instructions/samplers), retry vs_4_0/ps_4_0 automatically
			if (FAILED(hr) && target == EShaderTarget::DXBC_SM40_93)
			{
				if (errBlob) { errBlob->Release(); errBlob = nullptr; }
				targetProfile = isVertex ? "vs_4_0" : "ps_4_0";
				hr = d3dc.pfnCompile(
					hlslSource.data(),
					hlslSource.size(),
					sourceName.c_str(),
					nullptr,
					nullptr,
					entryPoint.c_str(),
					targetProfile,
					flags,
					0,
					&codeBlob,
					&errBlob);
			}

			if (FAILED(hr) || !codeBlob)
			{
				if (errBlob)
				{
					outError.assign(static_cast<const char*>(errBlob->GetBufferPointer()), errBlob->GetBufferSize());
					errBlob->Release();
				}
				else
				{
					outError = "D3DCompile failed";
				}
				return false;
			}
			if (errBlob)
				errBlob->Release();

			const uint8_t* ptr = static_cast<const uint8_t*>(codeBlob->GetBufferPointer());
			size_t sz = codeBlob->GetBufferSize();
			outStage.valid = true;
			outStage.target = target;
			outStage.entryPoint = entryPoint;
			outStage.profile = targetProfile;
			outStage.bytecode.assign(ptr, ptr + sz);

			// Reflect $Globals cbuffer, textures, and samplers!
			ID3D11ShaderReflection* refl = nullptr;
			if (SUCCEEDED(d3dc.pfnReflect(ptr, sz, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&refl))) && refl)
			{
				D3D11_SHADER_DESC sdesc{};
				refl->GetDesc(&sdesc);

				for (UINT cbIdx = 0; cbIdx < sdesc.ConstantBuffers; ++cbIdx)
				{
					ID3D11ShaderReflectionConstantBuffer* cb = refl->GetConstantBufferByIndex(cbIdx);
					if (!cb) continue;
					D3D11_SHADER_BUFFER_DESC bdesc{};
					if (FAILED(cb->GetDesc(&bdesc))) continue;

					// Support default $Globals (slot 0)
					if (cbIdx == 0 || (bdesc.Name && std::strcmp(bdesc.Name, "$Globals") == 0))
					{
						outStage.cbufferSize = (bdesc.Size + 15u) & ~15u;
						for (UINT vIdx = 0; vIdx < bdesc.Variables; ++vIdx)
						{
							ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(vIdx);
							if (!var) continue;
							D3D11_SHADER_VARIABLE_DESC vdesc{};
							if (FAILED(var->GetDesc(&vdesc)) || !vdesc.Name) continue;

							FxConstantMember m;
							m.name = vdesc.Name;
							m.byteOffset = vdesc.StartOffset;
							m.byteSize = vdesc.Size;

							ID3D11ShaderReflectionType* vtype = var->GetType();
							if (vtype)
							{
								D3D11_SHADER_TYPE_DESC tdesc{};
								if (SUCCEEDED(vtype->GetDesc(&tdesc)))
								{
									m.rows = static_cast<uint16_t>(tdesc.Rows > 0 ? tdesc.Rows : 1);
									m.cols = static_cast<uint16_t>(tdesc.Columns > 0 ? tdesc.Columns : 1);
									m.elements = static_cast<uint16_t>(tdesc.Elements > 0 ? tdesc.Elements : 1);
									if (tdesc.Type == D3D_SVT_INT) m.baseType = 1;
									else if (tdesc.Type == D3D_SVT_BOOL) m.baseType = 2;
									else if (tdesc.Type == D3D_SVT_UINT) m.baseType = 3;
									else m.baseType = 0;
								}
							}
							outStage.cbufferMembers.push_back(std::move(m));
						}
					}
				}

				for (UINT rIdx = 0; rIdx < sdesc.BoundResources; ++rIdx)
				{
					D3D11_SHADER_INPUT_BIND_DESC ibd{};
					if (FAILED(refl->GetResourceBindingDesc(rIdx, &ibd)) || !ibd.Name)
						continue;
					FxResourceBinding rb;
					rb.name = ibd.Name;
					rb.bindPoint = ibd.BindPoint;
					rb.bindCount = ibd.BindCount;
					if (ibd.Type == D3D_SIT_TEXTURE)
						outStage.textures.push_back(std::move(rb));
					else if (ibd.Type == D3D_SIT_SAMPLER)
						outStage.samplers.push_back(std::move(rb));
				}

				refl->Release();
			}

			codeBlob->Release();
			return true;
		}

		struct DxcCompilerLoader
		{
			HMODULE hDxil = nullptr;
			HMODULE hMod = nullptr;
			DxcCreateInstanceProc pfnCreateInstance = nullptr;

			DxcCompilerLoader()
			{
				hDxil = ::LoadLibraryW(L"dxil.dll");
				hMod = ::LoadLibraryW(L"dxcompiler.dll");
				if (!hMod || !hDxil)
				{
					namespace fs = std::filesystem;
					std::error_code ec;
					const fs::path kitsBin = L"C:\\Program Files (x86)\\Windows Kits\\10\\bin";
					if (fs::exists(kitsBin, ec))
					{
						std::vector<fs::path> candidates;
						for (const auto& entry : fs::directory_iterator(kitsBin, ec))
						{
							if (entry.is_directory(ec))
							{
								fs::path dllPath = entry.path() / L"x64" / L"dxcompiler.dll";
								if (fs::exists(dllPath, ec))
									candidates.push_back(dllPath);
							}
						}
						std::sort(candidates.begin(), candidates.end(), std::greater<fs::path>());
						for (const auto& cand : candidates)
						{
							if (!hDxil)
							{
								fs::path dxilPath = cand.parent_path() / L"dxil.dll";
								if (fs::exists(dxilPath, ec))
									hDxil = ::LoadLibraryW(dxilPath.c_str());
							}
							if (!hMod)
							{
								hMod = ::LoadLibraryW(cand.c_str());
							}
							if (hMod && hDxil)
								break;
						}
					}
				}
				if (hMod)
				{
					pfnCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(::GetProcAddress(hMod, "DxcCreateInstance"));
				}
			}
		};

		static DxcCompilerLoader& GetDxcCompiler()
		{
			static DxcCompilerLoader s_dxcLoader;
			return s_dxcLoader;
		}

		static std::wstring Utf8ToWide(const std::string& s)
		{
			if (s.empty()) return std::wstring();
			int n = ::MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
			if (n <= 0) return std::wstring(s.begin(), s.end());
			std::wstring w(static_cast<size_t>(n), L'\0');
			::MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
			return w;
		}

		bool CompileStageDXIL(
			const std::string& dxcSource,
			const std::string& fallbackD3dSource,
			const std::string& sourceName,
			const std::string& entryPoint,
			bool isVertex,
			FxCompiledStage& outStage,
			std::string& outError)
		{
			auto& dxc = GetDxcCompiler();
			if (!dxc.pfnCreateInstance)
			{
				// Graceful runtime fallback to SM 5.0 DXBC (natively supported by D3D12) when dxcompiler.dll is absent
				return CompileStageDXBC(fallbackD3dSource, sourceName, entryPoint, isVertex, EShaderTarget::DXBC_SM50, outStage, outError);
			}

			IDxcUtils* utils = nullptr;
			IDxcCompiler3* compiler = nullptr;
			if (FAILED(dxc.pfnCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) || !utils ||
				FAILED(dxc.pfnCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))) || !compiler)
			{
				if (utils) utils->Release();
				if (compiler) compiler->Release();
				return CompileStageDXBC(fallbackD3dSource, sourceName, entryPoint, isVertex, EShaderTarget::DXBC_SM50, outStage, outError);
			}

			std::wstring wEntry = Utf8ToWide(entryPoint);
			std::string cleanSourceName = sourceName;
			for (char& ch : cleanSourceName)
			{
				if (ch == ':' || ch == '/' || ch == '\\' || ch == ' ')
					ch = '_';
			}
			while (!cleanSourceName.empty() && cleanSourceName.front() == '_')
				cleanSourceName.erase(cleanSourceName.begin());
			if (cleanSourceName.empty())
				cleanSourceName = "para_shader.hlsl";
			else if (cleanSourceName.find('.') == std::string::npos)
				cleanSourceName += ".hlsl";
			std::wstring wSourceName = Utf8ToWide(cleanSourceName);
			const wchar_t* wProfile = isVertex ? L"vs_6_0" : L"ps_6_0";

			std::vector<const wchar_t*> args = {
				wSourceName.c_str(),
				L"-E", wEntry.c_str(),
				L"-T", wProfile,
				L"-Gec",
				L"-HV", L"2016",
				L"-Zpr",
				L"-O1",
				L"-Wno-conversion"
			};
			if (!isVertex)
			{
				// DXC -Gec maps VS output POSITION -> SV_Position, but leaves PS input POSITION0 as user semantic (POSITION,0).
				// Mapping POSITION/POSITION0 -> SV_Position when compiling ps_6_0 aligns VS->PS stage linkage with fxc/D3D11.
				args.push_back(L"-DPOSITION=SV_Position");
				args.push_back(L"-DPOSITION0=SV_Position");
			}

			DxcBuffer srcBuf{};
			srcBuf.Ptr = dxcSource.data();
			srcBuf.Size = dxcSource.size();
			srcBuf.Encoding = CP_UTF8;

			IDxcResult* result = nullptr;
			HRESULT hr = compiler->Compile(&srcBuf, args.data(), static_cast<UINT32>(args.size()), nullptr, IID_PPV_ARGS(&result));
			if (FAILED(hr) || !result)
			{
				if (result) result->Release();
				compiler->Release();
				utils->Release();
				outError = "IDxcCompiler3::Compile invocation failed";
				return false;
			}

			HRESULT statusHr = S_OK;
			result->GetStatus(&statusHr);
			if (FAILED(statusHr))
			{
				IDxcBlobUtf8* errBlob = nullptr;
				if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errBlob), nullptr)) && errBlob)
				{
					if (errBlob->GetStringLength() > 0)
						outError.assign(errBlob->GetStringPointer(), errBlob->GetStringLength());
					errBlob->Release();
				}
				if (outError.empty())
					outError = "DXC vs_6_0/ps_6_0 compilation failed";
				result->Release();
				compiler->Release();
				utils->Release();
				return false;
			}

			IDxcBlob* objBlob = nullptr;
			if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&objBlob), nullptr)) || !objBlob || objBlob->GetBufferSize() == 0)
			{
				if (objBlob) objBlob->Release();
				result->Release();
				compiler->Release();
				utils->Release();
				outError = "DXC returned empty DXIL object";
				return false;
			}

			const uint8_t* ptr = static_cast<const uint8_t*>(objBlob->GetBufferPointer());
			size_t sz = objBlob->GetBufferSize();

			// Retail D3D12 devices reject unsigned DXIL containers (digest bytes [4..19] all zero when dxil.dll is unavailable).
			// Automatically fall back to signed SM 5.0 DXBC, which D3D12 executes natively on all Windows 10/11 systems.
			bool isSignedDxil = false;
			if (sz >= 20)
			{
				for (size_t di = 4; di < 20; ++di)
				{
					if (ptr[di] != 0)
					{
						isSignedDxil = true;
						break;
					}
				}
			}
			if (!isSignedDxil)
			{
				objBlob->Release();
				result->Release();
				compiler->Release();
				utils->Release();
				return CompileStageDXBC(fallbackD3dSource, sourceName, entryPoint, isVertex, EShaderTarget::DXBC_SM50, outStage, outError);
			}

			outStage.valid = true;
			outStage.target = EShaderTarget::DXIL_SM60;
			outStage.entryPoint = entryPoint;
			outStage.profile = isVertex ? "vs_6_0" : "ps_6_0";
			outStage.bytecode.assign(ptr, ptr + sz);

			// Reflect $Globals cbuffer, textures, and samplers via ID3D12ShaderReflection
			IDxcBlob* reflBlob = nullptr;
			if (SUCCEEDED(result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflBlob), nullptr)) && reflBlob)
			{
				DxcBuffer reflBuf{};
				reflBuf.Ptr = reflBlob->GetBufferPointer();
				reflBuf.Size = reflBlob->GetBufferSize();
				reflBuf.Encoding = 0;

				ID3D12ShaderReflection* refl12 = nullptr;
				if (SUCCEEDED(utils->CreateReflection(&reflBuf, IID_PPV_ARGS(&refl12))) && refl12)
				{
					D3D12_SHADER_DESC sdesc{};
					refl12->GetDesc(&sdesc);

					for (UINT cbIdx = 0; cbIdx < sdesc.ConstantBuffers; ++cbIdx)
					{
						ID3D12ShaderReflectionConstantBuffer* cb = refl12->GetConstantBufferByIndex(cbIdx);
						if (!cb) continue;
						D3D12_SHADER_BUFFER_DESC bdesc{};
						if (FAILED(cb->GetDesc(&bdesc))) continue;

						if (cbIdx == 0 || (bdesc.Name && std::strcmp(bdesc.Name, "$Globals") == 0))
						{
							outStage.cbufferSize = (bdesc.Size + 15u) & ~15u;
							for (UINT vIdx = 0; vIdx < bdesc.Variables; ++vIdx)
							{
								ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(vIdx);
								if (!var) continue;
								D3D12_SHADER_VARIABLE_DESC vdesc{};
								if (FAILED(var->GetDesc(&vdesc)) || !vdesc.Name) continue;

								FxConstantMember m;
								m.name = vdesc.Name;
								m.byteOffset = vdesc.StartOffset;
								m.byteSize = vdesc.Size;

								ID3D12ShaderReflectionType* vtype = var->GetType();
								if (vtype)
								{
									D3D12_SHADER_TYPE_DESC tdesc{};
									if (SUCCEEDED(vtype->GetDesc(&tdesc)))
									{
										m.rows = static_cast<uint16_t>(tdesc.Rows > 0 ? tdesc.Rows : 1);
										m.cols = static_cast<uint16_t>(tdesc.Columns > 0 ? tdesc.Columns : 1);
										m.elements = static_cast<uint16_t>(tdesc.Elements > 0 ? tdesc.Elements : 1);
										if (tdesc.Type == D3D_SVT_INT) m.baseType = 1;
										else if (tdesc.Type == D3D_SVT_BOOL) m.baseType = 2;
										else if (tdesc.Type == D3D_SVT_UINT) m.baseType = 3;
										else m.baseType = 0;
									}
								}
								outStage.cbufferMembers.push_back(std::move(m));
							}
						}
					}

					for (UINT rIdx = 0; rIdx < sdesc.BoundResources; ++rIdx)
					{
						D3D12_SHADER_INPUT_BIND_DESC ibd{};
						if (FAILED(refl12->GetResourceBindingDesc(rIdx, &ibd)) || !ibd.Name)
							continue;
						FxResourceBinding rb;
						rb.name = ibd.Name;
						if (rb.name.compare(0, 11, "__para_tex_") == 0)
							rb.name.erase(0, 11);
						else if (rb.name.compare(0, 11, "__para_smp_") == 0)
							rb.name.erase(0, 11);
						rb.bindPoint = ibd.BindPoint;
						rb.bindCount = ibd.BindCount;
						if (ibd.Type == D3D_SIT_TEXTURE)
							outStage.textures.push_back(std::move(rb));
						else if (ibd.Type == D3D_SIT_SAMPLER)
							outStage.samplers.push_back(std::move(rb));
					}

					refl12->Release();
				}
				reflBlob->Release();
			}

			objBlob->Release();
			result->Release();
			compiler->Release();
			utils->Release();
			return true;
		}
#endif
	}

	bool FxCompiledEffect::SerializeToPFX(std::vector<uint8_t>& outBytes) const
	{
		outBytes.clear();
		// Single-target PFX1 chunk
		const char magic[4] = { 'P', 'F', 'X', 'E' };
		outBytes.insert(outBytes.end(), magic, magic + 4);
		WriteU8(outBytes, 1); // version 1
		WriteU8(outBytes, static_cast<uint8_t>(target));
		WriteStr(outBytes, sourceName);

		WriteU32(outBytes, static_cast<uint32_t>(params.size()));
		for (const auto& p : params)
		{
			WriteStr(outBytes, p.name);
			WriteStr(outBytes, p.semantic);
			WriteStr(outBytes, p.typeName);
			WriteU32(outBytes, static_cast<uint32_t>(p.registerIndex));
			WriteU32(outBytes, static_cast<uint32_t>(p.elementCount));
			WriteU8(outBytes, p.isBool ? 1 : 0);
			WriteU8(outBytes, p.isInt ? 1 : 0);
			WriteU8(outBytes, p.hasDefault ? 1 : 0);
			WriteU32(outBytes, static_cast<uint32_t>(p.defaultFloats.size()));
			for (float f : p.defaultFloats)
				WriteF32(outBytes, f);
		}

		WriteU32(outBytes, static_cast<uint32_t>(samplers.size()));
		for (const auto& s : samplers)
		{
			WriteStr(outBytes, s.samplerName);
			WriteStr(outBytes, s.textureName);
			WriteStr(outBytes, s.samplerType);
			WriteU32(outBytes, static_cast<uint32_t>(s.registerIndex));
			uint8_t mask = (s.hasMinFilter ? 1 : 0) | (s.hasMagFilter ? 2 : 0) | (s.hasMipFilter ? 4 : 0) |
				(s.hasAddressU ? 8 : 0) | (s.hasAddressV ? 16 : 0) | (s.hasAddressW ? 32 : 0) | (s.hasBorderColor ? 64 : 0);
			WriteU8(outBytes, mask);
			WriteU32(outBytes, s.minFilter);
			WriteU32(outBytes, s.magFilter);
			WriteU32(outBytes, s.mipFilter);
			WriteU32(outBytes, s.addressU);
			WriteU32(outBytes, s.addressV);
			WriteU32(outBytes, s.addressW);
			WriteU32(outBytes, s.borderColor);
		}

		WriteU32(outBytes, static_cast<uint32_t>(techniques.size()));
		for (const auto& t : techniques)
		{
			WriteStr(outBytes, t.name);
			WriteU32(outBytes, static_cast<uint32_t>(t.passes.size()));
			for (const auto& pass : t.passes)
			{
				WriteStr(outBytes, pass.name);
				WriteU8(outBytes, pass.stateOnly ? 1 : 0);
				WriteStage(outBytes, pass.vs);
				WriteStage(outBytes, pass.ps);
				WriteU32(outBytes, static_cast<uint32_t>(pass.renderStates.size()));
				for (const auto& rs : pass.renderStates)
				{
					WriteU16(outBytes, static_cast<uint16_t>(rs.state));
					WriteU32(outBytes, rs.value);
				}
			}
		}
		return true;
	}

	bool FxCompiledEffect::DeserializeFromPFX(const void* data, size_t size)
	{
		if (!data || size < 6)
			return false;
		BinaryReader r{ static_cast<const uint8_t*>(data), size, 0 };
		if (std::memcmp(r.ptr, "PFXE", 4) != 0)
			return false;
		r.pos = 4;
		uint8_t ver = 0, tgt = 0;
		if (!r.ReadU8(ver) || ver != 1 || !r.ReadU8(tgt) || !r.ReadStr(sourceName))
			return false;
		target = static_cast<EShaderTarget>(tgt);

		uint32_t paramCount = 0;
		if (!r.ReadU32(paramCount)) return false;
		params.resize(paramCount);
		for (uint32_t i = 0; i < paramCount; ++i)
		{
			auto& p = params[i];
			uint32_t regU = 0, elemU = 0, defCount = 0;
			uint8_t bBool = 0, bInt = 0, bDef = 0;
			if (!r.ReadStr(p.name) || !r.ReadStr(p.semantic) || !r.ReadStr(p.typeName) ||
				!r.ReadU32(regU) || !r.ReadU32(elemU) || !r.ReadU8(bBool) || !r.ReadU8(bInt) || !r.ReadU8(bDef) || !r.ReadU32(defCount))
				return false;
			p.registerIndex = static_cast<int>(regU);
			p.elementCount = static_cast<int>(elemU);
			p.isBool = (bBool != 0);
			p.isInt = (bInt != 0);
			p.hasDefault = (bDef != 0);
			p.defaultFloats.resize(defCount);
			for (uint32_t j = 0; j < defCount; ++j)
			{
				if (!r.ReadF32(p.defaultFloats[j])) return false;
			}
		}

		uint32_t sampCount = 0;
		if (!r.ReadU32(sampCount)) return false;
		samplers.resize(sampCount);
		for (uint32_t i = 0; i < sampCount; ++i)
		{
			auto& s = samplers[i];
			uint32_t regU = 0;
			uint8_t mask = 0;
			if (!r.ReadStr(s.samplerName) || !r.ReadStr(s.textureName) || !r.ReadStr(s.samplerType) ||
				!r.ReadU32(regU) || !r.ReadU8(mask) ||
				!r.ReadU32(s.minFilter) || !r.ReadU32(s.magFilter) || !r.ReadU32(s.mipFilter) ||
				!r.ReadU32(s.addressU) || !r.ReadU32(s.addressV) || !r.ReadU32(s.addressW) || !r.ReadU32(s.borderColor))
				return false;
			s.registerIndex = static_cast<int>(regU);
			s.hasMinFilter = (mask & 1) != 0;
			s.hasMagFilter = (mask & 2) != 0;
			s.hasMipFilter = (mask & 4) != 0;
			s.hasAddressU = (mask & 8) != 0;
			s.hasAddressV = (mask & 16) != 0;
			s.hasAddressW = (mask & 32) != 0;
			s.hasBorderColor = (mask & 64) != 0;
		}

		uint32_t techCount = 0;
		if (!r.ReadU32(techCount)) return false;
		techniques.resize(techCount);
		for (uint32_t tIdx = 0; tIdx < techCount; ++tIdx)
		{
			auto& t = techniques[tIdx];
			uint32_t passCount = 0;
			if (!r.ReadStr(t.name) || !r.ReadU32(passCount))
				return false;
			t.passes.resize(passCount);
			for (uint32_t pIdx = 0; pIdx < passCount; ++pIdx)
			{
				auto& pass = t.passes[pIdx];
				uint8_t stOnly = 0;
				if (!r.ReadStr(pass.name) || !r.ReadU8(stOnly) || !ReadStage(r, pass.vs) || !ReadStage(r, pass.ps))
					return false;
				pass.stateOnly = (stOnly != 0);
				uint32_t rsCount = 0;
				if (!r.ReadU32(rsCount)) return false;
				pass.renderStates.resize(rsCount);
				for (uint32_t k = 0; k < rsCount; ++k)
				{
					uint16_t st = 0;
					if (!r.ReadU16(st) || !r.ReadU32(pass.renderStates[k].value))
						return false;
					pass.renderStates[k].state = static_cast<ERenderState>(st);
				}
			}
		}
		return true;
	}

	bool CFxCompiler::Compile(
		const std::string& rawSource,
		const std::string& sourceName,
		EShaderTarget target,
		FxCompiledEffect& outEffect,
		std::string& outError,
		FxIncludeCallback includeResolver)
	{
		FxParsedDocument doc;
		if (!CFxParserV2::Parse(rawSource, sourceName, doc, outError, includeResolver))
			return false;
		return CompileParsed(doc, target, outEffect, outError);
	}

	bool CFxCompiler::CompileParsed(
		const FxParsedDocument& doc,
		EShaderTarget target,
		FxCompiledEffect& outEffect,
		std::string& outError)
	{
		outEffect = FxCompiledEffect();
		outEffect.sourceName = doc.sourceName;
		outEffect.target = target;
		outEffect.params = doc.params;
		outEffect.samplers = doc.samplers;
		outError.clear();

		// Append dummy pixel shader helper for VS-only passes (e.g. simple_VS.fx, occlusion_test.fx when PS=NULL)
		const char* dummyPsHelper = "\nfloat4 __ParaRHI_DummyPS() : COLOR0 { return float4(1.0, 1.0, 1.0, 1.0); }\n";
		std::string d3dSource = doc.hlslCodeForD3D + dummyPsHelper;
		std::string dxcSource = (doc.hlslCodeForDXC.empty() ? doc.hlslCodeForD3D : doc.hlslCodeForDXC) + dummyPsHelper;
		std::string glslSource = doc.hlslCodeForGLSL + dummyPsHelper;

		std::string firstTechError;

		for (const auto& parsedTech : doc.techniques)
		{
			FxCompiledTechnique compTech;
			compTech.name = parsedTech.name;
			bool techOk = true;
			std::string techErr;

			FxCompiledStage prevVS;
			FxCompiledStage prevPS;

			for (const auto& parsedPass : parsedTech.passes)
			{
				FxCompiledPass compPass;
				compPass.name = parsedPass.name;
				compPass.renderStates = parsedPass.renderStates;

				// State-only pass (like BlurGlowBuffer_Vert in fullscreen_glow.fx): inherit preceding pass's VS/PS
				if (parsedPass.vsEntry.empty() && parsedPass.psEntry.empty() && !parsedPass.vsExplicitNull && !parsedPass.psExplicitNull)
				{
					compPass.stateOnly = true;
					compPass.vs = prevVS;
					compPass.ps = prevPS;
					compTech.passes.push_back(std::move(compPass));
					continue;
				}

				std::string vsEntry = parsedPass.vsEntry;
				std::string psEntry = parsedPass.psEntry;
				if (psEntry.empty() && !vsEntry.empty())
				{
					psEntry = "__ParaRHI_DummyPS";
				}

				if (target == EShaderTarget::GLSL_120 || target == EShaderTarget::GLSL_330 ||
					target == EShaderTarget::ESSL_100 || target == EShaderTarget::ESSL_300)
				{
					if (!vsEntry.empty())
					{
						if (!TranslateStageGLSL(glslSource, vsEntry, true, target, compPass.vs, outEffect.params, techErr))
						{
							techOk = false;
							break;
						}
					}
					if (!psEntry.empty())
					{
						if (!TranslateStageGLSL(glslSource, psEntry, false, target, compPass.ps, outEffect.params, techErr))
						{
							techOk = false;
							break;
						}
					}
				}
				else if (target == EShaderTarget::DXBC_SM40_93 || target == EShaderTarget::DXBC_SM40 || target == EShaderTarget::DXBC_SM50)
				{
#ifdef _WIN32
					if (!vsEntry.empty())
					{
						if (!CompileStageDXBC(d3dSource, doc.sourceName, vsEntry, true, target, compPass.vs, techErr))
						{
							techOk = false;
							break;
						}
					}
					if (!psEntry.empty())
					{
						if (!CompileStageDXBC(d3dSource, doc.sourceName, psEntry, false, target, compPass.ps, techErr))
						{
							techOk = false;
							break;
						}
					}
#else
					outError = "DXBC compilation requires Windows d3dcompiler";
					return false;
#endif
				}
				else if (target == EShaderTarget::DXIL_SM60)
				{
#ifdef _WIN32
					if (!vsEntry.empty())
					{
						if (!CompileStageDXIL(dxcSource, d3dSource, doc.sourceName, vsEntry, true, compPass.vs, techErr))
						{
							techOk = false;
							break;
						}
					}
					if (!psEntry.empty())
					{
						if (!CompileStageDXIL(dxcSource, d3dSource, doc.sourceName, psEntry, false, compPass.ps, techErr))
						{
							techOk = false;
							break;
						}
					}
#else
					outError = "DXIL compilation requires Windows dxcompiler";
					return false;
#endif
				}
				else if (target == EShaderTarget::SPIRV_1_0)
				{
					std::lock_guard<std::mutex> lock(g_hlsl2glslMutex);
					if (!vsEntry.empty())
					{
						if (!CSpirvCodeGen::CompileStageSPIRV(glslSource, vsEntry, true, doc.samplers, doc.params, compPass.vs, outEffect.params, techErr) ||
							!CSpirvCodeGen::ValidateSpirvBinary(compPass.vs.bytecode, techErr))
						{
							techOk = false;
							break;
						}
					}
					if (!psEntry.empty())
					{
						if (!CSpirvCodeGen::CompileStageSPIRV(glslSource, psEntry, false, doc.samplers, doc.params, compPass.ps, outEffect.params, techErr) ||
							!CSpirvCodeGen::ValidateSpirvBinary(compPass.ps.bytecode, techErr))
						{
							techOk = false;
							break;
						}
					}
				}
				else if (target == EShaderTarget::MSL_2_0)
				{
					std::lock_guard<std::mutex> lock(g_hlsl2glslMutex);
					if (!vsEntry.empty())
					{
						if (!CMslCodeGen::CompileStageMSL(glslSource, vsEntry, true, doc.samplers, doc.params, compPass.vs, outEffect.params, techErr) ||
							!CMslCodeGen::ValidateMslSource(compPass.vs.bytecode, true, techErr))
						{
							techOk = false;
							break;
						}
					}
					if (!psEntry.empty())
					{
						if (!CMslCodeGen::CompileStageMSL(glslSource, psEntry, false, doc.samplers, doc.params, compPass.ps, outEffect.params, techErr) ||
							!CMslCodeGen::ValidateMslSource(compPass.ps.bytecode, false, techErr))
						{
							techOk = false;
							break;
						}
					}
				}
				else if (target == EShaderTarget::WGSL_1_0)
				{
					std::lock_guard<std::mutex> lock(g_hlsl2glslMutex);
					if (!vsEntry.empty())
					{
						if (!CWgslCodeGen::CompileStageWGSL(glslSource, vsEntry, true, doc.samplers, doc.params, compPass.vs, outEffect.params, techErr) ||
							!CWgslCodeGen::ValidateWgslSource(compPass.vs.bytecode, true, techErr))
						{
							techOk = false;
							break;
						}
					}
					if (!psEntry.empty())
					{
						if (!CWgslCodeGen::CompileStageWGSL(glslSource, psEntry, false, doc.samplers, doc.params, compPass.ps, outEffect.params, techErr) ||
							!CWgslCodeGen::ValidateWgslSource(compPass.ps.bytecode, false, techErr))
						{
							techOk = false;
							break;
						}
					}
				}
				else
				{
					outError = "Unsupported shader target";
					return false;
				}

				prevVS = compPass.vs;
				prevPS = compPass.ps;
				compTech.passes.push_back(std::move(compPass));
			}

			if (techOk && !compTech.passes.empty())
			{
				outEffect.techniques.push_back(std::move(compTech));
			}
			else if (firstTechError.empty())
			{
				firstTechError = "Technique '" + parsedTech.name + "': " + techErr;
			}
		}

		if (outEffect.techniques.empty())
		{
			outError = firstTechError.empty() ? ("No valid techniques compiled for " + doc.sourceName) : firstTechError;
			return false;
		}
		return true;
	}

	bool CFxCompiler::CompileMultiTargetPFX(
		const std::string& rawSource,
		const std::string& sourceName,
		const std::vector<EShaderTarget>& targets,
		std::vector<uint8_t>& outPfxBytes,
		std::string& outError,
		FxIncludeCallback includeResolver)
	{
		outPfxBytes.clear();
		FxParsedDocument doc;
		if (!CFxParserV2::Parse(rawSource, sourceName, doc, outError, includeResolver))
			return false;

		const char magic[4] = { 'P', 'F', 'X', '1' };
		outPfxBytes.insert(outPfxBytes.end(), magic, magic + 4);
		WriteU32(outPfxBytes, 1u); // container version 1

		std::vector<std::pair<EShaderTarget, std::vector<uint8_t>>> blobs;
		for (EShaderTarget t : targets)
		{
			FxCompiledEffect eff;
			std::string err;
			if (CompileParsed(doc, t, eff, err))
			{
				std::vector<uint8_t> chunk;
				eff.SerializeToPFX(chunk);
				blobs.emplace_back(t, std::move(chunk));
			}
			else if (outError.empty())
			{
				outError = err;
			}
		}

		if (blobs.empty())
			return false;

		outError.clear();
		WriteU32(outPfxBytes, static_cast<uint32_t>(blobs.size()));
		for (const auto& kv : blobs)
		{
			WriteU8(outPfxBytes, static_cast<uint8_t>(kv.first));
			WriteU32(outPfxBytes, static_cast<uint32_t>(kv.second.size()));
			outPfxBytes.insert(outPfxBytes.end(), kv.second.begin(), kv.second.end());
		}
		return true;
	}

	bool CFxCompiler::ExtractTargetFromPFX(
		const void* pfxData,
		size_t pfxSize,
		EShaderTarget preferredTarget,
		FxCompiledEffect& outEffect)
	{
		if (!pfxData || pfxSize < 8)
			return false;
		const uint8_t* ptr = static_cast<const uint8_t*>(pfxData);

		// Single-target PFXE chunk
		if (std::memcmp(ptr, "PFXE", 4) == 0)
			return outEffect.DeserializeFromPFX(pfxData, pfxSize);

		// Multi-target PFX1 container
		if (std::memcmp(ptr, "PFX1", 4) != 0)
			return false;

		BinaryReader r{ ptr, pfxSize, 4 };
		uint32_t ver = 0, count = 0;
		if (!r.ReadU32(ver) || ver != 1 || !r.ReadU32(count))
			return false;

		const uint8_t* fallbackPtr = nullptr;
		size_t fallbackSize = 0;

		for (uint32_t i = 0; i < count; ++i)
		{
			uint8_t tgt = 0;
			uint32_t chunkLen = 0;
			if (!r.ReadU8(tgt) || !r.ReadU32(chunkLen) || r.pos + chunkLen > r.size)
				return false;

			EShaderTarget entryTarget = static_cast<EShaderTarget>(tgt);
			if (entryTarget == preferredTarget)
				return outEffect.DeserializeFromPFX(r.ptr + r.pos, chunkLen);

			// Compatible DXBC / DXIL / GLSL family fallback
			bool familyMatch = false;
			if ((preferredTarget == EShaderTarget::DXBC_SM50 || preferredTarget == EShaderTarget::DXBC_SM40 || preferredTarget == EShaderTarget::DXBC_SM40_93) &&
				(entryTarget == EShaderTarget::DXBC_SM50 || entryTarget == EShaderTarget::DXBC_SM40 || entryTarget == EShaderTarget::DXBC_SM40_93))
				familyMatch = true;
			if (preferredTarget == EShaderTarget::DXIL_SM60 && entryTarget == EShaderTarget::DXBC_SM50)
				familyMatch = true;
			if ((preferredTarget == EShaderTarget::GLSL_330 || preferredTarget == EShaderTarget::GLSL_120) &&
				(entryTarget == EShaderTarget::GLSL_330 || entryTarget == EShaderTarget::GLSL_120))
				familyMatch = true;
			if ((preferredTarget == EShaderTarget::ESSL_300 || preferredTarget == EShaderTarget::ESSL_100) &&
				(entryTarget == EShaderTarget::ESSL_300 || entryTarget == EShaderTarget::ESSL_100))
				familyMatch = true;

			if (familyMatch && !fallbackPtr)
			{
				fallbackPtr = r.ptr + r.pos;
				fallbackSize = chunkLen;
			}
			r.pos += chunkLen;
		}

		if (fallbackPtr)
			return outEffect.DeserializeFromPFX(fallbackPtr, fallbackSize);
		return false;
	}
}
