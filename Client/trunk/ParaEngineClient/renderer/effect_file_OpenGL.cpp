//-----------------------------------------------------------------------------
// Class:	EffectFile opengl
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_OPENGL_RENDERER
#include "platform/OpenGLWrapper.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "SkyMesh.h"
#include "AutoCamera.h"
#include "effect_file_OpenGL.h"
#include "SunLight.h"
using namespace ParaEngine;
USING_NS_CC;
// TODO: add more buildin shaders here
#define STRINGIFY(A)  #A
#include "platform/shaders/blockEffect.shader"
#include "platform/shaders/blockSelectEffect.shader"
#include "platform/shaders/singleColorEffect.shader"
#include "platform/shaders/meshNormalEffect.shader"
#include "platform/shaders/particleEffect.shader"
#include "platform/shaders/guiEffect.shader"
#include "platform/shaders/guiTextEffect.shader"
#include "platform/shaders/skymeshEffect.shader"
#include "platform/shaders/skydomeEffect.shader"
#include "platform/shaders/terrainEffect.shader"
#include "platform/shaders/blockMaxEffect.shader"

std::unordered_map<uint32, std::string> CEffectFileOpenGL::s_id_to_names = CEffectFileOpenGL::LoadStaticIdNameMap();

std::unordered_map<uint32, std::string> CEffectFileOpenGL::LoadStaticIdNameMap()
{
	std::unordered_map<uint32, std::string> fields;
	fields[k_worldMatrix] = "world";
	fields[k_worldInverseMatrix] = "worldinverse";
	fields[k_worldViewMatrix] = "worldview";
	fields[k_worldViewProjMatrix] = "worldviewprojection";
	fields[k_worldMatrixArray] = "worldmatrixarray";
	fields[k_skyBoxMatrix] = "skyboxmatrix";
	fields[k_viewMatrix] = "view";
	fields[k_projMatrix] = "projection";
	fields[k_viewProjMatrix] = "viewprojection";
	fields[k_TexWorldViewProjMatrix] = "texworldviewproj";
	fields[k_ambientMaterialColor] = "materialambient";
	fields[k_diffuseMaterialColor] = "materialdiffuse";
	fields[k_specularMaterialColor] = "materialspecular";
	fields[k_emissiveMaterialColor] = "materialemissive";
	fields[k_posScaleOffset] = "posScaleOffset";
	fields[k_uvScaleOffset] = "uvScaleOffset";
	fields[k_lensFlareColor] = "flareColor";
	fields[k_fogParameters] = "fogparameters";
	fields[k_fogColor] = "fogColor";
	fields[k_shadowFactor] = "shadowfactor";
	fields[k_LightStrength] = "LightStrength";
	fields[k_LightColors] = "LightColors";
	fields[k_LightPositions] = "LightPositions";
	fields[k_fresnelR0] = "FresnelR0";
	fields[k_ConstVector0] = "ConstVector0";
	fields[k_ConstVector1] = "ConstVector1";
	fields[k_ConstVector2] = "ConstVector2";
	fields[k_ConstVector3] = "ConstVector3";
	fields[k_sunVector] = "sunvector";
	fields[k_sunColor] = "suncolor";
	fields[k_cameraPos] = "worldcamerapos";
	fields[k_cameraDistances] = "viewdistances";
	fields[k_cameraFacing] = "worldviewvector";
	fields[k_ambientLight] = "ambientlight";
	fields[k_sunlightInscatter] = "sunlight_inscatter";
	fields[k_sunlightExtinction] = "sunlight_extinction";
	fields[k_worldPos] = "worldpos";
	fields[k_texCoordOffset] = "texCoordOffset";
	fields[k_boneInfluenceCount] = "curnumbones";
	fields[k_fogEnable] = "fogenable";
	fields[k_bAlphaTesting] = "alphatesting";
	fields[k_bAlphaBlending] = "alphablending";
	fields[k_bBoolean0] = "k_bBoolean0";
	fields[k_bBoolean1] = "k_bBoolean1";
	fields[k_bBoolean2] = "k_bBoolean2";
	fields[k_bBoolean3] = "k_bBoolean3";
	fields[k_bBoolean4] = "k_bBoolean4";
	fields[k_bBoolean5] = "k_bBoolean5";
	fields[k_bBoolean6] = "k_bBoolean6";
	fields[k_bBoolean7] = "k_bBoolean7";
	fields[k_bBoolean8] = "k_bBoolean8";
	fields[k_bBoolean9] = "k_bBoolean9";
	fields[k_bBoolean10] = "k_bBoolean10";
	fields[k_bBoolean11] = "k_bBoolean11";
	fields[k_bBoolean12] = "k_bBoolean12";
	fields[k_bBoolean13] = "k_bBoolean13";
	fields[k_bBoolean14] = "k_bBoolean14";
	fields[k_bBoolean15] = "k_bBoolean15";
	fields[k_bSunlightEnable] = "sunlightenable";
	fields[k_nShadowmapSize] = "shadowmapsize";
	fields[k_fShadowRadius] = "shadowradius";
	fields[k_specularMaterialPower] = "materialpower";
	fields[k_reflectFactor] = "reflectfactor";
	fields[k_LocalLightNum] = "locallightnum";
	fields[k_LayersNum] = "layersnum";
	fields[k_time] = "time";
	fields[k_opacity] = "opacity";
	fields[k_specularPower] = "specularPower";
	fields[k_transitionFactor] = "transitionFactor";
	return fields;
}


