//-----------------------------------------------------------------------------
// Class:	CEffectFileRHI
// Authors:	ParaEngine Cross-Platform RHI Team
// Desc:	Unified multi-backend effect file implementation (ParaRHI)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifndef NPLRUNTIME_SERVER

#include "effect_file_RHI.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "SkyMesh.h"
#include "SunLight.h"
#include "EffectManager.h"
#include "RenderSystem/shader_compiler/FxCompiler.h"
#include "RenderSystem/shader_compiler/FxParserV2.h"

#ifdef PARA_ENABLE_OPENGL
#include "OpenGLWrapper/GLProgram.h"
#endif

#include <algorithm>
#include <cctype>
#include <cstring>

namespace ParaEngine
{
	const int CEffectFileBase::MAX_EFFECT_LIGHT_NUM;

	bool CEffectFileRHI::g_bTextureEnabled = true;

	namespace
	{
		std::string ToLowerStr(const std::string& s)
		{
			std::string out = s;
			std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			return out;
		}

		bool StartsWithStr(const std::string& s, const char* prefix)
		{
			size_t len = std::strlen(prefix);
			return s.size() >= len && s.compare(0, len, prefix) == 0;
		}

		std::string ReplaceExt(const std::string& path, const char* newExt)
		{
			size_t dot = path.find_last_of('.');
			size_t slash = path.find_last_of("/\\");
			if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
			{
				return path.substr(0, dot) + newExt;
			}
			return path + newExt;
		}

		std::string BaseStemLower(const std::string& path)
		{
			size_t slash = path.find_last_of("/\\");
			std::string name = (slash != std::string::npos) ? path.substr(slash + 1) : path;
			size_t dot = name.find_last_of('.');
			if (dot != std::string::npos)
				name = name.substr(0, dot);
			return ToLowerStr(name);
		}

		std::string ParentDirOf(const std::string& path)
		{
			if (path.empty() || path[0] == ':')
				return std::string();
			size_t slash = path.find_last_of("/\\");
			if (slash == std::string::npos)
				return std::string();
			return path.substr(0, slash + 1);
		}

		const char* BuiltinEmbeddedByStem(const std::string& stemLower)
		{
			static const struct { const char* stem; const char* res; } kMap[] = {
				{ "ocean_water", ":IDR_FX_OCEANWATER" },
				{ "occlusion_test", ":IDR_FX_OCCLUSION_TEST" },
				{ "simple_mesh", ":IDR_FX_SIMPLE_MESH" },
				{ "simple_mesh_normal", ":IDR_FX_SIMPLE_MESH_NORMAL" },
				{ "smesh_ctor", ":IDR_FX_SMESH_CTOR" },
				{ "terrain_normal", ":IDR_FX_TERRAIN_NORMAL" },
				{ "terrain_normal_depth_shadowmap", ":IDR_FX_TERRAIN_NORMAL_DEPTH_SHADOWMAP" },
				{ "fullscreen_glow", ":IDR_FX_FULLSCREEN_GLOW" },
				{ "sky", ":IDR_FX_SKY" },
				{ "simple_mesh_normal_shadow", ":IDR_FX_SIMPLE_MESH_SHADOW" },
				{ "simple_mesh_normal_tex2", ":IDR_FX_SIMPLE_MESH_NORMAL_TEX2" },
				{ "simple_mesh_normal_selected", ":IDR_FX_SIMPLE_MESH_NORMAL_SELECTED" },
				{ "simple_mesh_normal_border", ":IDR_FX_SIMPLE_MESH_NORMAL_BORDER" },
				{ "simple_mesh_normal_low", ":IDR_FX_SIMPLE_MESH_NORMAL_LOW" },
				{ "water_ripples", ":IDR_FX_WATER_RIPPLE" },
				{ "ocean_water_high", ":IDR_FX_OCEANWATER_HIGH" },
				{ "simple_mesh_normal_instanced", ":IDR_FX_SIMPLE_MESH_NORMAL_INSTANCED" },
				{ "simple_mesh_normal_unlit", ":IDR_FX_SIMPLE_MESH_NORMAL_UNLIT" },
				{ "simple_particle", ":IDR_FX_SIMPLE_PARTICLE" },
				{ "simple_cad_model", ":IDR_FX_SIMPLE_CAD_MODEL" },
				{ "dropshadow", ":IDR_FX_DROPSHADOW" },
				{ "screenwave", ":IDR_FX_SCREENWAVE" },
				{ "blockeffect", ":IDR_FX_BLOCK" },
				{ "shadowmap_blur", ":IDR_FX_SHADOWMAP_BLUR" },
				{ "simple_mesh_normal_vegetation", ":IDR_FX_SIMPLE_MESH_VEGETATION" },
				{ "simple_mesh_normal_specialeffects", ":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS" },
				{ "skydome", ":IDR_FX_SKYDOME" },
				{ "ocean_water_quad", ":IDR_FX_OCEANWATER_QUAD" },
				{ "ocean_water_cloud", ":IDR_FX_OCEANWATER_CLOUD" },
				{ "bmaxmodel", ":IDR_FX_BMAXMODEL" },
				{ "bmax_model_skinned", ":IDR_FX_BMAX_MODEL_SKINNED" },
				{ "character_skinned", ":IDR_FX_CHARACTER_SKINNED" },
				{ "singlecoloreffect", ":IDR_FX_SINGLECOLOR" },
				{ "guieffect", ":IDR_FX_GUI" },
				{ "guitexteffect", ":IDR_FX_GUI_TEXT" },
				{ "gui_simple", ":IDR_FX_GUI_SIMPLE" },
				{ "redbluestereo", ":IDR_FX_REDBLUESTEREO" },
				{ "simple_vs", ":IDR_FX_SIMPLE_VS" },
				{ "unlit_texture", ":IDR_FX_UNLIT_TEXTURE" },
				{ "cartooneffect", ":IDR_FX_CARTOON_EFFECT" },
				{ "coloreffect", ":IDR_FX_COLOR_EFFECT" },
				{ "deferredshadingdirectionallighting", ":IDR_FX_DEFERRED_DIRECTIONAL_LIGHTING" },
				{ "deferredshadingpointlighting", ":IDR_FX_DEFERRED_POINT_LIGHTING" },
				{ "deferredshadingspotlighting", ":IDR_FX_DEFERRED_SPOT_LIGHTING" },
				{ "grey", ":IDR_FX_GREY" },
				{ "greyblur", ":IDR_FX_GREY_BLUR" },
				{ "odsstereosingleeye", ":IDR_FX_ODS_STEREO_SINGLE_EYE" },
				{ "sceneeffect", ":IDR_FX_SCENE_EFFECT" },
				{ "supermultiviewstereo", ":IDR_FX_SUPER_MULTIVIEW_STEREO" },
				{ "vignetting", ":IDR_FX_VIGNETTING" },
				{ "composite", ":IDR_FX_COMPOSITE" },
				{ "compositeterrain", ":IDR_FX_COMPOSITE_TERRAIN" },
				{ "compositelite", ":IDR_FX_COMPOSITE_LITE" },
				{ "final", ":IDR_FX_FINAL" },
				{ "mrt_blocks", ":IDR_FX_MRT_BLOCKS" },
				{ "mrt_bmax_model", ":IDR_FX_MRT_BMAX_MODEL" },
				{ "mrt_bmax_model_skinned", ":IDR_FX_MRT_BMAX_MODEL_SKINNED" },
				{ "mrt_character_skinned", ":IDR_FX_MRT_CHARACTER_SKINNED" },
				{ "mrt_mesh_normal", ":IDR_FX_MRT_MESH_NORMAL" },
				{ "mrt_mesh_normal_instanced", ":IDR_FX_MRT_MESH_NORMAL_INSTANCED" },
				{ "mrt_terrain_normal", ":IDR_FX_MRT_TERRAIN_NORMAL" }
			};
			for (const auto& entry : kMap)
			{
				if (stemLower == entry.stem)
					return entry.res;
			}
			return nullptr;
		}

		bool TryReadParaFile(const std::string& path, std::string& outBytes)
		{
			if (path.empty())
				return false;
			CParaFile file(path.c_str());
			if (file.isEof() || file.getBuffer() == nullptr || file.getSize() == 0)
				return false;
			outBytes.assign(file.getBuffer(), static_cast<size_t>(file.getSize()));
			return true;
		}

		bool IsLegacyD3D9FxoBinary(const std::string& bytes)
		{
			if (bytes.size() < 4)
				return false;
			uint32_t magic = 0;
			std::memcpy(&magic, bytes.data(), 4);
			return magic == 0xFEFF0901u || magic == 0xBCF00ACF;
		}

		bool IsPfxBinary(const std::string& bytes)
		{
			return bytes.size() >= 8 &&
				(std::memcmp(bytes.data(), "PFX1", 4) == 0 || std::memcmp(bytes.data(), "PFXE", 4) == 0);
		}

		FxIncludeCallback MakeParaIncludeResolver(const std::string& parentFilePath)
		{
			std::string parentDir = ParentDirOf(parentFilePath);
			return [parentDir](const std::string& includePath, const std::string& /*parentSource*/, std::string& outContent) -> bool {
				std::string incLower = ToLowerStr(includePath);
				size_t slash = incLower.find_last_of("/\\");
				if (slash != std::string::npos)
					incLower = incLower.substr(slash + 1);

				if (incLower == "screen_space_reflection.h" && TryReadParaFile(":IDR_SHADER_INC_SCREEN_SPACE_REFLECTION", outContent))
					return true;

				if (incLower == "composite.fx" && TryReadParaFile(":IDR_FX_COMPOSITE", outContent))
					return true;

				if (incLower == "simple_mesh_normal_low.fx" && TryReadParaFile(":IDR_FX_SIMPLE_MESH_NORMAL_LOW", outContent))
					return true;
				if (incLower == "commondefine.h" || incLower == "commondefine.fx")
				{
					if (TryReadParaFile(":IDR_SHADER_INC_COMMON_DEFINE", outContent))
						return true;
				}
				if (incLower == "commonfunction.h" || incLower == "commonfunction.fx")
				{
					if (TryReadParaFile(":IDR_SHADER_INC_COMMON_FUNCTION", outContent))
						return true;
				}
				if (incLower == "fxaa.h" || incLower == "fxaa.hlsl")
				{
					if (TryReadParaFile(":IDR_SHADER_INC_FXAA", outContent))
						return true;
				}

				if (!parentDir.empty() && TryReadParaFile(parentDir + includePath, outContent))
					return true;
				if (TryReadParaFile("shaders/fx/" + includePath, outContent))
					return true;
				if (TryReadParaFile("shaders/" + includePath, outContent))
					return true;
				if (TryReadParaFile(includePath, outContent))
					return true;

				if (incLower == "common_shader_data.h")
					return TryReadParaFile(":IDR_SHADER_INC_COMMON_SHADER_DATA", outContent);
				if (incLower == "light_scattering_constants.h")
					return TryReadParaFile(":IDR_SHADER_INC_LIGHT_SCATTERING_CONSTANTS", outContent);
				if (incLower == "mesh_local_lights.h")
					return TryReadParaFile(":IDR_SHADER_INC_MESH_LOCAL_LIGHTS", outContent);
				if (incLower == "patch_deform.h")
					return TryReadParaFile(":IDR_SHADER_INC_PATCH_DEFORM", outContent);
				if (incLower == "simple_mesh_normal_vegetation.h")
					return TryReadParaFile(":IDR_SHADER_INC_SIMPLE_MESH_NORMAL_VEGETATION", outContent);
				return false;
			};
		}

