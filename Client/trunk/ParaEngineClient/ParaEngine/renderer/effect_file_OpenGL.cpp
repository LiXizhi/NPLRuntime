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
using namespace ParaEngine;

ParaEngine::CEffectFileOpenGL::CEffectFileOpenGL(const char* filename)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_pendingChangesCount(0)
	, mTechniqueIndex(0)
	,m_Effect(nullptr)
{
	SetFileName(filename);
	Init();
}

ParaEngine::CEffectFileOpenGL::CEffectFileOpenGL(const AssetKey& key)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_pendingChangesCount(0)
	, mTechniqueIndex(0)
	, m_Effect(nullptr)
{
	Init();
}

ParaEngine::CEffectFileOpenGL::~CEffectFileOpenGL()
{
	if (m_Effect != nullptr)
	{
		delete m_Effect;
		m_Effect = nullptr;
	}
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
	
	// Load and parse effetcs.
	CParaFile shaderFile(GetFileName().c_str());
	std::string shader_str(shaderFile.getBuffer(), shaderFile.getSize());
	m_Effect = new GLEffectsTree();
	GLEFFECTS::Driver parseDriver(*m_Effect);
	bool ret = parseDriver.parse_string(shader_str);
	if (!ret)
	{
		delete m_Effect;
		m_Effect = nullptr;
		OUTPUT_LOG("error: parse effect failed %s\n", GetFileName().c_str());
		return false;
	}

	// Parse uniforms
	
	ret = MappingEffectUniforms();
	if (ret) {
		OUTPUT_LOG("[%s] Parse uniforms sucessed.\n", GetFileName().c_str());
	}
	else {
		OUTPUT_LOG("[%s] Parse uniforms failed.\n", GetFileName().c_str());
		return false;
	}

	// Init effect
	ret = GeneratePasses();
	if (ret) {
		OUTPUT_LOG("[%s] Generate passes sucessed.\n", GetFileName().c_str());
	}
	else {
		OUTPUT_LOG("[%s] Generate passes failed.\n", GetFileName().c_str());
		return false;
	}

	if (m_filename == ":IDR_FX_SIMPLE_MESH_NORMAL")
	{
		SetFloat("opacity", 1.f);
	}
	if (m_filename == ":IDR_FX_GUI")
	{
		SetBool("k_bBoolean0", true);
	}

	return S_OK;
}

HRESULT ParaEngine::CEffectFileOpenGL::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	releaseEffect();
	return S_OK;
}

void ParaEngine::CEffectFileOpenGL::releaseEffect(int nTech, int nPass)
{
	if (nTech < 0)
	{
		for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
		{
			auto & passes = mTechniques[tech_index].mPasses;
			if (nPass < 0)
			{
				for (auto program : passes)
				{
					SAFE_RELEASE(program);
				}
				passes.clear();
			}
			else
			{
				auto program = GetGLProgram(tech_index, nPass);
				if (program)
				{
					SAFE_RELEASE(passes[nPass]);
				}
			}
		}
	}
	else if ((int)mTechniques.size()>nTech)
	{
		auto & passes = mTechniques[nTech].mPasses;
		if (nPass < 0)
		{
			for (auto program : passes)
			{
				SAFE_RELEASE(program);
			}
			passes.clear();
		}
		else
		{
			auto program = GetGLProgram(nTech, nPass);
			if (program)
			{
				SAFE_RELEASE(passes[nPass]);
			}
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
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program && data != 0)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				// transpose it, since openGL and directX packed matrix differently.
				// Xizhi 2014.9.16: for some reason, it does not need to be transposed, opengl already packed data in our way.
				// Matrix4 matTranposed = data->transpose();
				program->setUniformLocationWithMatrix4fv(uniform->location, (const GLfloat*)(data), 1);
				
				ret = true;
			}
		}
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::isMatrixUsed(eParameterHandles index)
{
	return isParameterUsed(index);
}