ParaEngine::CEffectFileOpenGL::CEffectFileOpenGL(const char* filename)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_pendingChangesCount(0)
{
	SetFileName(filename);
	Init();
}

ParaEngine::CEffectFileOpenGL::CEffectFileOpenGL(const AssetKey& key)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_pendingChangesCount(0)
{
	Init();
}

ParaEngine::CEffectFileOpenGL::~CEffectFileOpenGL()
{
	releaseEffect();
}


void ParaEngine::CEffectFileOpenGL::Init()
{
}

void ParaEngine::CEffectFileOpenGL::SetFileName(const std::string& filename)
{
	m_filename = filename;
}

const std::string& ParaEngine::CEffectFileOpenGL::GetFileName()
{
	return m_filename;
}

HRESULT ParaEngine::CEffectFileOpenGL::InitDeviceObjects()
{
	m_bIsInitialized = true;
	if (!LoadBuildinShader())
	{
		OUTPUT_LOG("warning: unknown shader %s\n", GetFileName().c_str());
	}
	return S_OK;
}

HRESULT ParaEngine::CEffectFileOpenGL::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	releaseEffect();
	return S_OK;
}

void ParaEngine::CEffectFileOpenGL::releaseEffect(int nPass)
{
	if (nPass < 0)
	{
		for (auto program : m_programs)
		{
			CC_SAFE_RELEASE_NULL(program);
		}
		m_programs.clear();
	}
	else
	{
		auto program = GetGLProgram(nPass);
		if (program)
		{
			CC_SAFE_RELEASE_NULL(m_programs[nPass]);
		}
	}
}

void ParaEngine::CEffectFileOpenGL::EnableTextures(bool bEnable)
{

}

bool ParaEngine::CEffectFileOpenGL::AreTextureEnabled()
{
	return true;
}

void ParaEngine::CEffectFileOpenGL::EnableSunLight(bool bEnableSunLight)
{

}

bool ParaEngine::CEffectFileOpenGL::setMatrix(eParameterHandles index, const Matrix4* data)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program && data!=0)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			// transpose it, since openGL and directX packed matrix differently.
			// Xizhi 2014.9.16: for some reason, it does not need to be transposed, opengl already packed data in our way.
			// Matrix4 matTranposed = data->transpose();
			program->setUniformLocationWithMatrix4fv(uniform->location, (const GLfloat*)(data), 1);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return false;
}

bool ParaEngine::CEffectFileOpenGL::isMatrixUsed(eParameterHandles index)
{
	return isParameterUsed(index);
}