		EShaderTarget SelectTargetForDevice(IRenderDevice* pDevice)
		{
			if (!pDevice)
				return EShaderTarget::Unknown;

			const ERHIBackend backend = pDevice->GetBackendType();
			const RHIDeviceCaps& caps = pDevice->GetCaps();
			switch (backend)
			{
			case ERHIBackend::OpenGL:
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
				return (caps.featureLevel >= 300) ? EShaderTarget::ESSL_300 : EShaderTarget::ESSL_100;
#else
				return EShaderTarget::GLSL_120;
#endif
			case ERHIBackend::D3D11:
				if (caps.featureLevel > 0 && caps.featureLevel < 0xa000)
					return EShaderTarget::DXBC_SM40_93;
				if (caps.featureLevel >= 0xb000)
					return EShaderTarget::DXBC_SM50;
				return EShaderTarget::DXBC_SM40;
			case ERHIBackend::D3D12:
				return EShaderTarget::DXIL_SM60;
			case ERHIBackend::Vulkan:
				return EShaderTarget::SPIRV_1_0;
			case ERHIBackend::Metal:
				return EShaderTarget::MSL_2_0;
			case ERHIBackend::WebGPU:
				return EShaderTarget::WGSL_1_0;
			case ERHIBackend::Null:
			default:
				return EShaderTarget::Unknown;
			}
		}

		uint32_t EstimateScalarElementBytes(const FxUniformParam& param)
		{
			const std::string t = ToLowerStr(param.typeName);
			if (param.isBool || param.isInt || t == "float" || t == "half" || t == "scalar" || t == "int" || t == "uint" || t == "bool" || t == "dword")
				return 4;
			if (t == "float2" || t == "half2" || t == "scalar2" || t == "int2")
				return 8;
			if (t == "float3" || t == "half3" || t == "scalar3" || t == "int3")
				return 12;
			if (t == "float4" || t == "half4" || t == "scalar4" || t == "int4")
				return 16;
			if (t.find("4x4") != std::string::npos || t == "matrix")
				return 64;
			if (t.find("4x3") != std::string::npos || t.find("3x4") != std::string::npos)
				return 48;
			if (t.find("3x3") != std::string::npos)
				return 36;
			if (t == "slightscatteringshaderparams")
				return 176;
			return 64;
		}

		uint32_t EstimateTypeByteSize(const FxUniformParam& param)
		{
			uint32_t elemBytes = EstimateScalarElementBytes(param);
			uint32_t count = param.elementCount > 1 ? static_cast<uint32_t>(param.elementCount) : 1u;
			if (count > 1 && elemBytes < 16)
				elemBytes = 16; // HLSL cbuffer array element 16-byte stride
			return elemBytes * count;
		}

		const std::unordered_map<std::string, uint32_t>& GetSemanticTableLower()
		{
			static std::unordered_map<std::string, uint32_t> s_table;
			if (s_table.empty())
			{
				s_table["world"] = CEffectFileBase::k_worldMatrix;
				s_table["worldinverse"] = CEffectFileBase::k_worldInverseMatrix;
				s_table["worldview"] = CEffectFileBase::k_worldViewMatrix;
				s_table["worldviewprojection"] = CEffectFileBase::k_worldViewProjMatrix;
				s_table["worldmatrixarray"] = CEffectFileBase::k_worldMatrixArray;
				s_table["skyboxmatrix"] = CEffectFileBase::k_skyBoxMatrix;
				s_table["view"] = CEffectFileBase::k_viewMatrix;
				s_table["projection"] = CEffectFileBase::k_projMatrix;
				s_table["viewprojection"] = CEffectFileBase::k_viewProjMatrix;
				s_table["texworldviewproj"] = CEffectFileBase::k_TexWorldViewProjMatrix;

				s_table["materialambient"] = CEffectFileBase::k_ambientMaterialColor;
				s_table["materialdiffuse"] = CEffectFileBase::k_diffuseMaterialColor;
				s_table["materialspecular"] = CEffectFileBase::k_specularMaterialColor;
				s_table["materialemissive"] = CEffectFileBase::k_emissiveMaterialColor;
				s_table["posscaleoffset"] = CEffectFileBase::k_posScaleOffset;
				s_table["uvscaleoffset"] = CEffectFileBase::k_uvScaleOffset;
				s_table["flarecolor"] = CEffectFileBase::k_lensFlareColor;
				s_table["fogparameters"] = CEffectFileBase::k_fogParameters;
				s_table["fogcolor"] = CEffectFileBase::k_fogColor;
				s_table["shadowfactor"] = CEffectFileBase::k_shadowFactor;
				s_table["lightstrength"] = CEffectFileBase::k_LightStrength;
				s_table["lightcolors"] = CEffectFileBase::k_LightColors;
				s_table["lightpositions"] = CEffectFileBase::k_LightPositions;
				s_table["lightparams"] = CEffectFileBase::k_LightParams;
				s_table["fresnelr0"] = CEffectFileBase::k_fresnelR0;
				s_table["constvector0"] = CEffectFileBase::k_ConstVector0;
				s_table["constvector1"] = CEffectFileBase::k_ConstVector1;
				s_table["constvector2"] = CEffectFileBase::k_ConstVector2;
				s_table["constvector3"] = CEffectFileBase::k_ConstVector3;
				s_table["sunvector"] = CEffectFileBase::k_sunVector;
				s_table["suncolor"] = CEffectFileBase::k_sunColor;
				s_table["worldcamerapos"] = CEffectFileBase::k_cameraPos;
				s_table["viewdistances"] = CEffectFileBase::k_cameraDistances;
				s_table["worldviewvector"] = CEffectFileBase::k_cameraFacing;
				s_table["ambientlight"] = CEffectFileBase::k_ambientLight;
				s_table["sunlight_inscatter"] = CEffectFileBase::k_sunlightInscatter;
				s_table["sunlight_extinction"] = CEffectFileBase::k_sunlightExtinction;
				s_table["worldpos"] = CEffectFileBase::k_worldPos;
				s_table["texcoordoffset"] = CEffectFileBase::k_texCoordOffset;
				s_table["atmosphericlightingparams"] = CEffectFileBase::k_atmosphericLighting;
				s_table["patchcorners"] = CEffectFileBase::k_patchCorners;

				s_table["fogenable"] = CEffectFileBase::k_fogEnable;
				s_table["alphatesting"] = CEffectFileBase::k_bAlphaTesting;
				s_table["alphablending"] = CEffectFileBase::k_bAlphaBlending;
				for (int i = 0; i < (CEffectFileBase::k_bBooleanMAX - CEffectFileBase::k_bBoolean0); ++i)
				{
					s_table["boolean" + std::to_string(i)] = CEffectFileBase::k_bBoolean0 + i;
				}
				s_table["sunlightenable"] = CEffectFileBase::k_bSunlightEnable;
				s_table["shadowmapsize"] = CEffectFileBase::k_nShadowmapSize;
				s_table["shadowradius"] = CEffectFileBase::k_fShadowRadius;
				s_table["materialpower"] = CEffectFileBase::k_specularMaterialPower;
				s_table["reflectfactor"] = CEffectFileBase::k_reflectFactor;
				s_table["locallightnum"] = CEffectFileBase::k_LocalLightNum;
				s_table["layersnum"] = CEffectFileBase::k_LayersNum;
				s_table["time"] = CEffectFileBase::k_time;
				s_table["opacity"] = CEffectFileBase::k_opacity;
				s_table["specularpower"] = CEffectFileBase::k_specularPower;
				s_table["transitionfactor"] = CEffectFileBase::k_transitionFactor;

				s_table["materialbasecolor"] = CEffectFileBase::k_material_base_color;
				s_table["materialmetallic"] = CEffectFileBase::k_material_metallic;
				s_table["materialspecular"] = CEffectFileBase::k_material_specular;
				s_table["materialroughness"] = CEffectFileBase::k_material_roughness;
				s_table["materialemissivecolor"] = CEffectFileBase::k_material_emissive_color;
				s_table["materialemissive"] = CEffectFileBase::k_material_emissive;
				s_table["materialopacity"] = CEffectFileBase::k_material_opacity;
				s_table["materialnormaltexture"] = CEffectFileBase::k_material_normal_texture;
				s_table["materialdiffusetexture"] = CEffectFileBase::k_material_diffuse_texture;
				s_table["materialuv"] = CEffectFileBase::k_material_uv;
			}
			return s_table;
		}
	}

	CEffectFileRHI::CEffectFileRHI(const char* filename)
	{
		for (int i = 0; i < k_max_param_handles; ++i)
			m_handleToSlot[i] = -1;
		std::memset(m_textureUsed, 0, sizeof(m_textureUsed));
		if (filename)
			m_filename = filename;
	}

	CEffectFileRHI::CEffectFileRHI(const AssetKey& key)
		: CEffectFileBase(key)
	{
		for (int i = 0; i < k_max_param_handles; ++i)
			m_handleToSlot[i] = -1;
		std::memset(m_textureUsed, 0, sizeof(m_textureUsed));
	}

	CEffectFileRHI::~CEffectFileRHI()
	{
		releaseEffect();
	}

	void CEffectFileRHI::SetFileName(const std::string& filename)
	{
		m_filename = filename;
	}

	const std::string& CEffectFileRHI::GetFileName() const
	{
		return m_filename;
	}

	void CEffectFileRHI::EnableTextures(bool bEnable)
	{
		g_bTextureEnabled = bEnable;
	}

	bool CEffectFileRHI::AreTextureEnabled()
	{
		return g_bTextureEnabled;
	}

	void CEffectFileRHI::EnableShareMode(bool bEnable)
	{
		m_bSharedMode = bEnable;
	}

	bool CEffectFileRHI::IsInShareMode() const
	{
		return m_bSharedMode;
	}

	bool CEffectFileRHI::BeginSharePassMode(bool bApplyParam, DWORD flag, bool bForceBegin)
	{
		EnableShareMode(true);
		if (begin(bApplyParam, flag, bForceBegin))
		{
			return BeginPass(0, true);
		}
		return false;
	}

