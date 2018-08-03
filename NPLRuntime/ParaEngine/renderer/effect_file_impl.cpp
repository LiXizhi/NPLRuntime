//-----------------------------------------------------------------------------
// Class:	CEffectFileImpl
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.6.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "effect_file_impl.h"
#include "ShaderIncludeHandle.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#if USE_DIRECTX_RENDERER
#include "MirrorSurface.h"
#endif
#include "SkyMesh.h"
#include "AutoCamera.h"
#include "SunLight.h"
#include <boost/filesystem.hpp>
#include <unordered_map>

#ifdef WIN32
#define strcmpi		_strcmpi
#endif

using namespace ParaEngine;
using namespace IParaEngine;
using namespace std;










/**
* Comments by LiXizhi
* The DirectX control panel(in windows control panel) can toggle retail/debug version dll.
*	Only define DEBUG_VS| DEBUG_PS, if you are running with debug version dll. Otherwise the FX file will not be loaded on some machines.
* instructions to debug effect files:
* - Build using "DEBUG shaders" or define the following macros
* - Setting break points in fx files.
* - Run the program by Debug->Direct3D->Start with Direct3D debugging
* - Run the game to the place, where sharder is used. Set the device to REF
* - now that the debugger will break at the break points
*/

/** Uncomment this line to debug vertex shaders */
//#define DEBUG_VS
/** Uncomment this line to debug pixel shaders */
//#define DEBUG_PS

/**
* if true, the shader will use half precision arithmetics,
* if false, float4 will be used.
*/
bool CEffectFileImpl::s_bUseHalfPrecision = false;// half will be enough

#define LIGHT_BOOLEAN_BASE			0
													 /**@def max number of local lights.it can be up to 8 */
#define MAX_SHADER_LIGHT_NUM		4

bool CEffectFileImpl::g_bTextureEnabled = true;



CEffectFileImpl::CEffectFileImpl(const char* filename)
	:m_pEffect(0), m_bIsBegin(false), m_bSharedMode(false), m_nTechniqueIndex(0)
{
	m_filename = filename;
	memset(m_paramHandle, 0, sizeof(m_paramHandle));
	for (int i = 0; i<k_max_param_handles; i++)
	{
		m_paramHandle[i].idx = PARA_INVALID_HANDLE;
	}
}

CEffectFileImpl::CEffectFileImpl(const AssetKey& key)
	:CEffectFileBase(key), m_pEffect(0), m_bIsBegin(false), m_bSharedMode(false), m_nTechniqueIndex(0)
{
	memset(m_paramHandle, 0, sizeof(m_paramHandle));
	for (int i = 0; i < k_max_param_handles; i++)
	{
		m_paramHandle[i].idx = PARA_INVALID_HANDLE;
	}
}

CEffectFileImpl::~CEffectFileImpl()
{
	m_pEffect = nullptr;
}

void CEffectFileImpl::EnableShareMode(bool bEnable)
{
	m_bSharedMode = bEnable;
}
bool CEffectFileImpl::IsInShareMode()
{
	return m_bSharedMode;
}

void CEffectFileImpl::EnableTextures(bool bEnable)
{
	g_bTextureEnabled = bEnable;
}
bool CEffectFileImpl::AreTextureEnabled()
{
	return g_bTextureEnabled;
}

bool CEffectFileImpl::EnableEnvironmentMapping(bool bEnable)
{
	return SetBoolean(6, bEnable);
}

void CEffectFileImpl::SetReflectFactor(float fFactor)
{
	if (CGlobals::GetEffectManager()->IsReflectionRenderingEnabled())
		setFloat(k_reflectFactor, fFactor);
}

bool CEffectFileImpl::EnableReflectionMapping(bool bEnable, float fSurfaceHeight)
{


#if USE_DIRECTX_RENDERER
	if (CGlobals::GetEffectManager()->IsReflectionRenderingEnabled() && SetBoolean(5, bEnable))
	{
		if (bEnable)
		{
			CMirrorSurface* pMirorSurface = CGlobals::GetScene()->GetMirrorSurface(0);
			if (pMirorSurface != 0)
			{
				// TODO: here I just assume the reflection surface is centered on the world origin of the current rendered object. 
				// TODO: if there are multiple surfaces, why not choose the one closest and above the camera eye? Currently it just choose the one rendered last

				DVector3 vPos = CGlobals::GetSceneState()->GetCurrentSceneObject()->GetPosition();
				vPos.y += fSurfaceHeight;
				pMirorSurface->SetPosition(vPos);
				// TODO: here I just assume texture sampler is on s1 register.
				setTexture(1, pMirorSurface->GetReflectionTexture());
			}
		}
		/*else
		setTexture(1, (TextureEntity*)NULL);*/
		return true;
	}
#endif


	return false;
}

void CEffectFileImpl::EnableNormalMap(bool bEnable)
{
	// TODO:
}
void CEffectFileImpl::EnableLightMap(bool bEnable)
{
	// TODO:
}


bool CEffectFileImpl::isParameterUsed(eParameterHandles index)const
{
	return isValidHandle(m_paramHandle[index]);
}

