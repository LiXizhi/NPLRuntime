//-----------------------------------------------------------------------------
// Class:	EffectFile opengl
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_OPENGL_RENDERER
#include "RenderDeviceOpenGL.h"
#include "OpenGLWrapper/GLProgram.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "SkyMesh.h"
#include "AutoCamera.h"
#include "effect_file_OpenGL.h"
#include "SunLight.h"
#include "driver.h"
#include "dxeffects.h"
#include "hlsl2glsl.h"
#include "hlslCrossCompiler.h"
#include "hlslLinker.h"
#include "ShaderIncludeHandle.h"
#include <boost/filesystem.hpp>

using namespace ParaEngine;


CEffectFileOpenGL::CEffectFileOpenGL(const char* filename)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_bIsBeginPass(false), m_pendingChangesCount(0)
	, m_nTechniqueIndex(0)
	, m_Effect(nullptr)
{
	SetFileName(filename);
	Init();
}

CEffectFileOpenGL::CEffectFileOpenGL(const AssetKey& key)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_pendingChangesCount(0)
	, m_nTechniqueIndex(0)
	, m_Effect(nullptr)
{
	Init();
}

CEffectFileOpenGL::~CEffectFileOpenGL()
{
	if (m_Effect != nullptr)
	{
		m_Effect = nullptr;
	}
	releaseEffect();
}


void CEffectFileOpenGL::Init()
{
	for (int i =0;i<k_max_param_handles;i++)
	{
		m_ParamHandle[i].idx = PARA_INVALID_HANDLE;
	}
}

void CEffectFileOpenGL::SetFileName(const std::string& filename)
{
	m_filename = filename;
}

const std::string& CEffectFileOpenGL::GetFileName()
{
	return m_filename;
}


void CEffectFileOpenGL::parseParameters()
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
	for (uint32_t index = 0; index < m_EffectDesc.Parameters; index++)
	{
		IParaEngine::ParameterHandle hParam = m_Effect->GetParameter(index);
		if (!IParaEngine::isValidHandle(hParam))continue;
		if (!m_Effect->GetParameterDesc(hParam, &ParamDesc)) continue;

		std::string lower_sem = ParamDesc.Semantic;
		std::transform(lower_sem.begin(), lower_sem.end(), lower_sem.begin(),
			[](unsigned char c) { return std::tolower(c); }
		);

		auto it = table.find(lower_sem);
		if (it != table.end())
		{
			m_ParamHandle[it->second] = hParam;
		}
		else
		{
			if (   ParamDesc.Type == EParameterType::PT_TEXTURE 
				|| ParamDesc.Type == EParameterType::PT_TEXTURE2D 
				|| ParamDesc.Type == EParameterType::PT_TEXTURE3D 
				|| ParamDesc.Type == PT_TEXTURECUBE) {

				int iPos = (int)ParamDesc.Name.find_first_of(numerals, 0, sizeof(numerals));

				if (iPos != string::npos)
				{
					int iTexture = atoi(&ParamDesc.Name[iPos]);
					if (iTexture >= 0 && iTexture < (k_tex_max - k_tex0))
					{
						m_ParamHandle[k_tex0 + iTexture] = hParam;
					}
				}
			}
			else {
				OUTPUT_LOG("Warning: unsupported paramter::%s :%s  at %s \n", ParamDesc.Name.c_str(), ParamDesc.Semantic.c_str(), m_filename.c_str());
			}
		}
	}
}