	void CEffectFileRHI::EndSharePassMode()
	{
		EnableShareMode(false);
		EndPass(true);
		end(true);
	}

	void CEffectFileRHI::OnSwitchInShader()
	{
	}

	void CEffectFileRHI::OnSwitchOutShader()
	{
		std::fill(m_LastTextures.begin(), m_LastTextures.end(), nullptr);
	}

	int32_t CEffectFileRHI::FindOrCreateSlotByName(const std::string& name)
	{
		auto it = m_nameToSlot.find(name);
		if (it != m_nameToSlot.end())
			return it->second;

		int32_t idx = static_cast<int32_t>(m_paramSlots.size());
		ParamSlot slot;
		slot.name = name;
		slot.kind = EParamKind::Float;
		slot.data.resize(64, 0);
		m_paramSlots.push_back(std::move(slot));
		m_nameToSlot[name] = idx;
		return idx;
	}

	HRESULT CEffectFileRHI::InitDeviceObjects()
	{
		m_bIsInitialized = true;
		m_bIsValid = false;
		releaseEffect();

		IRenderDevice* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return E_FAIL;

		std::string fileBytes;
		std::string resolvedName = m_filename;
		std::string lowerFile = ToLowerStr(m_filename);
		std::replace(lowerFile.begin(), lowerFile.end(), '\\', '/');
		const bool endsWithFxo = lowerFile.size() > 4 && lowerFile.substr(lowerFile.size() - 4) == ".fxo";

		// Transparently replace script/apps/Aries/Creator/Game/Shaders/... with embedded unified runtime shaders
		// so legacy Paraworld script shaders stay untouched for the old D3D9 client.
		if (lowerFile.find("script/apps/aries/creator/game/shaders/") != std::string::npos)
		{
			const char* builtinRes = BuiltinEmbeddedByStem(BaseStemLower(m_filename));
			if (builtinRes && TryReadParaFile(builtinRes, fileBytes))
			{
				resolvedName = builtinRes;
			}
		}

		if (fileBytes.empty() && endsWithFxo)
		{
			if (!TryReadParaFile(ReplaceExt(m_filename, ".pfx"), fileBytes))
			{
				if (!TryReadParaFile(ReplaceExt(m_filename, ".fx"), fileBytes))
				{
					const std::string stem = BaseStemLower(m_filename);
					if (!TryReadParaFile("shaders/fx/" + stem + ".fx", fileBytes))
					{
						const char* builtinRes = BuiltinEmbeddedByStem(stem);
						if (builtinRes)
							TryReadParaFile(builtinRes, fileBytes);
					}
				}
			}
		}

		if (fileBytes.empty())
		{
			if (!TryReadParaFile(m_filename, fileBytes))
			{
				const std::string stem = BaseStemLower(m_filename);
				if (!TryReadParaFile("shaders/fx/" + stem + ".fx", fileBytes))
				{
					const char* builtinRes = BuiltinEmbeddedByStem(stem);
					if (builtinRes)
						TryReadParaFile(builtinRes, fileBytes);
				}
			}
		}

		if (!fileBytes.empty() && IsLegacyD3D9FxoBinary(fileBytes))
		{
			fileBytes.clear();
			const std::string stem = BaseStemLower(m_filename);
			if (!TryReadParaFile(ReplaceExt(m_filename, ".fx"), fileBytes))
			{
				if (!TryReadParaFile("shaders/fx/" + stem + ".fx", fileBytes))
				{
					const char* builtinRes = BuiltinEmbeddedByStem(stem);
					if (builtinRes)
						TryReadParaFile(builtinRes, fileBytes);
				}
			}
		}

		if (fileBytes.empty())
		{
			OUTPUT_LOG("ERROR: CEffectFileRHI shader file %s not found\n", m_filename.c_str());
			return E_FAIL;
		}

		const ERHIBackend backend = pDevice->GetBackendType();
		const EShaderTarget target = SelectTargetForDevice(pDevice);

		// Seven mask/detail pairs plus base and shadow occupy all sixteen fragment samplers.
		const bool extendedTerrainTarget = target == EShaderTarget::ESSL_300 ||
			target == EShaderTarget::GLSL_120 || target == EShaderTarget::GLSL_330;
		if (resolvedName == ":IDR_FX_TERRAIN_NORMAL" && extendedTerrainTarget &&
			pDevice->GetMaxSimultaneousTextures() >= 16)
			fileBytes.insert(0, "#define TERRAIN_SEVEN_LAYERS 1\n");

		if (resolvedName == ":IDR_FX_GUI_BATCH" && backend == ERHIBackend::OpenGL &&
			pDevice->GetMaxSimultaneousTextures() >= 8)
			fileBytes.insert(0, "#define GUI_BATCH_EIGHT_TEXTURES 1\n");

		FxCompiledEffect compiledEffect;
		if (IsPfxBinary(fileBytes))
		{
			EShaderTarget extractTarget = (target == EShaderTarget::Unknown) ? EShaderTarget::DXBC_SM40 : target;
			if (!CFxCompiler::ExtractTargetFromPFX(fileBytes.data(), fileBytes.size(), extractTarget, compiledEffect))
			{
				OUTPUT_LOG("ERROR: CEffectFileRHI failed extracting PFX target for %s\n", m_filename.c_str());
				return E_FAIL;
			}
		}
		else if (backend == ERHIBackend::Null)
		{
			FxParsedDocument doc;
			std::string parseErr;
			if (!CFxParserV2::Parse(fileBytes, resolvedName, doc, parseErr, MakeParaIncludeResolver(resolvedName)))
			{
				OUTPUT_LOG("ERROR: CEffectFileRHI parse failed for %s: %s\n", m_filename.c_str(), parseErr.c_str());
				return E_FAIL;
			}
			compiledEffect.sourceName = doc.sourceName;
			compiledEffect.params = doc.params;
			compiledEffect.samplers = doc.samplers;
			for (const auto& pt : doc.techniques)
			{
				FxCompiledTechnique ct;
				ct.name = pt.name;
				for (const auto& pp : pt.passes)
				{
					FxCompiledPass cp;
					cp.name = pp.name;
					cp.renderStates = pp.renderStates;
					cp.vs.valid = true;
					cp.ps.valid = !pp.psExplicitNull;
					ct.passes.push_back(std::move(cp));
				}
				compiledEffect.techniques.push_back(std::move(ct));
			}
		}
		else
		{
			std::string compileErr;
			if (!CFxCompiler::Compile(fileBytes, resolvedName, target, compiledEffect, compileErr, MakeParaIncludeResolver(resolvedName)))
			{
				OUTPUT_LOG("ERROR: CEffectFileRHI compile failed for %s: %s\n", m_filename.c_str(), compileErr.c_str());
				return E_FAIL;
			}
		}

		if (compiledEffect.techniques.empty())
		{
			OUTPUT_LOG("ERROR: CEffectFileRHI %s has no techniques\n", m_filename.c_str());
			return E_FAIL;
		}

		// Populate parameter slots and semantic mappings
		m_paramSlots.clear();
		m_nameToSlot.clear();
		for (int i = 0; i < k_max_param_handles; ++i)
			m_handleToSlot[i] = -1;
		std::memset(m_textureUsed, 0, sizeof(m_textureUsed));
		m_samplers.clear();
		m_textureNameToStage.clear();

		const auto& semTable = GetSemanticTableLower();
		static const char kNumerals[] = "0123456789";

		for (const auto& p : compiledEffect.params)
		{
			ParamSlot slot;
			slot.name = p.name;
			slot.semantic = p.semantic;
			slot.elementCount = p.elementCount > 1 ? static_cast<uint32_t>(p.elementCount) : 1u;

			std::string lowerType = ToLowerStr(p.typeName);
			if (p.isBool || lowerType == "bool")
				slot.kind = EParamKind::Bool;
			else if (p.isInt || lowerType == "int" || lowerType == "uint" || lowerType == "dword")
				slot.kind = EParamKind::Int;
			else if (lowerType.find("4x4") != std::string::npos || lowerType == "matrix")
				slot.kind = EParamKind::Matrix;
			else if (lowerType == "slightscatteringshaderparams")
				slot.kind = EParamKind::RawStruct;
			else
				slot.kind = EParamKind::Float;

			uint32_t scalarBytes = EstimateScalarElementBytes(p);
			slot.scalarElemBytes = scalarBytes <= 255u ? static_cast<uint8_t>(scalarBytes) : 16u;
			slot.data.assign(EstimateTypeByteSize(p), 0);

			int32_t slotIdx = static_cast<int32_t>(m_paramSlots.size());
			m_paramSlots.push_back(std::move(slot));
			m_nameToSlot[p.name] = slotIdx;

			if (!p.semantic.empty())
			{
				std::string semLower = ToLowerStr(p.semantic);
				auto it = semTable.find(semLower);
				if (it != semTable.end())
				{
					uint32_t handle = it->second;
					if (semLower == "materialspecular")
					{
						handle = (m_paramSlots[slotIdx].scalarElemBytes >= 12)
							? static_cast<uint32_t>(CEffectFileBase::k_specularMaterialColor)
							: static_cast<uint32_t>(CEffectFileBase::k_material_specular);
					}
					else if (semLower == "materialemissive")
					{
						handle = (m_paramSlots[slotIdx].scalarElemBytes >= 12)
							? static_cast<uint32_t>(CEffectFileBase::k_emissiveMaterialColor)
							: static_cast<uint32_t>(CEffectFileBase::k_material_emissive);
					}
					m_handleToSlot[handle] = slotIdx;
				}
				else if (StartsWithStr(semLower, "texmat"))
				{
					size_t digitPos = p.name.find_first_of(kNumerals);
					if (digitPos != std::string::npos)
					{
						int texIdx = std::atoi(p.name.c_str() + digitPos);
						if (texIdx >= 0 && texIdx < (k_tex_mat_max - k_tex_mat0))
							m_handleToSlot[k_tex_mat0 + texIdx] = slotIdx;
					}
				}
			}
			else if (ToLowerStr(p.name) == "curnumbones")
			{
				m_handleToSlot[k_boneInfluenceCount] = slotIdx;
			}

			// Apply default initializers declared in .fx
			if (p.hasDefault && !p.defaultFloats.empty())
			{
				if (p.isBool)
				{
					int32_t bval = (p.defaultFloats[0] != 0.0f) ? 1 : 0;
					WriteSlotBytes(slotIdx, &bval, 0, sizeof(bval), EParamKind::Bool);
				}
				else if (p.isInt)
				{
					int32_t ival = static_cast<int32_t>(p.defaultFloats[0]);
					WriteSlotBytes(slotIdx, &ival, 0, sizeof(ival), EParamKind::Int);
				}
				else
				{
					WriteSlotBytes(slotIdx, p.defaultFloats.data(), 0, static_cast<uint32_t>(p.defaultFloats.size() * sizeof(float)), EParamKind::Float);
				}
			}
		}

		// Populate sampler & texture bindings
		for (size_t i = 0; i < compiledEffect.samplers.size(); ++i)
		{
			const auto& s = compiledEffect.samplers[i];
			int stage = (s.registerIndex >= 0) ? s.registerIndex : static_cast<int>(i);
			if (stage >= 0 && stage < 16)
			{
				m_textureUsed[stage] = true;
				if (!s.textureName.empty())
					m_textureNameToStage[s.textureName] = stage;
				if (!s.samplerName.empty())
					m_textureNameToStage[s.samplerName] = stage;
			}
			SamplerDescEntry sd;
			sd.samplerName = s.samplerName;
			sd.textureName = s.textureName;
			sd.registerIndex = stage;
			sd.hasMinFilter = s.hasMinFilter;
			sd.hasMagFilter = s.hasMagFilter;
			sd.hasMipFilter = s.hasMipFilter;
			sd.hasAddressU = s.hasAddressU;
			sd.hasAddressV = s.hasAddressV;
			sd.hasBorderColor = s.hasBorderColor;
			sd.minFilter = s.minFilter;
			sd.magFilter = s.magFilter;
			sd.mipFilter = s.mipFilter;
			sd.addressU = s.addressU;
			sd.addressV = s.addressV;
			sd.borderColor = s.borderColor;
			m_samplers.push_back(std::move(sd));
		}

		// Build runtime techniques & passes (gracefully skipping techniques unsupported by current GPU feature level)
		int firstDefaultTech = -1;
		for (size_t tIdx = 0; tIdx < compiledEffect.techniques.size(); ++tIdx)
		{
			const auto& ct = compiledEffect.techniques[tIdx];
			TechniqueRuntime rtTech;
			rtTech.name = ct.name;
			if (ct.name == "GenShadowMap")
				rtTech.nCategory = TechCategory_GenShadowMap;
			else if (ct.name == "Vegetation")
				rtTech.nCategory = TechCategory_Vegetation;
			else if (ct.name == "Character")
				rtTech.nCategory = TechCategory_Character;
			else if (ct.name == "DetailCharacter")
				rtTech.nCategory = TechCategory_DetailCharacter;
			else
				rtTech.nCategory = TechCategory_default;

			bool techSupported = true;
			for (size_t pIdx = 0; pIdx < ct.passes.size(); ++pIdx)
			{
				const auto& cp = ct.passes[pIdx];
				PassRuntime rtPass;
				rtPass.name = cp.name;

				for (const auto& rs : cp.renderStates)
				{
					PassStateEntry pse;
					pse.state = rs.state;
					pse.value = rs.value;
					rtPass.renderStates.push_back(pse);
				}

				if (!cp.vs.valid && !cp.ps.valid)
				{
					// State-only pass (e.g. Glow_5Tap pass 2 placeholder in fullscreen_glow.fx)
					rtTech.passes.push_back(std::move(rtPass));
					continue;
				}

				if (backend == ERHIBackend::OpenGL)
				{
#ifdef PARA_ENABLE_OPENGL
					std::string vsText(reinterpret_cast<const char*>(cp.vs.bytecode.data()), cp.vs.bytecode.size());
					std::string psText(reinterpret_cast<const char*>(cp.ps.bytecode.data()), cp.ps.bytecode.size());
					GLProgram* prog = new GLProgram();
					if (!prog->initWithByteArrays(vsText.c_str(), psText.c_str()) || !prog->link())
					{
						OUTPUT_LOG("WARN: CEffectFileRHI skipping unsupported OpenGL technique '%s' in %s (pass %zu)\n",
							ct.name.c_str(), m_filename.c_str(), pIdx);
						SAFE_RELEASE(prog);
						techSupported = false;
						break;
					}
					prog->updateUniforms();
					for (const auto& sd : m_samplers)
					{
						if (sd.registerIndex >= 0 && !sd.samplerName.empty())
						{
							GLint loc = prog->getUniformLocation(sd.samplerName);
							if (loc >= 0)
								prog->setUniformLocationWith1i(loc, sd.registerIndex);
						}
					}
					rtPass.glProgram = prog;

					// Create bindings for all effect slots present in this GLProgram
					for (int32_t sIdx = 0; sIdx < static_cast<int32_t>(m_paramSlots.size()); ++sIdx)
					{
						if (const auto* uniform = prog->getUniform(m_paramSlots[sIdx].name))
						{
							PassParamBinding b;
							b.slotIndex = sIdx;
							b.glUniform = uniform;
							rtPass.bindings.push_back(b);
						}
					}
#endif
				}
				else
				{
					RHIShaderProgramDesc progDesc;
					progDesc.vsBytecode = cp.vs.bytecode.empty() ? nullptr : cp.vs.bytecode.data();
					progDesc.vsBytecodeSize = static_cast<uint32_t>(cp.vs.bytecode.size());
					progDesc.psBytecode = cp.ps.bytecode.empty() ? nullptr : cp.ps.bytecode.data();
					progDesc.psBytecodeSize = static_cast<uint32_t>(cp.ps.bytecode.size());
					progDesc.vsGlobalCBufferSize = cp.vs.cbufferSize;
					progDesc.psGlobalCBufferSize = cp.ps.cbufferSize;
					progDesc.vsEntryPoint = cp.vs.entryPoint.c_str();
					progDesc.psEntryPoint = cp.ps.entryPoint.c_str();
					progDesc.debugName = m_filename.c_str();

					rtPass.rhiProgram = pDevice->CompileShaderProgram(progDesc);
					if (!rtPass.rhiProgram || !rtPass.rhiProgram->IsValid())
					{
						OUTPUT_LOG("INFO: CEffectFileRHI skipping technique '%s' in %s (pass %zu exceeds current GPU feature level)\n",
							ct.name.c_str(), m_filename.c_str(), pIdx);
						if (rtPass.rhiProgram)
							rtPass.rhiProgram->ReleaseRHIResource();
						rtPass.rhiProgram = nullptr;
						techSupported = false;
						break;
					}

					if (cp.vs.cbufferSize > 0)
						rtPass.vsCBuffer.assign((cp.vs.cbufferSize + 15u) & ~15u, 0);
					if (cp.ps.cbufferSize > 0)
						rtPass.psCBuffer.assign((cp.ps.cbufferSize + 15u) & ~15u, 0);

					// Also mark any texture bindings reflected in shader bytecode as used
					for (const auto& t : cp.vs.textures)
						if (t.bindPoint < 16) m_textureUsed[t.bindPoint] = true;
					for (const auto& t : cp.ps.textures)
						if (t.bindPoint < 16) m_textureUsed[t.bindPoint] = true;

					// Match reflected cbuffer members to ParamSlots (creating slots if reflection found extra members)
					std::unordered_map<int32_t, size_t> slotToBindingIdx;
					for (const auto& m : cp.vs.cbufferMembers)
					{
						int32_t sIdx = FindOrCreateSlotByName(m.name);
						if (m_paramSlots[sIdx].data.size() < m.byteSize)
							m_paramSlots[sIdx].data.resize(m.byteSize, 0);
						PassParamBinding b;
						b.slotIndex = sIdx;
						b.vsOffset = static_cast<int32_t>(m.byteOffset);
						b.vsSize = m.byteSize;
						slotToBindingIdx[sIdx] = rtPass.bindings.size();
						rtPass.bindings.push_back(b);
					}
					for (const auto& m : cp.ps.cbufferMembers)
					{
						int32_t sIdx = FindOrCreateSlotByName(m.name);
						if (m_paramSlots[sIdx].data.size() < m.byteSize)
							m_paramSlots[sIdx].data.resize(m.byteSize, 0);
						auto bit = slotToBindingIdx.find(sIdx);
						if (bit != slotToBindingIdx.end())
						{
							rtPass.bindings[bit->second].psOffset = static_cast<int32_t>(m.byteOffset);
							rtPass.bindings[bit->second].psSize = m.byteSize;
						}
						else
						{
							PassParamBinding b;
							b.slotIndex = sIdx;
							b.psOffset = static_cast<int32_t>(m.byteOffset);
							b.psSize = m.byteSize;
							rtPass.bindings.push_back(b);
						}
					}
				}

				rtTech.passes.push_back(std::move(rtPass));
			}

			if (!techSupported || rtTech.passes.empty())
			{
				for (auto& partialPass : rtTech.passes)
				{
					if (partialPass.rhiProgram)
					{
						partialPass.rhiProgram->ReleaseRHIResource();
						partialPass.rhiProgram = nullptr;
					}
#ifdef PARA_ENABLE_OPENGL
					SAFE_RELEASE(partialPass.glProgram);
#endif
				}
				continue;
			}

			if (rtTech.nCategory == TechCategory_default && firstDefaultTech < 0)
				firstDefaultTech = static_cast<int>(m_techniques.size());
			m_techniques.push_back(std::move(rtTech));
		}

		if (m_techniques.empty())
		{
			OUTPUT_LOG("ERROR: CEffectFileRHI no valid techniques compiled for %s\n", m_filename.c_str());
			releaseEffect();
			return E_FAIL;
		}

		m_nTechniqueIndex = (firstDefaultTech >= 0) ? firstDefaultTech : 0;
		m_nActivePassIndex = 0;

		if (resolvedName == ":IDR_FX_TERRAIN_NORMAL")
		{
			// FX metadata includes declarations inside inactive #ifdef branches.
			// Terrain uses layersnum to choose seven layers per draw, so only expose
			// it when the compiled default technique actually binds the parameter.
			const int32_t layerSlot = m_handleToSlot[k_LayersNum];
			bool hasLayerBinding = false;
			for (const auto& pass : m_techniques[m_nTechniqueIndex].passes)
				for (const auto& binding : pass.bindings)
					if (layerSlot >= 0 && binding.slotIndex == layerSlot)
						hasLayerBinding = true;
			if (!hasLayerBinding)
				m_handleToSlot[k_LayersNum] = -1;
		}

		if (m_filename == ":IDR_FX_SIMPLE_MESH_NORMAL")
			setFloat(k_opacity, 1.0f);
		if (m_filename == ":IDR_FX_GUI")
			SetBool("k_bBoolean0", TRUE);

		m_bIsValid = true;
		return S_OK;
	}