bool ParaEngine::CEffectFileOpenGL::setParameter(cocos2d::Uniform* uniform, const void* data, int32 size)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (uniform && program && data != 0)
	{
		if (uniform->type == GL_INT)
			program->setUniformLocationWith1i(uniform->location, *((const GLint*)(data)));
		else if (uniform->type == GL_BOOL)
			program->setUniformLocationWith1i(uniform->location, *((const bool*)(data)));
		else if (uniform->type == GL_FLOAT)
			program->setUniformLocationWith1f(uniform->location, *((const GLfloat*)(data)));
		else if (uniform->type == GL_FLOAT_VEC3)
			program->setUniformLocationWith3fv(uniform->location, (const GLfloat*)(data), uniform->size);
		else if (uniform->type == GL_FLOAT_VEC2)
			program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), uniform->size);
		else if (uniform->type == GL_FLOAT_VEC4)
			program->setUniformLocationWith4fv(uniform->location, (const GLfloat*)(data), uniform->size);
		else if (uniform->type == GL_FLOAT_MAT4)
			program->setUniformLocationWithMatrix4fv(uniform->location, (const GLfloat*)(data), uniform->size);
		else if (size > 0)
			program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), (uint32)((size + 1) / 2));
		else
		{
			OUTPUT_LOG("warn: unknown uniform size and type\n");
		}
		// PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const void* data, int32 size /*= D3DX_DEFAULT*/)
{
	return setParameter(GetUniformByID(index), data, size);
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector2* data)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program && data != 0)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), 1);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector3* data)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program && data != 0)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			program->setUniformLocationWith3fv(uniform->location, (const GLfloat*)(data), 1);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector4* data)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program && data != 0)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			program->setUniformLocationWith4fv(uniform->location, (const GLfloat*)(data), 1);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setBool(eParameterHandles index, BOOL bBoolean)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			program->setUniformLocationWith1i(uniform->location, bBoolean ? 1 : 0);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setInt(eParameterHandles index, int nValue)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			program->setUniformLocationWith1i(uniform->location, nValue);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setFloat(eParameterHandles index, float fValue)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program)
	{
		Uniform* uniform = GetUniformByID(index);
		if (uniform)
		{
			program->setUniformLocationWith1f(uniform->location, fValue);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::isParameterUsed(eParameterHandles index)
{
	return GetUniformByID(index) != 0;
}

bool ParaEngine::CEffectFileOpenGL::initWithByteArrays(const char* vShaderByteArray, const char* fShaderByteArray, int nPass)
{
	releaseEffect(nPass);
	auto program = GetGLProgram(nPass, true);
	if (program && program->initWithByteArrays(vShaderByteArray, fShaderByteArray))
		return true;
	else
	{
		OUTPUT_LOG("error: failed to compile shader: %s \n", GetFileName().c_str());
		if (program)
		{
			OUTPUT_LOG("vertex shader log: %s\nFragment shader log: %s\n", program->getVertexShaderLog().c_str(), program->getFragmentShaderLog().c_str());
		}
		return false;
	}
}

bool ParaEngine::CEffectFileOpenGL::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, int nPass)
{
	releaseEffect(nPass);
	auto program = GetGLProgram(nPass, true);
	if (program && program->initWithFilenames(vShaderFilename, fShaderFilename))
		return true;
	else
	{
		OUTPUT_LOG("error: failed to compile shader: %s \n", GetFileName().c_str());
		if (program)
		{
			OUTPUT_LOG("vertex shader log: %s\nFragment shader log: %s\n", program->getVertexShaderLog().c_str(), program->getFragmentShaderLog().c_str());
		}
		return false;
	}
}

cocos2d::GLProgram* ParaEngine::CEffectFileOpenGL::GetGLProgram(int nPass, bool bCreateIfNotExist)
{
	if ((int)m_programs.size() <= nPass && bCreateIfNotExist)
	{
		m_programs.resize(nPass+1, NULL);
	}
	if ((int)m_programs.size() > nPass)
	{
		auto program = m_programs[nPass];
		if (program == NULL && bCreateIfNotExist)
		{
			program = new cocos2d::GLProgram();
			m_programs[nPass] = program;
		}
		return program;
	}
	return NULL;
}

bool ParaEngine::CEffectFileOpenGL::link(int nPass)
{
	auto program = GetGLProgram(nPass);
	if (program)
	{
		// we need to support color2 for block shader
		program->bindAttribLocation("a_color2", GLProgram::VERTEX_ATTRIB_MAX + 0);
	}

	if (program && program->link())
	{
		OUTPUT_LOG("successfully linked shader program(glID:%d) %s. \n", program->getProgram(), GetFileName().c_str());
		return true;
	}
	else
	{
		OUTPUT_LOG("error: shader compiled but failed to link shader %s\n", GetFileName().c_str());
		if (program)
		{
			OUTPUT_LOG("shader log: %s\n", program->getProgramLog().c_str());
		}
	}
	return false;
}

bool ParaEngine::CEffectFileOpenGL::use(int nPass)
{
	auto program = GetGLProgram(nPass);
	if (program)
	{
		program->use();
		PE_CHECK_GL_ERROR_DEBUG();
		if (m_nActivePassIndex != nPass)
		{
			m_nActivePassIndex = nPass;
			// TODO: copy all uniforms to the new pass.
		}
		return true;
	}
	return false;
}

void ParaEngine::CEffectFileOpenGL::updateUniforms(int nPass)
{
	if (nPass < 0)
		nPass = m_nActivePassIndex;
	auto program = GetGLProgram(nPass);
	if (program)
	{
		program->updateUniforms();
		auto errorCode = glGetError();
		if (errorCode)
		{
			OUTPUT_LOG("unknown opengl error: 0x%04X before updateUniforms: %s pass: %d\n", errorCode, m_filename.c_str(), nPass);
		}
		// PE_CHECK_GL_ERROR_DEBUG();
	}
}

cocos2d::Uniform* ParaEngine::CEffectFileOpenGL::GetUniformByID(eParameterHandles id)
{
	return GetUniform(s_id_to_names[id]);
}

cocos2d::Uniform* ParaEngine::CEffectFileOpenGL::GetUniform(const std::string& sName)
{
	auto program = GetGLProgram(m_nActivePassIndex);
	if (program)
	{
		return program->getUniform(sName);
	}
	return NULL;
}

void ParaEngine::CEffectFileOpenGL::applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor)
{
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
}