HRESULT CEffectFileOpenGL::InitDeviceObjects()
{
	m_bIsInitialized = true;
	m_bIsValid = false;//set to true if created successfully.
	auto pRenderDevice = CGlobals::GetRenderDevice();

	auto file = std::make_shared<CParaFile>(m_filename.c_str());

	// fxo 
	if (file->isEof())
	{
		file = nullptr;
		if (m_filename.find(".fxo") != std::string::npos)
		{
			std::string fxname = m_filename.substr(0, m_filename.size() - 1);
			file = std::make_shared<CParaFile>(fxname.c_str());
			const char* source = file->getBuffer();
		}
	}

	std::string error = "";

	if (!file->isEof())
	{
		/*
		// Since we are loading a binary file here and this effect has already been compiled,
		// you can not pass compiler flags here (for example to debug the shaders).
		// To debug the shaders, one must pass these flags to the compiler that generated the
		// binary (for example fxc.exe).
		- From within the Solution Explorer window, right click on *.fx and select Properties from the context menu.
		- Select Configuration Properties/Custom Build Step to view the custom build step directives.
		*/
		boost::filesystem::path p(m_filename);
		auto shaderDir = p.parent_path();
		ShaderIncludeHandle includeImpl(shaderDir.string());
		m_Effect = CGlobals::GetRenderDevice()->CreateEffect(file->getBuffer(), file->getSize(), &includeImpl, error);
	}
	else
	{
		OUTPUT_LOG("ERROR: shader file %s not found\n", m_filename.c_str());
		return E_FAIL;
	}

	if (!m_Effect)
	{
		char* error_str = (error != "") ? error.c_str() : "failed loading effect file\n";
		OUTPUT_LOG("Failed Loading Effect file %s: error is %s\n", m_filename.c_str(), error_str);
		return E_FAIL;
	}


	// get the description
	m_Effect->GetDesc(&m_EffectDesc);

	m_Techniques.clear();
	//////////////////////////////////////////////////////////////////////////
	// get all valid techniques
	TechniqueDesc tech;
	bool bHasDefaultTechnique = false;

	for (int idx = 0; idx<m_EffectDesc.Techniques; idx++)
	{
		tech.hTechnique = m_Effect->GetTechnique(idx);
		if (m_Effect->GetTechniqueDesc(tech.hTechnique, &tech.techniqueDesc))
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
			m_Techniques.push_back(tech);
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
		if (m_Effect->SetTechnique(m_Techniques[m_nTechniqueIndex].hTechnique))
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

	if (m_filename == ":IDR_FX_SIMPLE_MESH_NORMAL")
	{
		SetFloat("opacity", 1.f);
	}
	if (m_filename == ":IDR_FX_GUI")
	{
		SetBool("k_bBoolean0", true);
	}

	return result;
}

HRESULT CEffectFileOpenGL::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	releaseEffect();
	return S_OK;
}

void CEffectFileOpenGL::releaseEffect(int nTech, int nPass)
{
	m_Effect = nullptr;
}

void CEffectFileOpenGL::EnableTextures(bool bEnable)
{

}

bool CEffectFileOpenGL::AreTextureEnabled()
{
	return true;
}

void CEffectFileOpenGL::EnableSunLight(bool bEnableSunLight)
{

}



bool CEffectFileOpenGL::setMatrix(eParameterHandles index, const Matrix4* data)
{
	if (m_Effect && isMatrixUsed(index))
	{
		return m_Effect->SetMatrix(m_ParamHandle[index], data->GetConstPointer());
	}
	return false;
}

bool CEffectFileOpenGL::isMatrixUsed(eParameterHandles index)
{
	return isParameterUsed(index);
}


bool CEffectFileOpenGL::setParameter(eParameterHandles index, const void* data, int32 size /*= D3DX_DEFAULT*/)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetValue(m_ParamHandle[index], data, size);
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector2* data)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetValue(m_ParamHandle[index], data, sizeof(Vector2));
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector3* data)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetValue(m_ParamHandle[index], data, sizeof(Vector3));
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector4* data)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetVector(m_ParamHandle[index], (DeviceVector4*)data);
		PE_ASSERT(result);
		return result;
	}
	return false;
}
bool CEffectFileOpenGL::setBool(eParameterHandles index, BOOL bBoolean)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetBool(m_ParamHandle[index], bBoolean);
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileOpenGL::setInt(eParameterHandles index, int nValue)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetInt(m_ParamHandle[index], nValue);
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileOpenGL::setFloat(eParameterHandles index, float fValue)
{
	if (m_Effect && isParameterUsed(index))
	{
		bool result = m_Effect->SetFloat(m_ParamHandle[index],fValue);
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileOpenGL::isParameterUsed(eParameterHandles index)
{
	return isValidHandle(m_ParamHandle[index]);
}











void CEffectFileOpenGL::applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor)
{
	//SetProgramParams([&](GLProgram* program) {
		if (isParameterUsed(k_fogEnable))
		{
			setBool(k_fogEnable, bEnableFog);
		}
		// unlike directx, we will apply for parameters regardless of whether fog is enabled.
		// if (bEnableFog)
		{
			if (isParameterUsed(k_fogParameters) && (fogParam != 0))
			{
				setParameter(k_fogParameters, fogParam);
			}

			if (isParameterUsed(k_fogColor) && (fogColor != 0))
			{
				setParameter(k_fogColor, fogColor);
			}
		}
	//	return true;
	//});
}

void CEffectFileOpenGL::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient /*= true*/)
{
	//SetProgramParams([&](GLProgram* program) {
		if (pSurfaceMaterial)
		{
			// set material properties
			const ParaMaterial & d3dMaterial = *pSurfaceMaterial;

			if (isParameterUsed(k_ambientMaterialColor))
			{
				if (bUseGlobalAmbient && (d3dMaterial.Ambient.r < 0.01f))
					setParameter(k_ambientMaterialColor, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Ambient);
				else
					setParameter(k_ambientMaterialColor, &d3dMaterial.Ambient);
			}

			if (isParameterUsed(k_diffuseMaterialColor))
			{
				if (CGlobals::GetEffectManager()->GetScene()->GetSceneState()->HasLocalMaterial())
				{
					setParameter(k_diffuseMaterialColor, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Diffuse);
					setParameter(k_LightStrength, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentLightStrength());
				}
				else
				{
					setParameter(k_diffuseMaterialColor, &d3dMaterial.Diffuse);
					Vector3 vEmpty(0, 0, 0);
					setParameter(k_LightStrength, &vEmpty);
				}
			}

			if (isParameterUsed(k_specularMaterialColor))
			{
				setParameter(k_specularMaterialColor, &d3dMaterial.Specular);
			}

			if (isParameterUsed(k_emissiveMaterialColor))
			{
				setParameter(k_specularMaterialColor, &d3dMaterial.Emissive);
			}

			if (isParameterUsed(k_specularMaterialPower))
			{
				setParameter(k_specularMaterialPower, &d3dMaterial.Power);
			}
		}
	//	return true;
	//});
}