	HRESULT CEffectFileRHI::RestoreDeviceObjects()
	{
		return S_OK;
	}

	HRESULT CEffectFileRHI::InvalidateDeviceObjects()
	{
		return S_OK;
	}

	HRESULT CEffectFileRHI::DeleteDeviceObjects()
	{
		releaseEffect();
		m_bIsInitialized = false;
		m_bIsValid = false;
		return S_OK;
	}

	HRESULT CEffectFileRHI::RendererRecreated()
	{
#ifdef PARA_ENABLE_OPENGL
		for (auto& tech : m_techniques)
		{
			for (auto& pass : tech.passes)
			{
				if (pass.glProgram)
					pass.glProgram->reset();
			}
		}
#endif
		releaseEffect();
		m_bIsInitialized = false;
		m_bIsValid = false;
		m_nTechniqueIndex = 0;
		m_nActivePassIndex = 0;
		return S_OK;
	}

	void CEffectFileRHI::releaseEffect(int nTech, int nPass)
	{
		auto releasePass = [](PassRuntime& pass) {
#ifdef PARA_ENABLE_OPENGL
			if (pass.glProgram)
			{
				SAFE_RELEASE(pass.glProgram);
			}
#endif
			if (pass.rhiProgram)
			{
				pass.rhiProgram->ReleaseRHIResource();
				pass.rhiProgram = nullptr;
			}
		};

		if (nTech < 0)
		{
			for (auto& tech : m_techniques)
			{
				if (nPass < 0)
				{
					for (auto& p : tech.passes)
						releasePass(p);
					tech.passes.clear();
				}
				else if (nPass < static_cast<int>(tech.passes.size()))
				{
					releasePass(tech.passes[nPass]);
				}
			}
			if (nPass < 0)
				m_techniques.clear();
		}
		else if (nTech < static_cast<int>(m_techniques.size()))
		{
			auto& tech = m_techniques[nTech];
			if (nPass < 0)
			{
				for (auto& p : tech.passes)
					releasePass(p);
				tech.passes.clear();
			}
			else if (nPass < static_cast<int>(tech.passes.size()))
			{
				releasePass(tech.passes[nPass]);
			}
		}
	}