bool ParaEngine::CEffectFileOpenGL::setParameter(Uniform* uniform, const void* data, int32 size)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (uniform && program && data != 0)
		{
			if (uniform->type == GL_INT)
			{
				program->setUniformLocationWith1i(uniform->location, *((const GLint*)(data)));

				
			}
			else if (uniform->type == GL_BOOL)
			{
				program->setUniformLocationWith1i(uniform->location, *((const bool*)(data)));
				
			}
			else if (uniform->type == GL_FLOAT)
			{
				program->setUniformLocationWith1f(uniform->location, *((const GLfloat*)(data)));
				
			}
			else if (uniform->type == GL_FLOAT_VEC3)
			{
				program->setUniformLocationWith3fv(uniform->location, (const GLfloat*)(data), uniform->size);
				
			}
			else if (uniform->type == GL_FLOAT_VEC2)
			{
				program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), uniform->size);
				
			}
			else if (uniform->type == GL_FLOAT_VEC4)
			{
				program->setUniformLocationWith4fv(uniform->location, (const GLfloat*)(data), uniform->size);
				
			}
			else if (uniform->type == GL_FLOAT_MAT4)
			{
				program->setUniformLocationWithMatrix4fv(uniform->location, (const GLfloat*)(data), uniform->size);
				
			}
			else if (size > 0)
			{
				program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), (uint32)((size + 1) / 2));
				
			}
			else
			{
				OUTPUT_LOG("warn: unknown uniform size and type\n");
			}
			// 
			ret = true;
		}
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const void* data, int32 size /*= D3DX_DEFAULT*/)
{
	return setParameter(GetUniformByID(index), data, size);
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector2* data)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program && data != 0)
		{
			auto uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), 1);
				
				ret = true;
			}
		}
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector3* data)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program && data != 0)
		{
			auto uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith3fv(uniform->location, (const GLfloat*)(data), 1);
				
				ret = true;
			}
		}
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector4* data)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program && data != 0)
		{
			auto uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith4fv(uniform->location, (const GLfloat*)(data), 1);
				
				ret = true;
			}
		}
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::setBool(eParameterHandles index, BOOL bBoolean)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith1i(uniform->location, bBoolean ? 1 : 0);
				
				ret = true;
			}
		}
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::setInt(eParameterHandles index, int nValue)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith1i(uniform->location, nValue);
				
				ret = true;
			}
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::setFloat(eParameterHandles index, float fValue)
{
	bool ret = false;
	for (auto tech_index = 0; tech_index < (int)mTechniques.size(); ++tech_index)
	{
		auto program = GetGLProgram(tech_index, m_nActivePassIndex);
		if (program)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith1f(uniform->location, fValue);
				
				ret = true;
			}
		}
	}
	return true;
}

bool ParaEngine::CEffectFileOpenGL::isParameterUsed(eParameterHandles index)
{
	return GetUniformByID(index) != 0;
}