void CEffectFileOpenGL::applyCameraMatrices()
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
				setParameter(k_cameraPos, &vEye);
			}
			// set the world camera facing vector
			if (isParameterUsed(k_cameraFacing))
			{
				Vector3 v = pCamera->GetWorldAhead();
				setParameter(k_cameraFacing, &v);
			}
		}
}

void CEffectFileOpenGL::applyWorldMatrices()
{
	//SetProgramParams([&](GLProgram* program) {
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
	//	return true;
	//});
}

void CEffectFileOpenGL::applyGlobalLightingData(CSunLight& sunlight)
{
	//SetProgramParams([&](GLProgram* program) {
		// pass the lighting structure to the shader
		if (isParameterUsed(k_sunColor))
		{
			LinearColor c = sunlight.GetSunColor();
			setParameter(k_sunColor, &c);
		}

		if (isParameterUsed(k_sunVector))
		{
			Vector3 vDir = -sunlight.GetSunDirection();
			Vector4 v(vDir.x, vDir.y, vDir.z, 1.0f);
			setParameter(k_sunVector, &v);
		}

		if (isParameterUsed(k_ambientLight))
		{
			setParameter(k_ambientLight, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Ambient);
		}


		if (isParameterUsed(k_shadowFactor))
		{
			float shadowFactor = sunlight.GetShadowFactor();
			Vector4 v(shadowFactor, 1 - shadowFactor, 0, 0);
			setParameter(k_shadowFactor, &v);
		}
	//	return true;
	//});
}

bool CEffectFileOpenGL::begin(bool bApplyParam /*= true*/, DWORD flag /*= 0*/)
{

	if (m_Effect->Begin())
	{
		m_bIsBegin = true;
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		if (bApplyParam)
		{
			//SetProgramParams([&](GLProgram* program) {
			// set the lighting parameters
			// from the global light manager
			applyGlobalLightingData(pScene->GetSunLight());

			// set the camera matrix
			applyCameraMatrices();
		}
		return true;
	}
	return false;
}

bool CEffectFileOpenGL::BeginPass(int nPass, bool bForceBegin /*= false*/)
{
	m_bIsBeginPass = true;
	return m_Effect->BeginPass(nPass);
}

void CEffectFileOpenGL::CommitChanges()
{
	m_Effect->CommitChanges();
}

void CEffectFileOpenGL::EndPass(bool bForceEnd /*= false*/)
{
	m_bIsBeginPass = false;
	m_Effect->EndPass();
}

void CEffectFileOpenGL::end(bool bForceEnd /*= false*/)
{
	m_bIsBegin = false;
	m_Effect->End();
}

HRESULT CEffectFileOpenGL::RendererRecreated()
{
	return S_OK;
}

bool CEffectFileOpenGL::EnableEnvironmentMapping(bool bEnable)
{
	return true;
}

bool CEffectFileOpenGL::EnableReflectionMapping(bool bEnable, float fSurfaceHeight /*= 0.f*/)
{
	return true;
}

void CEffectFileOpenGL::SetReflectFactor(float fFactor)
{

}

void CEffectFileOpenGL::EnableNormalMap(bool bEnable)
{

}