bool CEffectFileImpl::isMatrixUsed(eParameterHandles index)const
{
	return isValidHandle(m_paramHandle[index]);
}

bool CEffectFileImpl::isTextureUsed(int index)const
{
	return isValidHandle(m_paramHandle[k_tex0 + index]);
}

bool CEffectFileImpl::isTextureMatrixUsed(int index)const
{
	return isValidHandle(m_paramHandle[k_tex_mat0 + index]);
}




bool CEffectFileImpl::setMatrixArray(eParameterHandles index, const Matrix4* data, uint32_t count)
{
	if (m_pEffect && isMatrixUsed(index))
	{
		return m_pEffect->SetMatrixArray(m_paramHandle[index], data->GetConstPointer(), count);
	}
	return false;
}


bool CEffectFileImpl::setVectorArray(eParameterHandles index, const Vector4* pVector, uint32_t count) 
{
	if (m_pEffect && isParameterUsed(index))
	{
		return m_pEffect->SetVectorArray(m_paramHandle[index], (DeviceVector4*)pVector, count);
	}
	return false;
}

bool CEffectFileImpl::setFloatArray(eParameterHandles index, const float* data, uint32_t count)
{
	if (m_pEffect && isParameterUsed(index))
	{
		return m_pEffect->SetFloatArray(m_paramHandle[index], data, count);
	}
	return false;
}



bool CEffectFileImpl::setParameter(eParameterHandles index, const void* data, uint32_t size)
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result = m_pEffect->SetValue(m_paramHandle[index], data, size);

		PE_ASSERT(result);
		return result;
	}
	return false;
}
bool CEffectFileImpl::setBool(eParameterHandles index, bool bBoolean) 
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result = m_pEffect->SetBool(m_paramHandle[index], bBoolean);
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileImpl::setInt(eParameterHandles index, int nValue) 
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result = m_pEffect->SetInt(m_paramHandle[index], nValue);
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileImpl::setFloat(eParameterHandles index, float fValue) 
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result = m_pEffect->SetFloat(m_paramHandle[index], fValue);
		PE_ASSERT(result);
		return result;
	}
	return false;
}


bool CEffectFileImpl::setMatrix(eParameterHandles index, const Matrix4* data)
{
	if (m_pEffect && isMatrixUsed(index))
	{
		return m_pEffect->SetMatrix(m_paramHandle[index], data->GetConstPointer());
	}
	return false;
}

/** load the resource from a file or win32 resource
If the resource file name begins with ':', it is treated as a win32 resource.
e.g.":IDR_FX_OCEANWATER". loads data from a resource of type "TEXTFILE". See MSDN for more information about Windows resources.
*/
HRESULT CEffectFileImpl::InitDeviceObjects()
{
	m_bIsInitialized = true;
	m_bIsValid = false;//set to true if created successfully.
	auto pRenderDevice = CGlobals::GetRenderDevice();

	
	auto file = std::make_shared<CParaFile>();

#if USE_OPENGL_RENDERER
	if (m_filename.find(".fxo") != std::string::npos)
	{
		std::string fxname = m_filename.substr(0, m_filename.size() - 1);
		file = std::make_shared<CParaFile>(fxname.c_str());
	}else {
		file = make_shared<CParaFile>(m_filename.c_str());
	}

#else
	if (file->isEof())
	{
		file = nullptr;
		if (m_filename.find(".fxo") != std::string::npos)
		{
			std::string fxname = m_filename.substr(0, m_filename.size() - 1);
			file = std::make_shared<CParaFile>(fxname.c_str());
		}
	}
#endif

	// fxo 


	std::string error = "";

	if (!file->isEof())
	{
		boost::filesystem::path p(m_filename);
		auto shaderDir = p.parent_path();
		ShaderIncludeHandle includeImpl(shaderDir.string());
		m_pEffect = CGlobals::GetRenderDevice()->CreateEffect(file->getBuffer(), file->getSize(), &includeImpl, error);
	}
	else
	{
		OUTPUT_LOG("ERROR: shader file %s not found\n", m_filename.c_str());
		return E_FAIL;
	}

	if (!m_pEffect)
	{
		char* error_str = (error != "") ? error.c_str() : "failed loading effect file\n";
		OUTPUT_LOG("Failed Loading Effect file %s: error is %s\n", m_filename.c_str(), error_str);
		return E_FAIL;
	}


	// get the description
	m_pEffect->GetDesc(&m_EffectDesc);

	m_techniques.clear();
	//////////////////////////////////////////////////////////////////////////
	// get all valid techniques
	TechniqueDesc tech;
	bool bHasDefaultTechnique = false;

	for (int idx = 0; idx<m_EffectDesc.Techniques; idx++)
	{
		tech.hTechnique = m_pEffect->GetTechnique(idx);
		if (m_pEffect->GetTechniqueDesc(tech.hTechnique, &tech.techniqueDesc))
		{
			if (tech.techniqueDesc.Name == "GenShadowMap")
			{
				tech.nCategory = TechCategory_GenShadowMap;
			}
			else if (tech.techniqueDesc.Name == "Vegetation")
			{
				tech.nCategory = TechCategory_Vegetation;
			}
			else if (tech.techniqueDesc.Name == "Character")
			{
				tech.nCategory = TechCategory_Character;
			}
			else if (tech.techniqueDesc.Name == "DetailCharacter")
			{
				tech.nCategory = TechCategory_DetailCharacter;
			}
			else
			{
				tech.nCategory = TechCategory_default;
				bHasDefaultTechnique = true;
			}
			m_techniques.push_back(tech);
		}
		else {
			OUTPUT_LOG("ERROR: effect file: %s failed getting its description.\n", m_filename.c_str());
		}
	}

	HRESULT result = E_FAIL;

	if (!bHasDefaultTechnique)
	{
		// at least one default technique must be valid. "GenShadowMap" is not a default technique. 
		// So if "GenShadowMap" is valid, but others are not valid, the technique is not considered valid.
		OUTPUT_LOG("ERROR: effect file: %s not supported by your GPU.\n", m_filename.c_str());
		releaseEffect();
		result = E_FAIL;
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// activate the first valid technique
		m_nTechniqueIndex = 0;
		if (m_pEffect->SetTechnique(m_techniques[m_nTechniqueIndex].hTechnique))
		{
			// parse the effect parameters to build a list of handles
			parseParameters();
			m_bIsValid = true;
		}
		else
		{
			OUTPUT_LOG("ERROR: effect file: %s failed loading its technique.\n", m_filename.c_str());
			releaseEffect();
		}
		result = S_OK;
	}

	return result;
}