bool ParaEngine::CEffectFileOpenGL::initWithByteArrays(const char* vShaderByteArray, const char* fShaderByteArray, int nTech, int nPass)
{
	releaseEffect(nTech, nPass);
	auto program = GetGLProgram(nTech, nPass, true);
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

bool ParaEngine::CEffectFileOpenGL::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, int nTech, int nPass)
{
	releaseEffect(nTech, nPass);
	auto program = GetGLProgram(nTech, nPass, true);
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

GLProgram* ParaEngine::CEffectFileOpenGL::GetGLProgram(int nTech, int nPass, bool bCreateIfNotExist)
{
	if ((int)mTechniques.size() <= nTech)
	{
		mTechniques.resize(nTech + 1);
	}
	auto & passes = mTechniques[nTech].mPasses;
	if ((int)passes.size() <= nPass && bCreateIfNotExist)
	{
		passes.resize(nPass + 1, NULL);
	}
	if ((int)passes.size() > nPass)
	{
		auto program = passes[nPass];
		if (program == NULL && bCreateIfNotExist)
		{
			program = new GLProgram();
			passes[nPass] = program;
		}
		return program;
	}
	return NULL;
}

bool ParaEngine::CEffectFileOpenGL::link(int nTech, int nPass)
{
	auto program = GetGLProgram(nTech, nPass);
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

bool ParaEngine::CEffectFileOpenGL::use(int nTech, int nPass)
{
	if (-1 == nTech)
		nTech = mTechniqueIndex;
	if (-1 == nPass)
		nPass = m_nActivePassIndex;
	auto program = GetGLProgram(nTech, nPass);
	if (program)
	{
		program->use();
		
		if (mTechniqueIndex != nTech)
		{
			mTechniqueIndex = nTech;
		}
		if (m_nActivePassIndex != nPass)
		{
			m_nActivePassIndex = nPass;
			// TODO: copy all uniforms to the new pass.
		}
		return true;
	}
	return false;
}

void ParaEngine::CEffectFileOpenGL::updateUniforms(int nTech, int nPass)
{
	if (nTech < 0)
		nTech = mTechniqueIndex;
	if (nPass < 0)
		nPass = m_nActivePassIndex;
	auto program = GetGLProgram(nTech, nPass);
	if (program)
	{
		program->updateUniforms();
		auto errorCode = glGetError();
		if (errorCode)
		{
			OUTPUT_LOG("unknown opengl error: 0x%04X before updateUniforms: %s pass: %d\n", errorCode, m_filename.c_str(), nPass);
		}
		// 
	}
}

Uniform* ParaEngine::CEffectFileOpenGL::GetUniformByID(eParameterHandles id)
{
	return GetUniform(m_ID2Names[id]);
}

Uniform* ParaEngine::CEffectFileOpenGL::GetUniform(const std::string& sName)
{
	auto program = GetGLProgram(mTechniqueIndex, m_nActivePassIndex);
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
	auto program = GetGLProgram(mTechniqueIndex, m_nActivePassIndex);
	if (program != 0)
	{
		if ((mTechniques.size() == 1)&&(mTechniques[0].mPasses.size() == 1))
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
	return use(mTechniqueIndex, nPass);
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
	for (auto & tech : mTechniques)
	{
		for (auto program : tech.mPasses)
		{
			if (program)
				program->reset();
			SAFE_RELEASE(program);
		}
		tech.mPasses.clear();
	}
	mTechniques.clear();
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
		AddParamChange(m_ID2Names[k_bAlphaBlending], bAlphaBlending);
		// setBool(k_bAlphaBlending, bAlphaBlending);
	}
}

void ParaEngine::CEffectFileOpenGL::EnableAlphaTesting(bool bAlphaTesting)
{
	if (isParameterUsed(k_bAlphaTesting))
	{
		AddParamChange(m_ID2Names[k_bAlphaTesting], bAlphaTesting);
		// setBool(k_bAlphaTesting, bAlphaTesting);
	}
}

int ParaEngine::CEffectFileOpenGL::totalPasses() const
{
	if (mTechniqueIndex<(int)mTechniques.size())
		return (int)mTechniques[mTechniqueIndex].mPasses.size();
	else
		return 0;
}

bool ParaEngine::CEffectFileOpenGL::setTexture(int index, TextureEntity* data)
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

bool ParaEngine::CEffectFileOpenGL::setTexture(int index, DeviceTexturePtr_type pTex)
{
	return CGlobals::GetRenderDevice()->SetTexture(index, pTex);
}

HRESULT ParaEngine::CEffectFileOpenGL::SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 nBytes)
{
	auto uniform = GetUniform(hParameter);
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


bool ParaEngine::CEffectFileOpenGL::SetFirstValidTechniqueByCategory(TechniqueCategory nCat)
{
	if (mTechniqueIndex >= (int)mTechniques.size())
		return false;
	if (mTechniques[mTechniqueIndex].nCategory == nCat)
		return true;
	vector<TechniqueDescGL>::const_iterator itCur, itEnd = mTechniques.end();
	int i = 0;
	for (itCur = mTechniques.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if ((*itCur).nCategory == nCat)
		{
			mTechniqueIndex = i;
			return true;
		}
	}
	return false;
}

bool ParaEngine::CEffectFileOpenGL::SetTechniqueByIndex(int nIndex)
{
	if (mTechniqueIndex == nIndex)
		return true;
	else if ((int)mTechniques.size()>nIndex)
	{
		mTechniqueIndex = nIndex;
		return true;
	}
	else
		return false;
}

const ParaEngine::CEffectFileOpenGL::TechniqueDesc* ParaEngine::CEffectFileOpenGL::GetCurrentTechniqueDesc()
{
	static TechniqueDesc s_tech;
	if (mTechniqueIndex<(int)mTechniques.size())
		return &(mTechniques[mTechniqueIndex]);
	else
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

void ParaEngine::CEffectFileOpenGL::SetShadowMapSize(int nsize)
{
	if (isParameterUsed(k_nShadowmapSize))
	{
		setInt(k_nShadowmapSize, nsize);
	}
}

bool ParaEngine::CEffectFileOpenGL::MappingEffectUniforms()
{
	if (!m_Effect) return false;

	static std::unordered_map<std::string, uint32> table;
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
		table["posScaleOffset"] = k_posScaleOffset;
		table["uvScaleOffset"] = k_uvScaleOffset;
		table["flareColor"] = k_lensFlareColor;
		table["fogparameters"] = k_fogParameters;
		table["fogColor"] = k_fogColor;
		table["shadowfactor"] = k_shadowFactor;
		table["LightStrength"] = k_LightStrength;
		table["shadowfactor"] = k_shadowFactor;
		table["LightStrength"] = k_LightStrength;
		table["LightColors"] = k_LightColors;
		table["LightPositions"] = k_LightPositions;
		table["FresnelR0"] = k_fresnelR0;
		table["ConstVector0"] = k_ConstVector0;
		table["ConstVector1"] = k_ConstVector1;
		table["ConstVector2"] = k_ConstVector2;
		table["ConstVector3"] = k_ConstVector3;
		table["sunvector"] = k_sunVector;
		table["suncolor"] = k_sunColor;
		table["worldcamerapos"] = k_cameraPos;
		table["viewdistances"] = k_cameraDistances;
		table["worldviewvector"] = k_cameraFacing;
		table["ambientlight"] = k_ambientLight;
		table["sunlight_inscatter"] = k_sunlightInscatter;
		table["sunlight_extinction"] = k_sunlightExtinction;
		table["worldpos"] = k_worldPos;
		table["texCoordOffset"] = k_texCoordOffset;
		table["curnumbones"] = k_boneInfluenceCount;
		table["fogenable"] = k_fogEnable;
		table["alphatesting"] = k_bAlphaTesting;

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
		table["LightParams"] = k_LightParams;


	}

	m_ID2Names.clear();
	auto uniforms = m_Effect->getUniforms();
	static char numerals[] = { '0','1','2','3','4','5','6','7','8','9' };
	for (auto uniform : uniforms)
	{
		std::string sec = uniform->getSemantic();
		std::string name = uniform->getName();
		std::string type = uniform->getType();
		auto it = table.find(sec);
		if (it != table.end()) {
			uint32 id = it->second;
			m_ID2Names[id] = name;
		}
		else if (type == "sampler2D" || type == "sampler3D" || type == "SamplerCube") {
			int iPos = (int)name.find_first_of(numerals, 0, sizeof(numerals));

			if (iPos != string::npos)
			{
				int iTexture = atoi(&name[iPos]);
				if (iTexture >= 0 && iTexture < (k_tex_max - k_tex0))
				{
					table[name] = k_tex0 + iTexture;
				}
			}

		}
		else {
			std::cout << std::endl << "can't parse uniform " << name << ", unkonw semantic " << sec << std::endl;
		}
	}

	return true;

}

static const std::string StringToLower(const std::string& str)
{
	std::string data = str;
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}


bool ParaEngine::CEffectFileOpenGL::GeneratePasses()
{
	if (m_Effect == NULL)return false;
	auto techniques = m_Effect->getTechiques();
	if (techniques.empty()) {
		std::cout << std::endl << "no techinique" << std::endl;
		return false;
	}
	auto tec = techniques[0];
	auto passes = tec->getPasses();
	if (passes.empty())
	{
		std::cout << std::endl << "no pass" << std::endl;
		return false;
	}

	for (int nPass = 0; nPass < passes.size(); nPass++)
	{
		auto pass = passes[nPass];
		auto states = pass->getStateAssignments();
		std::string vs_codeblock_name = "";
		std::string ps_codeblock_name = "";
		for (auto state : states)
		{
			const std::string lowerName = StringToLower(state->getName());
			if (lowerName == "vertexshader" || lowerName == "pixelshader")
			{
				auto value = state->getValue();
				if (value->getValueType() == StateValueType::COMPILE)
				{
					auto compileValue = static_cast<const StateCompileValue*>(value);
					if (lowerName == "vertexshader")
					{
						vs_codeblock_name = compileValue->getEntryPoint();
					}
					if (lowerName == "pixelshader")
					{
						ps_codeblock_name = compileValue->getEntryPoint();
					}
					if (vs_codeblock_name != "" && ps_codeblock_name != "")
					{
						break;
					}
				}
			}
		}

		if (vs_codeblock_name == "")
		{
			std::cout << std::endl << "Vertex Shader Codeblock name can't be empty" << std::endl;
			return false;
		}

		if (ps_codeblock_name == "")
		{
			std::cout << std::endl << "Pixel Shader Codeblock name can't be empty" << std::endl;
			return false;
		}


		// find code block
		std::string vscode = "";
		std::string pscode = "";
		auto codeblocks = m_Effect->getCodeBlocks();
		for (auto codeblock : codeblocks)
		{
			if (codeblock->getName() == vs_codeblock_name)
			{
				vscode = codeblock->getCode();
			}
			if (codeblock->getName() == ps_codeblock_name)
			{
				pscode = codeblock->getCode();
			}
			if (vscode != "" && pscode != "") break;
		}

		if (vscode == "")
		{
			std::cout << std::endl << "can't find vertex shader codeblock. " << vs_codeblock_name << std::endl;
			return false;
		}
		if (pscode == "")
		{
			std::cout << std::endl << "can't find pixel shader codeblock. " << ps_codeblock_name << std::endl;
			return false;
		}
		std::cout << std::endl << "Compile Pass " << nPass << std::endl;        // compile
		if (initWithByteArrays(vscode.c_str(), pscode.c_str(), nPass))
		{
			if (link(nPass))
			{
				updateUniforms(nPass);
			}
			else
			{
				std::cerr << "[" << m_filename << "] link pass " << nPass << " failed!" << std::endl;
				return false;
			}
		}
		else
		{
			std::cerr << "[" << m_filename << "] compile pass " << nPass << " failed!" << std::endl;
			return false;
		}


	}


	return true;

}

#endif