void CEffectFileOpenGL::EnableLightMap(bool bEnable)
{

}

void CEffectFileOpenGL::EnableAlphaBlending(bool bAlphaBlending)
{

}

void CEffectFileOpenGL::EnableAlphaTesting(bool bAlphaTesting)
{
}

int CEffectFileOpenGL::totalPasses() const
{
	return 0;
}

bool CEffectFileOpenGL::setTexture(int index, TextureEntity* data)
{
	if (data != 0 && data->GetTexture())
	{
		setTexture(index, data->GetTexture());

		// ensure that sampler states matches the one used in the texture. if not, change the texture sampler
		// unless a texture is used with different sampler states during rendering, the glTexParameteri function is called at most once for a texture.
		uint32_t dwValue = 0;
		CGlobals::GetRenderDevice()->GetSamplerState(index, ESamplerStateType::MINFILTER, &dwValue);
		if (dwValue == D3DTEXF_POINT && !data->IsSamplerStateBlocky())
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			data->SetSamplerStateBlocky(true);
		}
		else if ( dwValue == D3DTEXF_LINEAR && data->IsSamplerStateBlocky() )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			data->SetSamplerStateBlocky(false);
		}
		return true;
	}
	else
		return setTexture(index, (DeviceTexturePtr_type)0);
}

bool CEffectFileOpenGL::isTextureUsed(int index)const
{
	return isValidHandle(m_ParamHandle[k_tex0 + index]);
}

bool CEffectFileOpenGL::setTexture(int index, DeviceTexturePtr_type pTex)
{
	if ( m_Effect && isTextureUsed(index))
	{
		return CGlobals::GetRenderDevice()->SetTexture(index, pTex);
	}
	return false;
}

bool CEffectFileOpenGL::SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 nBytes)
{
	return m_Effect->SetRawValue(hParameter, pData, ByteOffset, nBytes);
}
bool CEffectFileOpenGL::SetBool(const char* hParameter, BOOL bBoolean)
{
	return SetRawValue(hParameter, &bBoolean, 0, sizeof(bBoolean));
}

bool CEffectFileOpenGL::SetInt(const char* hParameter, int nValue)
{
	return SetRawValue(hParameter, &nValue, 0, sizeof(nValue));
}

bool CEffectFileOpenGL::SetFloat(const char* hParameter, float fValue)
{
	return SetRawValue(hParameter, &fValue, 0, sizeof(fValue));
}

bool CEffectFileOpenGL::SetVector2(const char* hParameter, const Vector2& vValue)
{
	return SetRawValue(hParameter, &vValue, 0, sizeof(vValue));
}

bool CEffectFileOpenGL::SetVector3(const char* hParameter, const Vector3& vValue)
{
	return SetRawValue(hParameter, &vValue, 0, sizeof(vValue));
}

bool CEffectFileOpenGL::SetVector4(const char* hParameter, const Vector4& vValue)
{
	return SetRawValue(hParameter, &vValue, 0, sizeof(vValue));
}

bool CEffectFileOpenGL::SetMatrix(const char* hParameter, const Matrix4& data)
{
	return SetRawValue(hParameter, &data, 0, sizeof(data));
}


bool CEffectFileOpenGL::SetFirstValidTechniqueByCategory(TechniqueCategory nCat)
{
	if (m_nTechniqueIndex >= (int)m_Techniques.size())
		return false;
	if (m_Techniques[m_nTechniqueIndex].nCategory == nCat)
		return true;
	vector<TechniqueDesc>::const_iterator itCur, itEnd = m_Techniques.end();
	int i = 0;
	for (itCur = m_Techniques.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if ((*itCur).nCategory == nCat)
		{
			m_nTechniqueIndex = i;
			return true;
		}
	}
	return false;
}

bool CEffectFileOpenGL::SetTechniqueByIndex(int nIndex)
{
	if (m_nTechniqueIndex == nIndex)
		return true;
	else if ((int)m_Techniques.size()>nIndex)
	{
		m_nTechniqueIndex = nIndex;
		return true;
	}
	else
		return false;
}

const CEffectFileOpenGL::TechniqueDesc* CEffectFileOpenGL::GetCurrentTechniqueDesc()
{
	static TechniqueDesc s_tech;
	if (m_nTechniqueIndex<(int)m_Techniques.size())
		return &(m_Techniques[m_nTechniqueIndex]);
	else
		return &s_tech;
}

bool CEffectFileOpenGL::SetBoolean(int nIndex, bool value)
{
	return false;
}

void CEffectFileOpenGL::SetShadowMapSize(int nsize)
{

}
#endif