bool CEffectFileImpl::SetTechniqueByIndex(int nIndex)
{
	if (m_nTechniqueIndex == nIndex)
		return true;
	else if (((int)m_techniques.size()>nIndex) && (m_pEffect->SetTechnique(m_techniques[nIndex].hTechnique)))
	{
		m_nTechniqueIndex = nIndex;
		return true;
	}
	else
		return false;
}

bool CEffectFileImpl::SetFirstValidTechniqueByCategory(TechniqueCategory nCat)
{
	if (m_nTechniqueIndex >= (int)m_techniques.size())
		return false;
	if (m_techniques[m_nTechniqueIndex].nCategory == nCat)
		return true;
	vector<TechniqueDesc>::const_iterator itCur, itEnd = m_techniques.end();
	int i = 0;
	for (itCur = m_techniques.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if ((*itCur).nCategory == nCat)
		{
			if (m_pEffect->SetTechnique(m_techniques[i].hTechnique))
			{
				m_nTechniqueIndex = i;
				return true;
			}
			else
				return false;
		}
	}
	return false;
}
void CEffectFileImpl::releaseEffect()
{
	m_pEffect = nullptr;
}

// destroy the resource
HRESULT CEffectFileImpl::DeleteDeviceObjects()
{
	releaseEffect();
	m_bIsInitialized = false;
	return S_OK;
}

// purge the resource from volatile memory
HRESULT CEffectFileImpl::InvalidateDeviceObjects()
{
	if (m_pEffect)
	{
		m_pEffect->OnLostDevice();
	}
	return S_OK;
}

// prepare the resource for use (create any volatile memory objects needed)
HRESULT CEffectFileImpl::RestoreDeviceObjects()
{
	if (m_pEffect)
	{
		m_pEffect->OnResetDevice();
	}
	return S_OK;
}

// save the resource to the file and return the size written
bool CEffectFileImpl::saveResource(const char* filename)
{
	return 0;
}
int CEffectFileImpl::totalPasses()const
{
	if (m_pEffect == 0)
	{
		OUTPUT_LOG("error");
	}
	//PE_ASSERT(m_pEffect);
	if (m_nTechniqueIndex<(int)m_techniques.size())
		return m_techniques[m_nTechniqueIndex].techniqueDesc.Passes;
	else
		return 0;
}

bool CEffectFileImpl::setTexture(int index, TextureEntity* data)
{
	if (g_bTextureEnabled && m_pEffect && isTextureUsed(index) && !IsTextureLocked(index))
	{
		return setTextureInternal(index, (data != 0) ? data->GetTexture() : NULL);
	}
	return false;
}
bool CEffectFileImpl::setTexture(int index, IParaEngine::ITexture* pTex)
{
	if (g_bTextureEnabled && m_pEffect && isTextureUsed(index) && !IsTextureLocked(index))
	{
		return setTextureInternal(index, pTex);
	}
	return false;
}

bool CEffectFileImpl::setTextureInternal(int index, ITexture* pTex)
{
	if ((int)(m_LastTextures.size()) <= index)
	{
		m_LastTextures.resize(index + 1, NULL);
	}
	if (m_LastTextures[index] != pTex)
	{
		m_LastTextures[index] = pTex;
		// TODO: implement statemanagerstate to set state, sampler and texture. This may be wrong if shader textures in hlsl are not written in the exact order. 
		// so that they map to s0,s15 in the written order in the compiled shader code. 
		//return SUCCEEDED(m_pEffect->SetTexture(m_paramHandle[k_tex0+index], (data!=0) ? data->GetTexture(): NULL));
		return CGlobals::GetRenderDevice()->SetTexture(index, pTex);
	}
	return true;
}