	int CEffectFileRHI::totalPasses() const
	{
		if (m_nTechniqueIndex >= 0 && m_nTechniqueIndex < static_cast<int>(m_techniques.size()))
			return static_cast<int>(m_techniques[m_nTechniqueIndex].passes.size());
		return 0;
	}

	bool CEffectFileRHI::SetFirstValidTechniqueByCategory(TechniqueCategory nCat)
	{
		if (m_nTechniqueIndex >= 0 && m_nTechniqueIndex < static_cast<int>(m_techniques.size()) &&
			m_techniques[m_nTechniqueIndex].nCategory == nCat)
		{
			return true;
		}
		for (int i = 0; i < static_cast<int>(m_techniques.size()); ++i)
		{
			if (m_techniques[i].nCategory == nCat)
			{
				m_nTechniqueIndex = i;
				return true;
			}
		}
		return false;
	}

	bool CEffectFileRHI::SetTechniqueByIndex(int nIndex)
	{
		if (nIndex >= 0 && nIndex < static_cast<int>(m_techniques.size()))
		{
			m_nTechniqueIndex = nIndex;
			return true;
		}
		return false;
	}

	const CEffectFileBase::TechniqueDesc* CEffectFileRHI::GetCurrentTechniqueDesc()
	{
		static const TechniqueDesc s_defaultDesc;
		if (m_nTechniqueIndex >= 0 && m_nTechniqueIndex < static_cast<int>(m_techniques.size()))
			return &m_techniques[m_nTechniqueIndex];
		return &s_defaultDesc;
	}

	int CEffectFileRHI::GetCurrentTechniqueIndex() const
	{
		return m_nTechniqueIndex;
	}

	bool CEffectFileRHI::isParameterUsed(eParameterHandles index) const
	{
		return index >= 0 && index < k_max_param_handles && m_handleToSlot[index] >= 0;
	}

	bool CEffectFileRHI::isMatrixUsed(eParameterHandles index) const
	{
		return isParameterUsed(index);
	}

	bool CEffectFileRHI::isTextureUsed(int index) const
	{
		if (index < 0 || index >= 16)
			return false;
		// If shader declared no explicit samplers (or stage is marked), report accurately
		return m_textureUsed[index] || m_samplers.empty();
	}

	bool CEffectFileRHI::isTextureMatrixUsed(int index) const
	{
		return index >= 0 && (k_tex_mat0 + index) < k_tex_mat_max && m_handleToSlot[k_tex_mat0 + index] >= 0;
	}

	bool CEffectFileRHI::WriteSlotBytes(int32_t slotIndex, const void* pData, uint32_t byteOffset, uint32_t byteCount, EParamKind writeKind)
	{
		if (slotIndex < 0 || slotIndex >= static_cast<int32_t>(m_paramSlots.size()) || !pData || byteCount == 0)
			return false;

		ParamSlot& slot = m_paramSlots[slotIndex];
		if (byteOffset + byteCount > slot.data.size())
		{
			// Allow array/struct expansion when caller provides valid larger array payload,
			// while keeping scalar/vec3 safe inside HLSL cbuffer bounds during SyncPassUniforms.
			slot.data.resize(byteOffset + byteCount, 0);
		}

		if (std::memcmp(slot.data.data() + byteOffset, pData, byteCount) != 0 || slot.revision == 0)
		{
			std::memcpy(slot.data.data() + byteOffset, pData, byteCount);
			++slot.revision;
			++m_paramRevision;
		}
		(void)writeKind;
		return true;
	}

	void CEffectFileRHI::SyncPassUniforms(PassRuntime& pass, bool isOpenGL)
	{
		if (isOpenGL)
		{
#ifdef PARA_ENABLE_OPENGL
			GLProgram* program = pass.glProgram;
			if (!program)
				return;
			if (pass.syncedGLRevision == m_paramRevision)
			{
				// The program can still have pending direct uniform writes.
				program->commit();
				return;
			}
			pass.syncedGLRevision = m_paramRevision;

			for (auto& b : pass.bindings)
			{
				const ParamSlot& slot = m_paramSlots[b.slotIndex];
				if (slot.revision == 0 || b.syncedRevision == slot.revision)
					continue;

				const Uniform* u = b.glUniform;
				if (u && !slot.data.empty())
				{
					const void* raw = slot.data.data();
					if (u->type == GL_INT || u->type == GL_BOOL)
					{
						int32_t v = 0;
						std::memcpy(&v, raw, (std::min)(sizeof(v), slot.data.size()));
						program->setUniformLocationWith1i(u->location, v);
					}
					else if (u->type == GL_FLOAT)
					{
						if (u->size > 1 && slot.elementCount > 1 && slot.scalarElemBytes == 4)
						{
							uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 4)));
							if (count > 0)
							{
								std::vector<float> packed(count);
								const float* src = reinterpret_cast<const float*>(raw);
								for (uint32_t elemIdx = 0; elemIdx < count; ++elemIdx)
									packed[elemIdx] = src[elemIdx * 4];
								glUseProgram(program->getProgram());
								glUniform1fv(u->location, static_cast<GLsizei>(count), packed.data());
							}
						}
						else
						{
							uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / sizeof(float)));
							if (count <= 1)
								program->setUniformLocationWith1f(u->location, *reinterpret_cast<const float*>(raw));
							else
							{
								glUseProgram(program->getProgram());
								glUniform1fv(u->location, static_cast<GLsizei>(count), reinterpret_cast<const GLfloat*>(raw));
							}
						}
					}
					else if (u->type == GL_FLOAT_VEC2)
					{
						if (u->size > 1 && slot.elementCount > 1 && slot.scalarElemBytes == 8)
						{
							uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 4)));
							if (count > 0)
							{
								std::vector<float> packed(count * 2);
								const float* src = reinterpret_cast<const float*>(raw);
								for (uint32_t elemIdx = 0; elemIdx < count; ++elemIdx)
								{
									packed[elemIdx * 2 + 0] = src[elemIdx * 4 + 0];
									packed[elemIdx * 2 + 1] = src[elemIdx * 4 + 1];
								}
								program->setUniformLocationWith2fv(u->location, packed.data(), count);
							}
						}
						else
						{
							uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 2)));
							if (count > 0)
								program->setUniformLocationWith2fv(u->location, reinterpret_cast<const float*>(raw), count);
						}
					}
					else if (u->type == GL_FLOAT_VEC3)
					{
						if (u->size > 1 && slot.elementCount > 1)
						{
							uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 4)));
							if (count > 0)
							{
								std::vector<float> packed(count * 3);
								const float* src = reinterpret_cast<const float*>(raw);
								for (uint32_t elemIdx = 0; elemIdx < count; ++elemIdx)
								{
									packed[elemIdx * 3 + 0] = src[elemIdx * 4 + 0];
									packed[elemIdx * 3 + 1] = src[elemIdx * 4 + 1];
									packed[elemIdx * 3 + 2] = src[elemIdx * 4 + 2];
								}
								program->setUniformLocationWith3fv(u->location, packed.data(), count);
							}
						}
						else
						{
							uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 3)));
							if (count > 0)
								program->setUniformLocationWith3fv(u->location, reinterpret_cast<const float*>(raw), count);
						}
					}
					else if (u->type == GL_FLOAT_VEC4)
					{
						uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 4)));
						if (count > 0)
							program->setUniformLocationWith4fv(u->location, reinterpret_cast<const float*>(raw), count);
					}
					else if (u->type == GL_FLOAT_MAT4)
					{
						uint32_t count = (std::min)(static_cast<uint32_t>(u->size), static_cast<uint32_t>(slot.data.size() / (sizeof(float) * 16)));
						if (count > 0)
							program->setUniformLocationWithMatrix4fv(u->location, reinterpret_cast<const float*>(raw), count);
					}
				}
				b.syncedRevision = slot.revision;
			}
			program->commit();