void ParaEngine::CEffectFileOpenGL::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient /*= true*/)
{
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
}

void ParaEngine::CEffectFileOpenGL::applyCameraMatrices()
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

void ParaEngine::CEffectFileOpenGL::applyWorldMatrices()
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

void ParaEngine::CEffectFileOpenGL::applyGlobalLightingData(CSunLight& sunlight)
{
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
		setParameter(k_shadowFactor,&v);
	}
}

bool ParaEngine::CEffectFileOpenGL::begin(bool bApplyParam /*= true*/, DWORD flag /*= 0*/)
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();
	auto program = GetGLProgram();
	if (program != 0)
	{
		if (m_programs.size() == 1)
			program->use();
		else
		{
			// TODO: multiple pass effect in opengl is implemented as shader arrays.
			// uniform values are currently NOT shared among different passes like directX.
			// hence DO NOT set any uniform between begin() and beginPass().
			// TODO: In future: uniform values should be cached until Commit() is called.
			program->use();
		}

		if (bApplyParam)
		{
			// set the lighting parameters
			// from the global light manager
			applyGlobalLightingData(pScene->GetSunLight());

			// set the camera matrix
			applyCameraMatrices();
		}

		m_bIsBegin = true;
		return true;
	}
	else
		return false;
}

bool ParaEngine::CEffectFileOpenGL::BeginPass(int nPass, bool bForceBegin /*= false*/)
{
	return use(nPass);
}

void ParaEngine::CEffectFileOpenGL::CommitChanges()
{
	if (m_pendingChangesCount > 0)
	{
		for (uint32 i = 0; i < m_pendingChangesCount;++i)
		{
			CParameter& param = m_pendingChanges[i];
			setParameter(GetUniform(param.GetName()), param.GetRawData(), param.GetRawDataLength());
		}
		m_pendingChangesCount = 0;
	}
}

void ParaEngine::CEffectFileOpenGL::EndPass(bool bForceEnd /*= false*/)
{

}