bool CEffectFileImpl::begin(bool bApplyParam,bool bForceBegin)
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();
	if (m_pEffect != NULL)
	{
		if (bApplyParam)
		{
			// set the lighting parameters
			// from the global light manager
			applyGlobalLightingData(pScene->GetSunLight());

			// set the camera matrix
			applyCameraMatrices();
		}

		if (bForceBegin || !m_bSharedMode)
		{
			bool result = m_pEffect->Begin();
			if (result)
			{
				m_bIsBegin = true;
				return true;
			}
			else
			{
				OUTPUT_LOG("error: CEffectFileImpl::begin failed: %s \n", m_filename.c_str());
				return false;
			}
		}
		return true;
	}
	else
		return false;
}

bool CEffectFileImpl::BeginPass(int pass, bool bForceBegin)
{
	if (bForceBegin || !m_bSharedMode)
	{
		bool result = m_pEffect->BeginPass(pass);
		if (!result)
		{
			OUTPUT_LOG("error: CEffectFileImpl::BeginPass failed: %s \n", m_filename.c_str());
			return false;
		}
	}
	return true;
}

void CEffectFileImpl::CommitChanges()
{
	m_pEffect->CommitChanges();
}

void CEffectFileImpl::EndPass(bool bForceEnd)
{
	if (bForceEnd || !m_bSharedMode)
	{
		if (m_bIsBegin)
		{
			if (!(m_pEffect && m_pEffect->EndPass()))
			{
				OUTPUT_LOG("error: CEffectFileImpl::EndPass failed: %s \n", m_filename.c_str());
			}
		}
	}
}

void CEffectFileImpl::end(bool bForceEnd)
{
	if (bForceEnd || !m_bSharedMode)
	{
		if (m_bIsBegin)
		{
			// tricky: since d3dxeffect->BeginPass(0) will reset states such as Texture to NULL, 
			// we need to call OnSwitchOutShader() when pass is end. 
			if (!m_bSharedMode)
			{
				OnSwitchOutShader();
			}
			if (!(m_pEffect && m_pEffect->End()))
			{
				OUTPUT_LOG("error: CEffectFileImpl::end failed: %s \n", m_filename.c_str());
				return;
			}
			m_bIsBegin = false;
		}
	}
}

int CEffectFileImpl::BeginWith(LPCSTR str, LPCSTR searchStr)
{
	int i = 0;
	while (str[i] != 0)
	{
		if (searchStr[i] == 0)
			return i;
		else if (str[i] != searchStr[i])
			return -1;
		++i;
	}
	return -1;
}
bool CEffectFileImpl::GetNumber(LPCSTR str, int nBeginIndex, int* pOut)
{
	int i = nBeginIndex;
	while (str[i] >= '0' && str[i] <= '9')
		++i;
	if (i>nBeginIndex)
	{
		int value = 0;
		int nCount = i - nBeginIndex;
		for (i = 0; i<nCount; ++i)
		{
			value += (int)((str[nBeginIndex + i] - '0'))*(int)pow((float)10, (nCount - i - 1));
		}
		*pOut = value;
		return true;
	}
	return false;
}