#endif
		}
		else
		{
			for (auto& b : pass.bindings)
			{
				const ParamSlot& slot = m_paramSlots[b.slotIndex];
				if (slot.revision == 0 || b.syncedRevision == slot.revision)
					continue;

				if (b.vsOffset >= 0 && b.vsSize > 0 && static_cast<size_t>(b.vsOffset) + b.vsSize <= pass.vsCBuffer.size())
				{
					uint32_t copyBytes = (std::min)(b.vsSize, static_cast<uint32_t>(slot.data.size()));
					std::memcpy(pass.vsCBuffer.data() + b.vsOffset, slot.data.data(), copyBytes);
					pass.cbufferDirty = true;
				}
				if (b.psOffset >= 0 && b.psSize > 0 && static_cast<size_t>(b.psOffset) + b.psSize <= pass.psCBuffer.size())
				{
					uint32_t copyBytes = (std::min)(b.psSize, static_cast<uint32_t>(slot.data.size()));
					std::memcpy(pass.psCBuffer.data() + b.psOffset, slot.data.data(), copyBytes);
					pass.cbufferDirty = true;
				}
				b.syncedRevision = slot.revision;
			}

			if (pass.cbufferDirty)
			{
				IRenderDevice* pDevice = CGlobals::GetRenderDevice();
				if (pDevice)
				{
					pDevice->SetShaderConstants(
						pass.vsCBuffer.empty() ? nullptr : pass.vsCBuffer.data(),
						static_cast<uint32_t>(pass.vsCBuffer.size()),
						pass.psCBuffer.empty() ? nullptr : pass.psCBuffer.data(),
						static_cast<uint32_t>(pass.psCBuffer.size()));
				}
				pass.cbufferDirty = false;
			}
		}
	}

	void CEffectFileRHI::ApplyPassStates(const PassRuntime& pass)
	{
		IRenderDevice* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return;

		const bool saveRenderStates = (m_dwBeginFlags & D3DXFX_DONOTSAVESTATE) == 0;
		const bool saveSamplerStates = (m_dwBeginFlags & D3DXFX_DONOTSAVESAMPLERSTATE) == 0;

		for (const auto& rs : pass.renderStates)
		{
			if (rs.state == ERenderState::Unknown)
				continue;
			if (saveRenderStates)
			{
				SavedRenderState saved;
				saved.state = rs.state;
				saved.oldValue = pDevice->GetRenderState(rs.state);
				m_savedRenderStates.push_back(saved);
			}
			pDevice->SetRenderState(rs.state, rs.value);
		}

		EffectManager* pEffectMgr = CGlobals::GetEffectManager();
		for (const auto& sd : m_samplers)
		{
			if (sd.registerIndex < 0 || sd.registerIndex >= 16)
				continue;
			const uint32_t stage = static_cast<uint32_t>(sd.registerIndex);
			auto applySamplerState = [&](ESamplerStateType type, uint32_t val) {
				if (saveSamplerStates)
				{
					uint32_t prev = 0;
					pDevice->GetSamplerState(stage, type, &prev);
					m_savedSamplerStates.push_back({ stage, type, prev });
				}
				if (pEffectMgr)
					pEffectMgr->SetSamplerState(stage, type, val);
				else
					pDevice->SetSamplerState(stage, type, val);
			};

			if (sd.hasMinFilter) applySamplerState(ESamplerStateType::MINFILTER, sd.minFilter);
			if (sd.hasMagFilter) applySamplerState(ESamplerStateType::MAGFILTER, sd.magFilter);
			if (sd.hasMipFilter) applySamplerState(ESamplerStateType::MIPFILTER, sd.mipFilter);
			if (sd.hasAddressU)  applySamplerState(ESamplerStateType::ADDRESSU, sd.addressU);
			if (sd.hasAddressV)  applySamplerState(ESamplerStateType::ADDRESSV, sd.addressV);
			if (sd.hasBorderColor) applySamplerState(ESamplerStateType::BORDERCOLOR, sd.borderColor);
		}

		for (size_t stage = 0; stage < m_NamedStageTextures.size(); ++stage)
		{
			if (m_NamedStageTextures[stage] != nullptr)
			{
				DeviceTexturePtr_type tex = m_NamedStageTextures[stage]->GetTexture();
				if (tex)
					pDevice->SetTexture(static_cast<uint32_t>(stage), tex);
			}
		}
	}

	void CEffectFileRHI::RestoreSavedStates()
	{
		IRenderDevice* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
		{
			m_savedRenderStates.clear();
			m_savedSamplerStates.clear();
			return;
		}

		for (auto it = m_savedRenderStates.rbegin(); it != m_savedRenderStates.rend(); ++it)
		{
			pDevice->SetRenderState(it->state, it->oldValue);
		}
		m_savedRenderStates.clear();

		EffectManager* pEffectMgr = CGlobals::GetEffectManager();
		for (auto it = m_savedSamplerStates.rbegin(); it != m_savedSamplerStates.rend(); ++it)
		{
			if (pEffectMgr)
				pEffectMgr->SetSamplerState(static_cast<int>(it->stage), it->type, it->oldValue);
			else
				pDevice->SetSamplerState(it->stage, it->type, it->oldValue);
		}
		m_savedSamplerStates.clear();
	}

	bool CEffectFileRHI::begin(bool bApplyParam, DWORD flag, bool bForceBegin)
	{
		if (!m_bIsValid || m_nTechniqueIndex < 0 || m_nTechniqueIndex >= static_cast<int>(m_techniques.size()))
			return false;

		if (bApplyParam)
		{
			IScene* pScene = CGlobals::GetEffectManager()->GetScene();
			if (pScene)
				applyGlobalLightingData(pScene->GetSunLight());
			applyCameraMatrices();
		}

		if (bForceBegin || !m_bSharedMode)
		{
			m_dwBeginFlags = flag;
			m_bIsBegin = true;
		}
		return true;
	}

	bool CEffectFileRHI::BeginPass(int pass, bool bForceBegin)
	{
		if (!m_bIsValid || m_nTechniqueIndex < 0 || m_nTechniqueIndex >= static_cast<int>(m_techniques.size()))
			return false;

		auto& tech = m_techniques[m_nTechniqueIndex];
		if (pass < 0 || pass >= static_cast<int>(tech.passes.size()))
			return false;

		if (bForceBegin || !m_bSharedMode)
		{
			m_nActivePassIndex = pass;
			m_bIsBeginPass = true;

			PassRuntime& rtPass = tech.passes[pass];
			ApplyPassStates(rtPass);

			IRenderDevice* pDevice = CGlobals::GetRenderDevice();
			const bool isOpenGL = pDevice && pDevice->GetBackendType() == ERHIBackend::OpenGL;
			if (isOpenGL)
			{
#ifdef PARA_ENABLE_OPENGL
				if (rtPass.glProgram)
					rtPass.glProgram->use();
#endif
			}
			else
			{
				if (!pDevice)
					return false;
				if (rtPass.rhiProgram && !pDevice->BindShaderProgram(rtPass.rhiProgram))
					return false;
				rtPass.cbufferDirty = true; // Force cbuffer rebind on pass switch
			}

			SyncPassUniforms(rtPass, isOpenGL);
		}
		return true;
	}

	void CEffectFileRHI::CommitChanges()
	{
		if (!m_bIsValid || m_nTechniqueIndex < 0 || m_nTechniqueIndex >= static_cast<int>(m_techniques.size()))
			return;
		auto& tech = m_techniques[m_nTechniqueIndex];
		if (m_nActivePassIndex < 0 || m_nActivePassIndex >= static_cast<int>(tech.passes.size()))
			return;

		IRenderDevice* pDevice = CGlobals::GetRenderDevice();
		const bool isOpenGL = pDevice && pDevice->GetBackendType() == ERHIBackend::OpenGL;
		SyncPassUniforms(tech.passes[m_nActivePassIndex], isOpenGL);
	}

	void CEffectFileRHI::EndPass(bool bForceEnd)
	{
		if (bForceEnd || !m_bSharedMode)
		{
			if (m_bIsBeginPass)
			{
				RestoreSavedStates();
				m_bIsBeginPass = false;
			}
		}
	}

	void CEffectFileRHI::end(bool bForceEnd)
	{
		if (bForceEnd || !m_bSharedMode)
		{
			if (m_bIsBegin)
			{
				if (m_bIsBeginPass)
				{
					RestoreSavedStates();
					m_bIsBeginPass = false;
				}
				if (!m_bSharedMode)
					OnSwitchOutShader();
				m_bIsBegin = false;
			}
		}
	}

	bool CEffectFileRHI::setParameter(eParameterHandles index, const void* data, int32 size)
	{
		if (!isParameterUsed(index) || !data)
			return false;
		int32_t slotIdx = m_handleToSlot[index];
		uint32_t byteSize = (size > 0 && size != D3DX_DEFAULT)
			? static_cast<uint32_t>(size)
			: static_cast<uint32_t>(m_paramSlots[slotIdx].data.size());
		if (byteSize > m_paramSlots[slotIdx].data.size() && size == D3DX_DEFAULT)
			byteSize = static_cast<uint32_t>(m_paramSlots[slotIdx].data.size());
		// Clamp default-sized Vector4 writes into float3 slots cleanly
		if (size == D3DX_DEFAULT && byteSize == 12)
			return WriteSlotBytes(slotIdx, data, 0, 12, EParamKind::Float);
		return WriteSlotBytes(slotIdx, data, 0, byteSize, m_paramSlots[slotIdx].kind);
	}

	bool CEffectFileRHI::setParameter(eParameterHandles index, const Vector2* data)
	{
		return data ? setParameter(index, data, sizeof(Vector2)) : false;
	}

	bool CEffectFileRHI::setParameter(eParameterHandles index, const Vector3* data)
	{
		return data ? setParameter(index, data, sizeof(Vector3)) : false;
	}

	bool CEffectFileRHI::setParameter(eParameterHandles index, const Vector4* data)
	{
		if (!isParameterUsed(index) || !data)
			return false;
		int32_t slotIdx = m_handleToSlot[index];
		uint32_t targetSize = (std::min)(static_cast<uint32_t>(sizeof(Vector4)), static_cast<uint32_t>(m_paramSlots[slotIdx].data.size()));
		return WriteSlotBytes(slotIdx, data, 0, targetSize > 0 ? targetSize : sizeof(Vector4), EParamKind::Float);
	}

	bool CEffectFileRHI::setBool(eParameterHandles index, BOOL bBoolean)
	{
		if (!isParameterUsed(index))
			return false;
		int32_t val = bBoolean ? 1 : 0;
		return WriteSlotBytes(m_handleToSlot[index], &val, 0, sizeof(val), EParamKind::Bool);
	}

	bool CEffectFileRHI::setInt(eParameterHandles index, int nValue)
	{
		if (!isParameterUsed(index))
			return false;
		int32_t slotIdx = m_handleToSlot[index];
		if (m_paramSlots[slotIdx].kind == EParamKind::Float)
		{
			float fval = static_cast<float>(nValue);
			return WriteSlotBytes(slotIdx, &fval, 0, sizeof(fval), EParamKind::Float);
		}
		int32_t val = static_cast<int32_t>(nValue);
		return WriteSlotBytes(slotIdx, &val, 0, sizeof(val), EParamKind::Int);
	}

	bool CEffectFileRHI::setFloat(eParameterHandles index, float fValue)
	{
		if (!isParameterUsed(index))
			return false;
		return WriteSlotBytes(m_handleToSlot[index], &fValue, 0, sizeof(fValue), EParamKind::Float);
	}

	bool CEffectFileRHI::setMatrix(eParameterHandles index, const Matrix4* data)
	{
		if (!isMatrixUsed(index) || !data)
			return false;
		return WriteSlotBytes(m_handleToSlot[index], data, 0, sizeof(Matrix4), EParamKind::Matrix);
	}

	bool CEffectFileRHI::setMatrixArray(eParameterHandles index, const Matrix4* data, uint32 count)
	{
		if (!isMatrixUsed(index) || !data || count == 0)
			return false;
		return WriteSlotBytes(m_handleToSlot[index], data, 0, sizeof(Matrix4) * count, EParamKind::Matrix);
	}

	bool CEffectFileRHI::setMatrixInArray(eParameterHandles index, uint32 element, const Matrix4* data)
	{
		if (!isMatrixUsed(index) || !data)
			return false;
		return WriteSlotBytes(m_handleToSlot[index], data, element * sizeof(Matrix4), sizeof(Matrix4), EParamKind::Matrix);
	}

	bool CEffectFileRHI::setFloatArray(eParameterHandles index, const float* data, uint32 count)
	{
		if (!isParameterUsed(index) || !data || count == 0)
			return false;
		return WriteSlotBytes(m_handleToSlot[index], data, 0, sizeof(float) * count, EParamKind::Float);
	}

	bool CEffectFileRHI::setVectorArray(eParameterHandles index, const Vector4* pVector, uint32 count)
	{
		if (!isParameterUsed(index) || !pVector || count == 0)
			return false;
		return WriteSlotBytes(m_handleToSlot[index], pVector, 0, sizeof(Vector4) * count, EParamKind::Float);
	}

	bool CEffectFileRHI::setFloatInArray(eParameterHandles index, uint32 element, float data)
	{
		if (!isParameterUsed(index))
			return false;
		return WriteSlotBytes(m_handleToSlot[index], &data, element * sizeof(float), sizeof(float), EParamKind::Float);
	}

	bool CEffectFileRHI::setTextureMatrix(int index, const Matrix4* data)
	{
		if (!isTextureMatrixUsed(index) || !data)
			return false;
		return setMatrix(static_cast<eParameterHandles>(k_tex_mat0 + index), data);
	}

	bool CEffectFileRHI::setTexture(int index, TextureEntity* data)
	{
		if (!g_bTextureEnabled || index < 0 || IsTextureLocked(index))
			return false;

		IRenderDevice* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return false;

		DeviceTexturePtr_type pTex = (data != nullptr) ? data->GetTexture() : nullptr;
		bool ok = setTexture(index, pTex);

#ifdef PARA_ENABLE_OPENGL
		if (ok && data && pTex && pDevice->GetBackendType() == ERHIBackend::OpenGL)
		{
			uint32_t dwValue = 0;
			pDevice->GetSamplerState(static_cast<uint32_t>(index), ESamplerStateType::MINFILTER, &dwValue);
			GLuint glTexId = static_cast<GLuint>(reinterpret_cast<uintptr_t>(pTex->GetNativeHandle()));
			if (dwValue == D3DTEXF_POINT && !data->IsSamplerStateBlocky())
			{
				glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(index));
				glBindTexture(GL_TEXTURE_2D, glTexId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				data->SetSamplerStateBlocky(true);
			}
			else if (dwValue == D3DTEXF_LINEAR && data->IsSamplerStateBlocky())
			{
				const PixelFormat texFmt = pTex->GetFormat();
				const bool is32BitFloat = (texFmt == PixelFormat::R32F || texFmt == PixelFormat::G32R32F || texFmt == PixelFormat::A32B32G32R32F);
				if (!is32BitFloat)
				{
					glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(index));
					glBindTexture(GL_TEXTURE_2D, glTexId);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pTex->GetMipLevels() > 1) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pTex->pointMagnification ? GL_NEAREST : GL_LINEAR);
					data->SetSamplerStateBlocky(false);
				}
			}
		}