void ParaEngine::CEffectFileOpenGL::end(bool bForceEnd /*= false*/)
{
	m_bIsBegin = false;
}

HRESULT ParaEngine::CEffectFileOpenGL::RendererRecreated()
{
	for (auto program : m_programs)
	{
		if (program)
			program->reset();
		CC_SAFE_RELEASE_NULL(program);
	}
	m_programs.clear();
	m_bIsInitialized = false;
	return S_OK;
}

bool ParaEngine::CEffectFileOpenGL::EnableEnvironmentMapping(bool bEnable)
{
	return true;
}

bool ParaEngine::CEffectFileOpenGL::EnableReflectionMapping(bool bEnable, float fSurfaceHeight /*= 0.f*/)
{
	return true;
}

void ParaEngine::CEffectFileOpenGL::SetReflectFactor(float fFactor)
{

}

void ParaEngine::CEffectFileOpenGL::EnableNormalMap(bool bEnable)
{

}

void ParaEngine::CEffectFileOpenGL::EnableLightMap(bool bEnable)
{

}

void ParaEngine::CEffectFileOpenGL::EnableAlphaBlending(bool bAlphaBlending)
{
	if (isParameterUsed(k_bAlphaBlending))
	{
		AddParamChange(s_id_to_names[k_bAlphaBlending], bAlphaBlending);
		// setBool(k_bAlphaBlending, bAlphaBlending);
	}
}

void ParaEngine::CEffectFileOpenGL::EnableAlphaTesting(bool bAlphaTesting)
{
	if (isParameterUsed(k_bAlphaTesting))
	{
		AddParamChange(s_id_to_names[k_bAlphaTesting], bAlphaTesting);
		// setBool(k_bAlphaTesting, bAlphaTesting);
	}
}

int ParaEngine::CEffectFileOpenGL::totalPasses() const
{
	return (int)m_programs.size();
}