void CEffectFileImpl::parseParameters()
{
	using namespace IParaEngine;

	static std::unordered_map<std::string, uint32> table;
	if (table.empty())
	{
		table["world"] = k_worldMatrix;
		table["worldinverse"] = k_worldInverseMatrix;
		table["worldview"] = k_worldViewMatrix;
		table["worldviewprojection"] = k_worldViewProjMatrix;
		table["worldmatrixarray"] = k_worldMatrixArray;
		table["skyboxmatrix"] = k_skyBoxMatrix;
		table["view"] = k_viewMatrix;
		table["projection"] = k_projMatrix;
		table["viewprojection"] = k_viewProjMatrix;
		table["texworldviewproj"] = k_TexWorldViewProjMatrix;


		table["materialambient"] = k_ambientMaterialColor;
		table["materialdiffuse"] = k_diffuseMaterialColor;
		table["materialspecular"] = k_specularMaterialColor;
		table["materialemissive"] = k_emissiveMaterialColor;
		table["posscaleoffset"] = k_posScaleOffset;
		table["uvscaleoffset"] = k_uvScaleOffset;
		table["flarecolor"] = k_lensFlareColor;
		table["fogparameters"] = k_fogParameters;
		table["fogcolor"] = k_fogColor;
		table["lightstrength"] = k_LightStrength;
		table["shadowfactor"] = k_shadowFactor;
		table["lightstrength"] = k_LightStrength;
		table["lightcolors"] = k_LightColors;
		table["lightpositions"] = k_LightPositions;
		table["fresnelr0"] = k_fresnelR0;
		table["constvector0"] = k_ConstVector0;
		table["constvector1"] = k_ConstVector1;
		table["constvector2"] = k_ConstVector2;
		table["constvector3"] = k_ConstVector3;
		table["sunvector"] = k_sunVector;
		table["suncolor"] = k_sunColor;
		table["worldcamerapos"] = k_cameraPos;
		table["viewdistances"] = k_cameraDistances;
		table["worldviewvector"] = k_cameraFacing;
		table["ambientlight"] = k_ambientLight;
		table["sunlight_inscatter"] = k_sunlightInscatter;
		table["sunlight_extinction"] = k_sunlightExtinction;
		table["worldpos"] = k_worldPos;
		table["texcoordoffset"] = k_texCoordOffset;
		table["curnumbones"] = k_boneInfluenceCount;

		table["fogenable"] = k_fogEnable;
		table["alphatesting"] = k_bAlphaTesting;
		table["alphablending"] = k_bAlphaBlending;

		// boolean
		for (int i = 0; i < (k_bBooleanMAX - k_bBoolean0); i++)
		{
			char buf[64]{ 0 };
			sprintf(buf, "boolean%d", i);
			table[buf] = k_bBoolean0 + i;
		}

		table["sunlightenable"] = k_bSunlightEnable;
		table["shadowmapsize"] = k_nShadowmapSize;
		table["shadowradius"] = k_fShadowRadius;
		table["materialpower"] = k_specularMaterialPower;
		table["reflectfactor"] = k_reflectFactor;
		table["locallightnum"] = k_LocalLightNum;
		table["layersnum"] = k_LayersNum;
		table["time"] = k_time;
		table["opacity"] = k_opacity;
		table["specularPower"] = k_specularPower;
		table["transitionFactor"] = k_transitionFactor;
		table["lightparams"] = k_LightParams;
		table["atmosphericlightingparams"] = k_atmosphericLighting;
		table["patchcorners"] = k_patchCorners;
	}


	static char numerals[] = { '0','1','2','3','4','5','6','7','8','9' };
	IParaEngine::ParameterDesc ParamDesc;
	for (uint32_t index = 0; index<m_EffectDesc.Parameters; index++)
	{
		IParaEngine::ParameterHandle hParam = m_pEffect->GetParameter(index);
		if (!IParaEngine::isValidHandle(hParam))continue;
		if (!m_pEffect->GetParameterDesc(hParam, &ParamDesc)) continue;

		std::string lower_sem = ParamDesc.Semantic;
		std::transform(lower_sem.begin(), lower_sem.end(), lower_sem.begin(),
			[](unsigned char c) { return std::tolower(c); }
		);

		auto it = table.find(lower_sem);
		if (it != table.end())
		{
			m_paramHandle[it->second] = hParam;
		}
		else
		{
			if (ParamDesc.Type == EParameterType::PT_TEXTURE || ParamDesc.Type == EParameterType::PT_TEXTURE2D || ParamDesc.Type == EParameterType::PT_TEXTURE3D || ParamDesc.Type == PT_TEXTURECUBE) {
				int iPos = (int)ParamDesc.Name.find_first_of(numerals, 0, sizeof(numerals));

				if (iPos != string::npos)
				{
					int iTexture = atoi(&ParamDesc.Name[iPos]);
					if (iTexture >= 0 && iTexture < (k_tex_max - k_tex0))
					{
						m_paramHandle[k_tex0 + iTexture] = hParam;
					}
				}
			}
			else {
				OUTPUT_LOG("Warning: unsupported paramter::%s :%s  at %s \n", ParamDesc.Name.c_str(), ParamDesc.Semantic.c_str(), m_filename.c_str());
			}
		}
	}
}

void CEffectFileImpl::EnableSunLight(bool bEnableSunLight)
{
	if (isParameterUsed(k_bSunlightEnable))
	{
		setBool(k_bSunlightEnable, bEnableSunLight);
	}
}

void CEffectFileImpl::EnableAlphaBlending(bool bAlphaBlending)
{
	if (isParameterUsed(k_bAlphaBlending))
	{
		setBool(k_bAlphaBlending, bAlphaBlending);
	}
}

void CEffectFileImpl::EnableAlphaTesting(bool bAlphaTesting)
{
	if (isParameterUsed(k_bAlphaTesting))
	{
		setBool(k_bAlphaTesting, bAlphaTesting);
		if (!CGlobals::GetEffectManager()->IsD3DAlphaTestingDisabled())
			CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ALPHATESTENABLE, bAlphaTesting);
	}
}