#endif
		return ok;
	}

	bool CEffectFileRHI::setTexture(int index, DeviceTexturePtr_type pTex)
	{
		if (!g_bTextureEnabled || index < 0 || IsTextureLocked(index))
			return false;

		IRenderDevice* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return false;

		if (static_cast<int>(m_LastTextures.size()) <= index)
			m_LastTextures.resize(index + 1, nullptr);

		// Always forward to IRenderDevice::SetTexture because texture binding slots are global
		// device state shared across effects, direct pDevice->SetTexture calls, and RT-conflict
		// unbinds (each RHI backend already deduplicates redundant binds in O(1)).
		m_LastTextures[index] = pTex;
		return pDevice->SetTexture(static_cast<uint32_t>(index), pTex);
	}

	bool CEffectFileRHI::SetTextureByName(const char* name, TextureEntity* data)
	{
		if (!name || !name[0])
			return false;
		auto it = m_textureNameToStage.find(name);
		if (it == m_textureNameToStage.end())
			return false;

		int stage = it->second;
		if (stage < 0 || stage >= 16)
			return false;

		if (static_cast<int>(m_NamedStageTextures.size()) <= stage)
			m_NamedStageTextures.resize(stage + 1, nullptr);
		m_NamedStageTextures[stage] = data;

		if (m_bIsBeginPass && data && data->GetTexture() && CGlobals::GetRenderDevice())
		{
			CGlobals::GetRenderDevice()->SetTexture(static_cast<uint32_t>(stage), data->GetTexture());
		}
		return true;
	}

	void CEffectFileRHI::LockTexture(int nIndex)
	{
		if (nIndex < 0)
		{
			if (m_LockedTextures.size() < 9)
				m_LockedTextures.resize(9, true);
			std::fill(m_LockedTextures.begin(), m_LockedTextures.end(), true);
		}
		else
		{
			if (nIndex >= static_cast<int>(m_LockedTextures.size()))
				m_LockedTextures.resize(nIndex + 1, false);
			m_LockedTextures[nIndex] = true;
		}
	}

	void CEffectFileRHI::UnLockTexture(int nIndex)
	{
		if (nIndex < 0)
		{
			std::fill(m_LockedTextures.begin(), m_LockedTextures.end(), false);
		}
		else if (nIndex < static_cast<int>(m_LockedTextures.size()))
		{
			m_LockedTextures[nIndex] = false;
		}
	}

	bool CEffectFileRHI::IsTextureLocked(int nIndex) const
	{
		return nIndex >= 0 && nIndex < static_cast<int>(m_LockedTextures.size()) && m_LockedTextures[nIndex];
	}

	HRESULT CEffectFileRHI::SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 Bytes)
	{
		if (!hParameter || !pData || Bytes == 0)
			return E_FAIL;
		auto it = m_nameToSlot.find(hParameter);
		if (it == m_nameToSlot.end())
			return E_FAIL;
		ParamSlot& slot = m_paramSlots[it->second];
		// Scatter tightly-packed float/float2/float3 arrays into HLSL 16-byte cbuffer array stride
		if (slot.elementCount > 1 && slot.scalarElemBytes > 0 && slot.scalarElemBytes < 16 &&
			ByteOffset == 0 && Bytes <= slot.elementCount * slot.scalarElemBytes && (Bytes % slot.scalarElemBytes) == 0)
		{
			uint32_t count = Bytes / slot.scalarElemBytes;
			std::vector<uint8_t> strided(slot.elementCount * 16u, 0);
			if (!slot.data.empty())
			{
				uint32_t keep = (std::min)(static_cast<uint32_t>(strided.size()), static_cast<uint32_t>(slot.data.size()));
				std::memcpy(strided.data(), slot.data.data(), keep);
			}
			const uint8_t* src = static_cast<const uint8_t*>(pData);
			for (uint32_t i = 0; i < count; ++i)
				std::memcpy(strided.data() + i * 16u, src + i * slot.scalarElemBytes, slot.scalarElemBytes);
			return WriteSlotBytes(it->second, strided.data(), 0, static_cast<uint32_t>(strided.size()), slot.kind) ? S_OK : E_FAIL;
		}
		return WriteSlotBytes(it->second, pData, ByteOffset, Bytes, slot.kind) ? S_OK : E_FAIL;
	}

	bool CEffectFileRHI::SetBool(const char* hParameter, BOOL bBoolean)
	{
		int32_t v = bBoolean ? 1 : 0;
		return SUCCEEDED(SetRawValue(hParameter, &v, 0, sizeof(v)));
	}

	bool CEffectFileRHI::SetInt(const char* hParameter, int nValue)
	{
		if (!hParameter)
			return false;
		auto it = m_nameToSlot.find(hParameter);
		if (it != m_nameToSlot.end() && m_paramSlots[it->second].kind == EParamKind::Float)
		{
			float fv = static_cast<float>(nValue);
			return SUCCEEDED(SetRawValue(hParameter, &fv, 0, sizeof(fv)));
		}
		int32_t v = static_cast<int32_t>(nValue);
		return SUCCEEDED(SetRawValue(hParameter, &v, 0, sizeof(v)));
	}

	bool CEffectFileRHI::SetFloat(const char* hParameter, float fValue)
	{
		return SUCCEEDED(SetRawValue(hParameter, &fValue, 0, sizeof(fValue)));
	}

	bool CEffectFileRHI::SetVector2(const char* hParameter, const Vector2& vValue)
	{
		return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
	}

	bool CEffectFileRHI::SetVector3(const char* hParameter, const Vector3& vValue)
	{
		return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
	}

	bool CEffectFileRHI::SetVector4(const char* hParameter, const Vector4& vValue)
	{
		return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
	}

	bool CEffectFileRHI::SetMatrix(const char* hParameter, const Matrix4& data)
	{
		return SUCCEEDED(SetRawValue(hParameter, &data, 0, sizeof(data)));
	}

	void CEffectFileRHI::SetShadowMapSize(int nsize)
	{
		if (isParameterUsed(k_nShadowmapSize))
			setInt(k_nShadowmapSize, nsize);
	}

	void CEffectFileRHI::SetShadowRadius(float fRadius)
	{
		if (isParameterUsed(k_fShadowRadius))
			setFloat(k_fShadowRadius, fRadius);
	}

	void CEffectFileRHI::EnableShadowmap(int nShadowMethod)
	{
		SetBoolean(8, nShadowMethod > 0);
		SetBoolean(9, nShadowMethod == 1);
	}

	bool CEffectFileRHI::SetBoolean(int nIndex, bool value)
	{
		if (nIndex < 0 || nIndex >= (k_bBooleanMAX - k_bBoolean0))
			return false;
		auto h = static_cast<eParameterHandles>(k_bBoolean0 + nIndex);
		if (isParameterUsed(h))
			return setBool(h, value ? TRUE : FALSE);
		return false;
	}

	void CEffectFileRHI::EnableSunLight(bool bEnableSunLight)
	{
		if (isParameterUsed(k_bSunlightEnable))
			setBool(k_bSunlightEnable, bEnableSunLight ? TRUE : FALSE);
	}

	void CEffectFileRHI::EnableAlphaBlending(bool bAlphaBlending)
	{
		if (isParameterUsed(k_bAlphaBlending))
			setBool(k_bAlphaBlending, bAlphaBlending ? TRUE : FALSE);
	}

	void CEffectFileRHI::EnableAlphaTesting(bool bAlphaTesting)
	{
		if (isParameterUsed(k_bAlphaTesting))
		{
			setBool(k_bAlphaTesting, bAlphaTesting ? TRUE : FALSE);
			if (CGlobals::GetEffectManager() && !CGlobals::GetEffectManager()->IsD3DAlphaTestingDisabled() && CGlobals::GetRenderDevice())
			{
				CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ALPHATESTENABLE, bAlphaTesting ? 1u : 0u);
			}
		}
	}

	bool CEffectFileRHI::EnableEnvironmentMapping(bool bEnable)
	{
		return SetBoolean(6, bEnable);
	}

	bool CEffectFileRHI::EnableReflectionMapping(bool bEnable, float /*fSurfaceHeight*/)
	{
		if (CGlobals::GetEffectManager() && CGlobals::GetEffectManager()->IsReflectionRenderingEnabled())
			return SetBoolean(5, bEnable);
		return false;
	}

	void CEffectFileRHI::SetReflectFactor(float fFactor)
	{
		if (CGlobals::GetEffectManager() && CGlobals::GetEffectManager()->IsReflectionRenderingEnabled())
			setFloat(k_reflectFactor, fFactor);
	}

	void CEffectFileRHI::EnableNormalMap(bool /*bEnable*/)
	{
	}

	void CEffectFileRHI::EnableLightMap(bool /*bEnable*/)
	{
	}

	void CEffectFileRHI::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient)
	{
		if (!pSurfaceMaterial)
			return;

		const ParaMaterial& d3dMaterial = *pSurfaceMaterial;
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();

		if (isParameterUsed(k_ambientMaterialColor))
		{
			if (bUseGlobalAmbient && (d3dMaterial.Ambient.r < 0.01f) && pScene && pScene->GetSceneState())
				setParameter(k_ambientMaterialColor, &pScene->GetSceneState()->GetCurrentMaterial().Ambient, sizeof(LinearColor));
			else
				setParameter(k_ambientMaterialColor, &d3dMaterial.Ambient, sizeof(LinearColor));
		}

		if (isParameterUsed(k_diffuseMaterialColor))
		{
			if (pScene && pScene->GetSceneState() && pScene->GetSceneState()->HasLocalMaterial())
			{
				setParameter(k_diffuseMaterialColor, &pScene->GetSceneState()->GetCurrentMaterial().Diffuse, sizeof(LinearColor));
				const Vector3& ls = pScene->GetSceneState()->GetCurrentLightStrength();
				setParameter(k_LightStrength, &ls);
			}
			else
			{
				setParameter(k_diffuseMaterialColor, &d3dMaterial.Diffuse, sizeof(LinearColor));
				Vector3 defaultLightStrength(1.0f, 0.0f, 0.0f);
				setParameter(k_LightStrength, &defaultLightStrength);
			}
		}

		if (isParameterUsed(k_specularMaterialColor))
			setParameter(k_specularMaterialColor, &d3dMaterial.Specular, sizeof(LinearColor));

		if (isParameterUsed(k_emissiveMaterialColor))
		{
			// In ParaEngine .x/.fbx static meshes (CParaXMaterial), d3dMaterial.Emissive.r encodes
			// reflection-map reflectivity (often 1.0f), not an emissive RGB color (see ParaXMaterial.h
			// GetReflectivity() and legacy effect_file_DirectX.cpp / effect_file_OpenGL.cpp).
			// Animated ParaX emissive passes set k_emissiveMaterialColor explicitly in ModelRenderPass.
			LinearColor zeroEmissive(0.f, 0.f, 0.f, 0.f);
			setParameter(k_emissiveMaterialColor, &zeroEmissive, sizeof(LinearColor));
		}

		if (isParameterUsed(k_specularMaterialPower))
			setFloat(k_specularMaterialPower, d3dMaterial.Power);
	}

	void CEffectFileRHI::applyGlobalLightingData(CSunLight& sunlight)
	{
		if (isParameterUsed(k_atmosphericLighting) && sunlight.GetLightScatteringData())
		{
			setParameter(k_atmosphericLighting, sunlight.GetLightScatteringData()->getShaderData(), sizeof(sLightScatteringShaderParams));
		}

		if (isParameterUsed(k_sunColor))
		{
			LinearColor c = sunlight.GetSunColor();
			setParameter(k_sunColor, &c, sizeof(LinearColor));
		}

		if (isParameterUsed(k_sunVector))
		{
			Vector3 vDir = -sunlight.GetSunDirection();
			Vector4 v4(vDir.x, vDir.y, vDir.z, 1.0f);
			setParameter(k_sunVector, &v4);
		}

		if (isParameterUsed(k_ambientLight))
		{
			IScene* pScene = CGlobals::GetEffectManager()->GetScene();
			if (pScene && pScene->GetSceneState())
				setParameter(k_ambientLight, &pScene->GetSceneState()->GetCurrentMaterial().Ambient, sizeof(LinearColor));
		}

		if (isParameterUsed(k_shadowFactor))
		{
			float sf = sunlight.GetShadowFactor();
			Vector4 v4(sf, 1.0f - sf, 0.0f, 0.0f);
			setParameter(k_shadowFactor, &v4);
		}
	}

	void CEffectFileRHI::applyWorldMatrices()
	{
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		if (!pScene || !pScene->GetCurrentCamera())
			return;

		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());

		if (isMatrixUsed(k_worldMatrix))
			setMatrix(k_worldMatrix, pWorld);

		if (isMatrixUsed(k_worldInverseMatrix))
		{
			Matrix4 mWorldInverse = pWorld->inverse();
			setMatrix(k_worldInverseMatrix, &mWorldInverse);
		}

		if (isMatrixUsed(k_worldViewMatrix))
		{
			Matrix4 mWorldView;
			ParaMatrixMultiply(&mWorldView, pWorld, pView);
			setMatrix(k_worldViewMatrix, &mWorldView);
		}

		if (isMatrixUsed(k_worldViewProjMatrix))
		{
			Matrix4 ViewProj, mWorldViewProj;
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			setMatrix(k_worldViewProjMatrix, &mWorldViewProj);
		}
	}

	void CEffectFileRHI::applyCameraMatrices()
	{
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		if (!pScene)
			return;

		CBaseCamera* pCamera = pScene->GetCurrentCamera();
		if (!pCamera)
			return;

		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());
		Matrix4 ViewProj;

		if (isMatrixUsed(k_worldMatrix))
			setMatrix(k_worldMatrix, pWorld);

		if (isMatrixUsed(k_worldInverseMatrix))
		{
			Matrix4 mWorldInverse = pWorld->inverse();
			setMatrix(k_worldInverseMatrix, &mWorldInverse);
		}

		if (isMatrixUsed(k_worldViewMatrix))
		{
			Matrix4 mWorldView;
			ParaMatrixMultiply(&mWorldView, pWorld, pView);
			setMatrix(k_worldViewMatrix, &mWorldView);
		}

		if (isMatrixUsed(k_viewProjMatrix))
		{
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			setMatrix(k_viewProjMatrix, &ViewProj);
		}

		if (isMatrixUsed(k_worldViewProjMatrix))
		{
			if (!isMatrixUsed(k_viewProjMatrix))
				ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			setMatrix(k_worldViewProjMatrix, &mWorldViewProj);
		}

		if (isMatrixUsed(k_viewMatrix))
			setMatrix(k_viewMatrix, pView);

		if (isMatrixUsed(k_projMatrix))
			setMatrix(k_projMatrix, pProj);

		if (CGlobals::GetEffectManager()->IsUsingShadowMap() && isMatrixUsed(k_TexWorldViewProjMatrix))
		{
			Matrix4 mTex;
			ParaMatrixMultiply(&mTex, pWorld, CGlobals::GetEffectManager()->GetTexViewProjMatrix());
			setMatrix(k_TexWorldViewProjMatrix, &mTex);
		}

		if (isParameterUsed(k_cameraPos))
		{
			Vector3 vEye = pCamera->GetRenderEyePosition() - pScene->GetRenderOrigin();
			Vector4 vEye4(vEye.x, vEye.y, vEye.z, 1.0f);
			setParameter(k_cameraPos, &vEye4);
		}

		if (isParameterUsed(k_cameraFacing))
		{
			Vector3 vAhead = pCamera->GetWorldAhead();
			Vector4 vAhead4(vAhead.x, vAhead.y, vAhead.z, 0.0f);
			setParameter(k_cameraFacing, &vAhead4);
		}
	}

	void CEffectFileRHI::applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor)
	{
		if (isParameterUsed(k_fogEnable))
			setBool(k_fogEnable, bEnableFog ? TRUE : FALSE);

		if (isParameterUsed(k_fogParameters) && fogParam)
			setParameter(k_fogParameters, fogParam);

		if (isParameterUsed(k_fogColor) && fogColor)
			setParameter(k_fogColor, reinterpret_cast<const Vector4*>(fogColor));
	}

	void CEffectFileRHI::applyLocalLightingData(const LightList* plights, int nLightNum)
	{
		nLightNum = (std::min)(nLightNum, MAX_EFFECT_LIGHT_NUM);
		if (isParameterUsed(k_LocalLightNum))
		{
			setInt(k_LocalLightNum, nLightNum);
			if (plights != nullptr && nLightNum > 0)
			{
				const LightList& lights = *plights;
				Vector4 pos_ranges[MAX_EFFECT_LIGHT_NUM];
				Vector4 colors[MAX_EFFECT_LIGHT_NUM];
				Vector4 params[MAX_EFFECT_LIGHT_NUM];
				int count = (std::min)(nLightNum, static_cast<int>(lights.size()));
				for (int i = 0; i < count; ++i)
				{
					Vector3 pos = lights[i]->Position;
					pos_ranges[i] = Vector4(pos.x, pos.y, pos.z, lights[i]->Range);
					colors[i] = reinterpret_cast<const Vector4&>(lights[i]->Diffuse);
					params[i] = Vector4(lights[i]->Attenuation0, lights[i]->Attenuation1, lights[i]->Attenuation2, 1.0f);
				}
				if (count > 0)
				{
					setVectorArray(k_LightPositions, pos_ranges, static_cast<uint32>(count));
					setVectorArray(k_LightColors, colors, static_cast<uint32>(count));
					setVectorArray(k_LightParams, params, static_cast<uint32>(count));
				}
			}
		}
	}

	void CEffectFileRHI::applyLayersNum(int nLayers)
	{
		if (isParameterUsed(k_LayersNum))
			setInt(k_LayersNum, nLayers);
	}

	void CEffectFileRHI::applyTexWorldViewProj(const Matrix4* mat)
	{
		if (isMatrixUsed(k_TexWorldViewProjMatrix))
			setMatrix(k_TexWorldViewProjMatrix, mat);
	}

	CParameterBlock* CEffectFileRHI::GetParamBlock(bool /*bCreateIfNotExist*/)
	{
		return &m_SharedParamBlock;
	}
}

#endif // !NPLRUNTIME_SERVER