bool ParaEngine::CEffectFileOpenGL::setTexture(int index, TextureEntity* data)
{
	if (data != 0 && data->GetTexture())
	{
		setTexture(index, data->GetTexture());

		// ensure that sampler states matches the one used in the texture. if not, change the texture sampler
		// unless a texture is used with different sampler states during rendering, the glTexParameteri function is called at most once for a texture.
		DWORD dwValue = 0;
		CGlobals::GetRenderDevice()->GetSamplerState(index, D3DSAMP_MINFILTER, &dwValue);
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

bool ParaEngine::CEffectFileOpenGL::setTexture(int index, DeviceTexturePtr_type pTex)
{
	return SUCCEEDED(CGlobals::GetRenderDevice()->SetTexture(index, pTex));
}

HRESULT ParaEngine::CEffectFileOpenGL::SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 nBytes)
{
	cocos2d::Uniform* uniform = GetUniform(hParameter);
	if (uniform)
	{
		PE_ASSERT(ByteOffset == 0);
		return setParameter(uniform, pData, nBytes) ? S_OK : E_FAIL;
	}
	return E_FAIL;
}
bool ParaEngine::CEffectFileOpenGL::SetBool(const char* hParameter, BOOL bBoolean)
{
	return SUCCEEDED(SetRawValue(hParameter, &bBoolean, 0, sizeof(bBoolean)));
}

bool ParaEngine::CEffectFileOpenGL::SetInt(const char* hParameter, int nValue)
{
	return SUCCEEDED(SetRawValue(hParameter, &nValue, 0, sizeof(nValue)));
}

bool ParaEngine::CEffectFileOpenGL::SetFloat(const char* hParameter, float fValue)
{
	return SUCCEEDED(SetRawValue(hParameter, &fValue, 0, sizeof(fValue)));
}

bool ParaEngine::CEffectFileOpenGL::SetVector2(const char* hParameter, const Vector2& vValue)
{
	return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
}

bool ParaEngine::CEffectFileOpenGL::SetVector3(const char* hParameter, const Vector3& vValue)
{
	return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
}

bool ParaEngine::CEffectFileOpenGL::SetVector4(const char* hParameter, const Vector4& vValue)
{
	return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
}

bool ParaEngine::CEffectFileOpenGL::SetMatrix(const char* hParameter, const Matrix4& data)
{
	return SUCCEEDED(SetRawValue(hParameter, &data, 0, sizeof(data)));
}


bool ParaEngine::CEffectFileOpenGL::LoadBuildinShader()
{
	if (m_filename == ":IDR_FX_BLOCK")
	{
		int nPass = 0;
		if (initWithByteArrays(shaderBlockEffect_vert, shaderOpacheBlockEffect_frag, nPass))
		{
			if (link(nPass))
			{
				updateUniforms(nPass);
			}
		}
		nPass = 1;
		if (initWithByteArrays(shaderBlockSelectEffect_vert, shaderBlockSelectEffect_frag, nPass))
		{
			if (link(nPass))
			{
				updateUniforms(nPass);
			}
		}
		nPass = 2;
		if (initWithByteArrays(shaderTransparentBlockEffect_vert, shaderTransparentBlockEffect_frag, nPass))
		{
			if (link(nPass))
			{
				updateUniforms(nPass);
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_SINGLECOLOR")
	{
		if (initWithByteArrays(singleColorEffect_vert, singleColorEffect_frag))
		{
			if (link())
			{
				updateUniforms();
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_SIMPLE_MESH_NORMAL")
	{
		if (initWithByteArrays(meshNormalEffect_vert, meshNormalEffect_frag))
		{
			if (link())
			{
				updateUniforms();
				// set initial value, opengl does not support initial value in shader.
				SetFloat("opacity", 1.f);
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_SIMPLE_PARTICLE")
	{
		if (initWithByteArrays(particleEffect_vert, particleEffect_frag))
		{
			if (link())
			{
				updateUniforms();
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_GUI")
	{
		if (initWithByteArrays(guiEffect_vert, guiEffect_frag))
		{
			if (link())
			{
				updateUniforms();
				// set initial value, opengl does not support initial value in shader.
				SetBool("k_bBoolean0", true);
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_GUI_TEXT")
	{
		if (initWithByteArrays(guiTextEffect_vert, guiTextEffect_frag))
		{
			if (link())
			{
				updateUniforms();
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_SKY")
	{
		if (initWithByteArrays(skyMeshEffect_vert, skyMeshEffect_frag))
		{
			if (link())
			{
				updateUniforms();
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_SKYDOME")
	{
		int nPass = 0;
		if (initWithByteArrays(skyDomeEffect_vert, skyDomeDayEffect_frag, nPass))
		{
			if (link(nPass))
			{
				updateUniforms(nPass);
			}
		}
		nPass = 1;
		if (initWithByteArrays(skyDomeEffect_vert, skyDomeNightEffect_frag, nPass))
		{
			if (link(nPass))
			{
				updateUniforms(nPass);
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_TERRAIN_NORMAL")
	{
		if (initWithByteArrays(terrainEffect_vert, terrainEffect_frag))
		{
			if (link())
			{
				updateUniforms();
			}
		}
		return true;
	}
	else if (m_filename == ":IDR_FX_BMAXMODEL")
	{
		if (initWithByteArrays(shaderBlockMaxEffect_vert, shaderBlockMaxEffect_frag))
		{
			if (link())
			{
				updateUniforms();
			}
		}
		return true;
	}
	return false;
}

bool ParaEngine::CEffectFileOpenGL::SetFirstValidTechniqueByCategory(TechniqueCategory nCat)
{
	return false;
}

bool ParaEngine::CEffectFileOpenGL::SetTechniqueByIndex(int nIndex)
{
	return false;
}

const ParaEngine::CEffectFileOpenGL::TechniqueDesc* ParaEngine::CEffectFileOpenGL::GetCurrentTechniqueDesc()
{
	static TechniqueDesc s_tech;
	return &s_tech;
}

bool ParaEngine::CEffectFileOpenGL::SetBoolean(int nIndex, bool value)
{
	PE_ASSERT(nIndex < (k_bBooleanMAX - k_bBoolean0));
	if (isParameterUsed((eParameterHandles)(k_bBoolean0 + nIndex)))
	{
		return setBool((eParameterHandles)(k_bBoolean0 + nIndex), value);
	}
	return false;
}

#endif