void CEffectFileImpl::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient)
{
	if (pSurfaceMaterial)
	{
		// set material properties
		const ParaMaterial & d3dMaterial = *pSurfaceMaterial;

		if (isParameterUsed(k_ambientMaterialColor))
		{
			if (bUseGlobalAmbient && (d3dMaterial.Ambient.r < 0.01f))
				setParameter(k_ambientMaterialColor, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Ambient,sizeof(LinearColor));
			else
				setParameter(k_ambientMaterialColor, &d3dMaterial.Ambient,sizeof(LinearColor));
		}

		if (isParameterUsed(k_diffuseMaterialColor))
		{
			if (CGlobals::GetEffectManager()->GetScene()->GetSceneState()->HasLocalMaterial())
			{
				setParameter(k_diffuseMaterialColor, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Diffuse,sizeof(LinearColor));
				setParameter(k_LightStrength, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentLightStrength(),sizeof(Vector3));
			}
			else
			{
				setParameter(k_diffuseMaterialColor, &d3dMaterial.Diffuse,sizeof(LinearColor));
				Vector3 vEmpty(0, 0, 0);
				setParameter(k_LightStrength, &vEmpty,sizeof(Vector3));
			}
		}

		if (isParameterUsed(k_specularMaterialColor))
		{
			setParameter(k_specularMaterialColor, &d3dMaterial.Specular,sizeof(LinearColor));
		}

		if (isParameterUsed(k_emissiveMaterialColor))
		{
			setParameter(k_specularMaterialColor, &d3dMaterial.Emissive,sizeof(LinearColor));
		}

		if (isParameterUsed(k_specularMaterialPower))
		{
			setParameter(k_specularMaterialPower, &d3dMaterial.Power,sizeof(float));
		}
	}
}


void CEffectFileImpl::applyGlobalLightingData(CSunLight& sunlight)
{
	// pass the lighting structure to the shader
	if (isParameterUsed(k_atmosphericLighting))
	{
		setParameter(
			k_atmosphericLighting,
			sunlight.GetLightScatteringData()->getShaderData(),sizeof(sLightScatteringShaderParams));
	}

	// pass the lighting structure to the shader
	if (isParameterUsed(k_sunColor))
	{
		setParameter(
			k_sunColor,
			&(sunlight.GetSunColor()),sizeof(LinearColor));
	}

	if (isParameterUsed(k_sunVector))
	{
		Vector3 vDir = -sunlight.GetSunDirection();
		setParameter(
			k_sunVector,
			&Vector4(vDir.x, vDir.y, vDir.z, 1.0f),sizeof(Vector4));
	}

	if (isParameterUsed(k_ambientLight))
	{
		setParameter(
			k_ambientLight,
			&CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Ambient,sizeof(LinearColor));
	}


	if (isParameterUsed(k_shadowFactor))
	{
		float shadowFactor = sunlight.GetShadowFactor();
		setParameter(
			k_shadowFactor,
			&Vector4(shadowFactor, 1 - shadowFactor, 0, 0),
			sizeof(Vector4)
		);
	}
}

void CEffectFileImpl::applyWorldMatrices()
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();

	CBaseCamera* pCamera = pScene->GetCurrentCamera();
	if (pCamera)
	{
		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());
		Matrix4 ViewProj;
		// set the world matrix
		if (isMatrixUsed(k_worldMatrix))
		{
			setMatrix(k_worldMatrix, pWorld);
		}

		// set the world inverse matrix
		if (isMatrixUsed(k_worldInverseMatrix))
		{
			Matrix4 mWorldInverse;
			mWorldInverse = pWorld->inverse();
			setMatrix(k_worldInverseMatrix, &mWorldInverse);
		}
		// set the world view matrix
		if (isMatrixUsed(k_worldViewMatrix))
		{
			Matrix4 mWorldView;
			ParaMatrixMultiply(&mWorldView, pWorld, pView);
			setMatrix(k_worldViewMatrix, &mWorldView);
		}
		// set the world view projection matrix
		if (isMatrixUsed(k_worldViewProjMatrix))
		{
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			setMatrix(k_worldViewProjMatrix, &mWorldViewProj);
		}
	}
}

void CEffectFileImpl::applyCameraMatrices()
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();

	CBaseCamera* pCamera = pScene->GetCurrentCamera();
	if (pCamera)
	{
		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());
		Matrix4 ViewProj;
		// set the world matrix
		if (isMatrixUsed(k_worldMatrix))
		{
			setMatrix(k_worldMatrix, pWorld);
		}

		// set the world inverse matrix
		if (isMatrixUsed(k_worldInverseMatrix))
		{
			Matrix4 mWorldInverse;
			mWorldInverse = pWorld->inverse();
			setMatrix(k_worldInverseMatrix, &mWorldInverse);
		}
		// set the world view matrix
		if (isMatrixUsed(k_worldViewMatrix))
		{
			Matrix4 mWorldView;
			ParaMatrixMultiply(&mWorldView, pWorld, pView);
			setMatrix(k_worldViewMatrix, &mWorldView);
		}

		// set the combined matrix
		if (isMatrixUsed(k_viewProjMatrix))
		{
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			setMatrix(k_viewProjMatrix, &ViewProj);
		}

		// set the world view projection matrix
		if (isMatrixUsed(k_worldViewProjMatrix))
		{
			if (!isMatrixUsed(k_viewProjMatrix))
				ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			setMatrix(k_worldViewProjMatrix, &mWorldViewProj);
		}

		// set the view matrix
		if (isMatrixUsed(k_viewMatrix))
		{
			setMatrix(k_viewMatrix, pView);
		}

		// set the projection matrix
		if (isMatrixUsed(k_projMatrix))
		{
			setMatrix(k_projMatrix, pProj);
		}

		// set the tex world view projection matrix
		if (CGlobals::GetEffectManager()->IsUsingShadowMap() && isMatrixUsed(k_TexWorldViewProjMatrix))
		{
			Matrix4 mTex;
			ParaMatrixMultiply(&mTex, pWorld, CGlobals::GetEffectManager()->GetTexViewProjMatrix());
			setMatrix(k_TexWorldViewProjMatrix, &mTex);
		}

		// set the world camera position
		if (isParameterUsed(k_cameraPos))
		{
			Vector3 vEye = pCamera->GetRenderEyePosition() - pScene->GetRenderOrigin();
			setParameter(k_cameraPos, &Vector4(vEye.x, vEye.y, vEye.z, 1.0f),sizeof(Vector4));
		}
		// set the world camera facing vector
		if (isParameterUsed(k_cameraFacing))
		{
			setParameter(k_cameraFacing, &pCamera->GetWorldAhead(),sizeof(Vector3));
		}

		//// set the matrix used by sky boxes
		//if (isMatrixUsed(k_skyBoxMatrix))
		//{
		//	setMatrix(k_skyBoxMatrix, &pCamera->skyBoxMatrix());
		//}

		/*cVector4 camDistances(
		pCamera->nearPlane(),
		pCamera->farPlane(),
		pCamera->viewDistance(),
		pCamera->invFarPlane());

		if (isParameterUsed(k_cameraDistances))
		{
		setParameter(k_cameraDistances, &camDistances);
		}*/
	}
}

void CEffectFileImpl::applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor)
{
	if (isParameterUsed(k_fogEnable))
	{
		setBool(k_fogEnable, bEnableFog);
	}
	if (bEnableFog)
	{
		if (isParameterUsed(k_fogParameters) && (fogParam != 0))
		{
			setParameter(k_fogParameters, fogParam,sizeof(Vector4));
		}

		if (isParameterUsed(k_fogColor) && (fogColor != 0))
		{
			setParameter(k_fogColor, fogColor,sizeof(LinearColor));
		}
	}
}

void CEffectFileImpl::applyLocalLightingData(const LightList* plights, int nLightNum)
{
	nLightNum = min(nLightNum, MAX_EFFECT_LIGHT_NUM);

	if (isParameterUsed(k_LocalLightNum))
	{
		setInt(k_LocalLightNum, nLightNum);

		//////////////////////////////////////////////////////////////////////////
		// set local lights
		if (plights != 0 && nLightNum>0)
		{
			const LightList& lights = *plights;
			PE_ASSERT(nLightNum <= (int)lights.size());

			Vector4 pos_ranges[MAX_EFFECT_LIGHT_NUM];
			Vector4 colors[MAX_EFFECT_LIGHT_NUM];
			Vector4 params[MAX_EFFECT_LIGHT_NUM];
			for (int i = 0; i<nLightNum; ++i)
			{
				Vector3 pos = lights[i]->Position;
				pos_ranges[i].x = pos.x;
				pos_ranges[i].y = pos.y;
				pos_ranges[i].z = pos.z;
				pos_ranges[i].w = lights[i]->Range;
				colors[i] = reinterpret_cast<const Vector4&> (lights[i]->Diffuse);
				params[i].x = lights[i]->Attenuation0;
				params[i].y = lights[i]->Attenuation1;
				params[i].z = lights[i]->Attenuation2;
				params[i].w = 1;
			}
			setVectorArray((eParameterHandles)(k_LightPositions), pos_ranges, nLightNum);
			setVectorArray((eParameterHandles)(k_LightColors), colors, nLightNum);
			setVectorArray((eParameterHandles)(k_LightParams), params, nLightNum);
		}
	}
	/*for (int i =0;i<MAX_SHADER_LIGHT_NUM;++i)
	{
	SetBoolean(LIGHT_BOOLEAN_BASE+i, (i<nLightNum));
	}*/
}

void CEffectFileImpl::applyLayersNum(int nLayers)
{
	if (isParameterUsed(k_LayersNum))
	{
		setInt(k_LayersNum, nLayers);
	}
}

void CEffectFileImpl::applyTexWorldViewProj(const Matrix4* mat)
{
	if (isMatrixUsed(k_TexWorldViewProjMatrix))
	{
		setMatrix(k_TexWorldViewProjMatrix, mat);
	}
}

void CEffectFileImpl::SetShadowMapSize(int nsize)
{
	if (isParameterUsed(k_nShadowmapSize))
	{
		setInt(k_nShadowmapSize, nsize);
	}
}

void CEffectFileImpl::SetShadowRadius(float fRadius)
{
	if (isParameterUsed(k_fShadowRadius))
	{
		setFloat(k_fShadowRadius, fRadius);
	}
}

void CEffectFileImpl::EnableShadowmap(int nShadowMethod)
{
#define SHADOW_BOOLEAN_BASE			8
	SetBoolean(SHADOW_BOOLEAN_BASE, nShadowMethod > 0);
	SetBoolean(SHADOW_BOOLEAN_BASE + 1, nShadowMethod == 1);
}
bool CEffectFileImpl::SetBoolean(int nIndex, bool value)
{
	PE_ASSERT(nIndex< (k_bBooleanMAX - k_bBoolean0));
	if (isParameterUsed((eParameterHandles)(k_bBoolean0 + nIndex)))
	{
		return setBool((eParameterHandles)(k_bBoolean0 + nIndex), value);
	}
	return false;
}
int CEffectFileImpl::GetCurrentTechniqueIndex()
{
	return m_nTechniqueIndex;
}
const CEffectFileImpl::TechniqueDesc* CEffectFileImpl::GetCurrentTechniqueDesc()
{
	const static TechniqueDesc g_techdesc;
	if (m_nTechniqueIndex<(int)m_techniques.size())
		return &(m_techniques[m_nTechniqueIndex]);
	else
		return &g_techdesc;
}

std::shared_ptr<IParaEngine::IEffect> ParaEngine::CEffectFileImpl::GetDeviceEffect()
{
	return m_pEffect;
}

CParameterBlock* ParaEngine::CEffectFileImpl::GetParamBlock(bool bCreateIfNotExist /*= false*/)
{
	return &m_SharedParamBlock;
}

void ParaEngine::CEffectFileImpl::LockTexture(int nIndex)
{
	if (nIndex <0)
	{
		// lock all 9 textures 
		m_LockedTextures.resize(9);
		int nSize = (int)m_LockedTextures.size();
		for (int i = 0; i<nSize; ++i)
		{
			m_LockedTextures[i] = true;
		}
	}
	else
	{
		if (nIndex >= (int)(m_LockedTextures.size()))
		{
			m_LockedTextures.resize(nIndex + 1, false);
		}
		m_LockedTextures[nIndex] = true;
	}
}

void ParaEngine::CEffectFileImpl::UnLockTexture(int nIndex)
{
	if (nIndex <0)
	{
		int nSize = (int)m_LockedTextures.size();
		for (int i = 0; i<nSize; ++i)
		{
			m_LockedTextures[i] = false;
		}
	}
	else
	{
		if (nIndex < (int)(m_LockedTextures.size()))
		{
			m_LockedTextures[nIndex] = false;
		}
	}
}

bool ParaEngine::CEffectFileImpl::IsTextureLocked(int nIndex) const
{
	return (nIndex < (int)(m_LockedTextures.size())) && m_LockedTextures[nIndex];
}



void ParaEngine::CEffectFileImpl::OnSwitchInShader()
{
}

void ParaEngine::CEffectFileImpl::OnSwitchOutShader()
{
	vector<void*>::iterator itCur, itEnd = m_LastTextures.end();
	for (itCur = m_LastTextures.begin(); itCur != itEnd; ++itCur)
	{
		(*itCur) = NULL;
	}
}

bool ParaEngine::CEffectFileImpl::BeginSharePassMode(bool bApplyParam)
{
	EnableShareMode(true);
	if (begin(bApplyParam,true))
	{
		return BeginPass(0, true);
	}
	return false;
}

void ParaEngine::CEffectFileImpl::EndSharePassMode()
{
	EnableShareMode(false);
	EndPass(true);
	end(true);
}

void ParaEngine::CEffectFileImpl::SetFileName(const std::string& filename)
{
	m_filename = filename;
}

IParaEngine::ParameterHandle& ParaEngine::CEffectFileImpl::GetTextureHandle(int nIndex)
{
	return m_paramHandle[k_tex0 + nIndex];
}


bool ParaEngine::CEffectFileImpl::SetRawValue(const char* name, const void* pData, uint32_t ByteOffset, uint32_t Bytes)
{
	return m_pEffect->SetRawValue(name, pData, ByteOffset, Bytes);
}

bool ParaEngine::CEffectFileImpl::SetBool(const char* name, bool bBoolean)
{
	return SetRawValue(name, &bBoolean, 0, sizeof(bBoolean));
}

bool ParaEngine::CEffectFileImpl::SetInt(const char* name, int nValue)
{
	return SetRawValue(name, &nValue, 0, sizeof(nValue));
}


bool ParaEngine::CEffectFileImpl::SetFloat(const char* name, float fValue)
{
	return SetRawValue(name, &fValue, 0, sizeof(fValue));
}

bool ParaEngine::CEffectFileImpl::SetVector2(const char* name, const Vector2& vValue)
{
	return SetRawValue(name, &vValue, 0, sizeof(vValue));
}


bool ParaEngine::CEffectFileImpl::SetVector3(const char* name, const Vector3& vValue)
{
	return SetRawValue(name, &vValue, 0, sizeof(vValue));
}


bool ParaEngine::CEffectFileImpl::SetVector4(const char* name, const Vector4& vValue)
{
	return SetRawValue(name, &vValue, 0, sizeof(vValue));
}


bool ParaEngine::CEffectFileImpl::SetMatrix(const char* name, const Matrix4& data)
{
	return SetRawValue(name, &data, 0, sizeof(data));
}