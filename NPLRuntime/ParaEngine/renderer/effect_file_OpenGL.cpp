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

using namespace ParaEngine;


ParaEngine::CEffectFileOpenGL::CEffectFileOpenGL(const char* filename)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_bIsBeginPass(false), m_pendingChangesCount(0)
	, m_nTechniqueIndex(0)
	,m_Effect(nullptr)
{
	SetFileName(filename);
	Init();
}

ParaEngine::CEffectFileOpenGL::CEffectFileOpenGL(const AssetKey& key)
	: m_nActivePassIndex(0), m_bIsBegin(false), m_pendingChangesCount(0)
	, m_nTechniqueIndex(0)
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
	m_Effect = new DxEffectsTree();
	DxEffectsParser::Driver parseDriver(*m_Effect);
	bool ret = parseDriver.parse_string(shader_str,GetFileName());
	if (!ret)
	{
		delete m_Effect;
		m_Effect = nullptr;
		OUTPUT_LOG("error: parse effect failed %s\n", GetFileName().c_str());
		return false;
	}



	// Init effect
	ret = GeneratePasses();
	if (ret) {
		OUTPUT_LOG("[%s] Generate passes succeeded.\n", GetFileName().c_str());
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
		for (auto tech_index = 0; tech_index < (int)m_techniques.size(); ++tech_index)
		{
			auto & passes = m_techniques[tech_index].m_passes;
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
	else if ((int)m_techniques.size()>nTech)
	{
		auto & passes = m_techniques[nTech].m_passes;
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

bool ParaEngine::CEffectFileOpenGL::SetProgramParams(ParaEngine::CEffectFileOpenGL::ProgramCallbackFunction_t func)
{
	bool result = false;
	if (m_bIsBegin && m_bIsBeginPass && m_nActivePassIndex >= 0 && m_nTechniqueIndex>=0)
	{
		// set only current technique's current pass
		auto program = GetGLProgram(m_nTechniqueIndex, m_nActivePassIndex);
		if (program) {
			result = func(program);
		}
	}
	else if (m_bIsBegin)
	{
		// set current technique's all passes
		if ((int)m_techniques.size() > m_nTechniqueIndex && m_nTechniqueIndex>=0)
		{
			int nCount = m_techniques[m_nTechniqueIndex].m_passes.size();
			int nOldPass = m_nActivePassIndex;
			for (int i=0;i<nCount; ++i)
			{
				auto program = GetGLProgram(m_nTechniqueIndex, i);
				if (program) {
					m_nActivePassIndex = i;
					result = func(program) || result;
				}
			}
			m_nActivePassIndex = nOldPass;
		}
	}
	else
	{
		// set all techniques and all passes
		int nOldPass = m_nActivePassIndex;
		int nOldTech = m_nTechniqueIndex;
		for (auto tech_index = 0; tech_index < (int)m_techniques.size(); ++tech_index)
		{
			m_nTechniqueIndex = tech_index;
			int nCount = m_techniques[tech_index].m_passes.size();
			for (int i = 0; i < nCount; ++i)
			{
				auto program = GetGLProgram(tech_index, i);
				if (program) {
					m_nActivePassIndex = i;
					result = func(program) || result;
				}
			}
		}
		m_nActivePassIndex = nOldPass;
		m_nTechniqueIndex = nOldTech;
	}
	return result;
}

bool ParaEngine::CEffectFileOpenGL::setMatrix(eParameterHandles index, const Matrix4* data)
{
	return SetProgramParams([&](GLProgram* program){
		if (program && data != 0)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				// transpose it, since openGL and directX packed matrix differently.
				// Xizhi 2014.9.16: for some reason, it does not need to be transposed, opengl already packed data in our way.
				// Matrix4 matTranposed = data->transpose();
				program->setUniformLocationWithMatrix4fv(uniform->location, (const GLfloat*)(data), 1);

				return true;
			}
		}
		return false;
	});
}

bool ParaEngine::CEffectFileOpenGL::isMatrixUsed(eParameterHandles index)
{
	return isParameterUsed(index);
}

CParameterBlock* ParaEngine::CEffectFileOpenGL::GetParamBlock(bool bCreateIfNotExist /*= false*/)
{
	return &m_SharedParamBlock;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(Uniform* uniform, const void* data, int32 size)
{
	bool ret = false;
	auto program = GetGLProgram(m_nTechniqueIndex, m_nActivePassIndex);
	if (uniform && program && data != 0)
	{
		if (uniform->type == GL_INT)
		{
			program->setUniformLocationWith1i(uniform->location, *((const GLint*)(data)));

			PE_CHECK_GL_ERROR_DEBUG();
		}
		else if (uniform->type == GL_BOOL)
		{
			program->setUniformLocationWith1i(uniform->location, *((const bool*)(data)));
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else if (uniform->type == GL_FLOAT)
		{
			program->setUniformLocationWith1f(uniform->location, *((const GLfloat*)(data)));
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else if (uniform->type == GL_FLOAT_VEC3)
		{
			program->setUniformLocationWith3fv(uniform->location, (const GLfloat*)(data), uniform->size);
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else if (uniform->type == GL_FLOAT_VEC2)
		{
			program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), uniform->size);
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else if (uniform->type == GL_FLOAT_VEC4)
		{
			program->setUniformLocationWith4fv(uniform->location, (const GLfloat*)(data), uniform->size);
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else if (uniform->type == GL_FLOAT_MAT4)
		{
			program->setUniformLocationWithMatrix4fv(uniform->location, (const GLfloat*)(data), uniform->size);
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else if (size > 0)
		{
			program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), (uint32)((size + 1) / 2));
			PE_CHECK_GL_ERROR_DEBUG();
				
		}
		else
		{
			OUTPUT_LOG("warn: unknown uniform size and type\n");
		}
		// 
		ret = true;
	}
	return ret;
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const void* data, int32 size /*= D3DX_DEFAULT*/)
{
	return SetProgramParams([&](GLProgram* program) {
		return setParameter(GetUniformByID(index), data, size);
	});
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector2* data)
{
	return SetProgramParams([&](GLProgram* program) {
		if (program && data != 0)
		{
			auto uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith2fv(uniform->location, (const GLfloat*)(data), 1);
				
				return true;
			}
		}
		return false;
	});
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector3* data)
{
	return SetProgramParams([&](GLProgram* program) {
		if (program && data != 0)
		{
			auto uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith3fv(uniform->location, (const GLfloat*)(data), 1);
				
				return true;
			}
		}
		return false;
	});
}

bool ParaEngine::CEffectFileOpenGL::setParameter(eParameterHandles index, const Vector4* data)
{
	return SetProgramParams([&](GLProgram* program) {
		if (program && data != 0)
		{
			auto uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith4fv(uniform->location, (const GLfloat*)(data), 1);
				
				return true;
			}
		}
		return false;
	});
}

bool ParaEngine::CEffectFileOpenGL::setBool(eParameterHandles index, BOOL bBoolean)
{
	return SetProgramParams([&](GLProgram* program) {
		if (program)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith1i(uniform->location, bBoolean ? 1 : 0);
				
				return true;
			}
		}
		return false;
	});
}

bool ParaEngine::CEffectFileOpenGL::setInt(eParameterHandles index, int nValue)
{
	return SetProgramParams([&](GLProgram* program) {
		if (program)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith1i(uniform->location, nValue);
				
				return true;
			}
		}
		return false;
	});
}

bool ParaEngine::CEffectFileOpenGL::setFloat(eParameterHandles index, float fValue)
{
	return SetProgramParams([&](GLProgram* program) {
		if (program)
		{
			Uniform* uniform = GetUniformByID(index);
			if (uniform)
			{
				program->setUniformLocationWith1f(uniform->location, fValue);
				return true;
			}
		}
		return false;
	});
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
	if ((int)m_techniques.size() <= nTech)
	{
		m_techniques.resize(nTech + 1);
	}
	auto & passes = m_techniques[nTech].m_passes;
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
		//program->bindAttribLocation("a_color2", GLProgram::VERTEX_ATTRIB_MAX + 0);
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
		nTech = m_nTechniqueIndex;
	if (-1 == nPass)
		nPass = m_nActivePassIndex;
	auto program = GetGLProgram(nTech, nPass);
	if (program)
	{
		program->use();

		PE_CHECK_GL_ERROR_DEBUG();
		
		if (m_nTechniqueIndex != nTech)
		{
			m_nTechniqueIndex = nTech;
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
		nTech = m_nTechniqueIndex;
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
	}
}

Uniform* ParaEngine::CEffectFileOpenGL::GetUniformByID(eParameterHandles id)
{
	return GetUniform(m_ID2Names[id]);
}

Uniform* ParaEngine::CEffectFileOpenGL::GetUniform(const std::string& sName)
{
	auto program = GetGLProgram(m_nTechniqueIndex, m_nActivePassIndex);
	if (program)
	{
		return program->getUniform(sName);
	}
	return NULL;
}

void ParaEngine::CEffectFileOpenGL::applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor)
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

void ParaEngine::CEffectFileOpenGL::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient /*= true*/)
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

void ParaEngine::CEffectFileOpenGL::applyCameraMatrices()
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
	//	return true;
	//});
}

void ParaEngine::CEffectFileOpenGL::applyWorldMatrices()
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

void ParaEngine::CEffectFileOpenGL::applyGlobalLightingData(CSunLight& sunlight)
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

bool ParaEngine::CEffectFileOpenGL::begin(bool bApplyParam /*= true*/, DWORD flag /*= 0*/, bool bForceBegin)
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();
	auto program = GetGLProgram(m_nTechniqueIndex, m_nActivePassIndex);
	if (program != 0)
	{
		if (bApplyParam)
		{
			//SetProgramParams([&](GLProgram* program) {
				// set the lighting parameters
				// from the global light manager
				applyGlobalLightingData(pScene->GetSunLight());

				// set the camera matrix
				applyCameraMatrices();
			//	return true;
			//});
		}

		m_bIsBegin = true;
		return true;
	}
	else
		return false;
}

bool ParaEngine::CEffectFileOpenGL::BeginPass(int nPass, bool bForceBegin /*= false*/)
{
	m_bIsBeginPass = true;
	return use(m_nTechniqueIndex, nPass);
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

	auto program = GetGLProgram(m_nTechniqueIndex, m_nActivePassIndex);
	if(program)
		program->commit();
}

void ParaEngine::CEffectFileOpenGL::EndPass(bool bForceEnd /*= false*/)
{
	m_bIsBeginPass = false;
}

void ParaEngine::CEffectFileOpenGL::end(bool bForceEnd /*= false*/)
{
	m_bIsBegin = false;
}

HRESULT ParaEngine::CEffectFileOpenGL::RendererRecreated()
{
	for (auto & tech : m_techniques)
	{
		for (auto program : tech.m_passes)
		{
			if (program)
				program->reset();
			SAFE_RELEASE(program);
		}
		tech.m_passes.clear();
	}
	m_techniques.clear();
	m_bIsInitialized = false;
	m_nTechniqueIndex = 0;
	m_nActivePassIndex = 0;
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
	if (m_nTechniqueIndex<(int)m_techniques.size())
		return (int)m_techniques[m_nTechniqueIndex].m_passes.size();
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
	if (m_nTechniqueIndex >= (int)m_techniques.size())
		return false;
	if (m_techniques[m_nTechniqueIndex].nCategory == nCat)
		return true;
	vector<TechniqueDescGL>::const_iterator itCur, itEnd = m_techniques.end();
	int i = 0;
	for (itCur = m_techniques.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if ((*itCur).nCategory == nCat)
		{
			m_nTechniqueIndex = i;
			return true;
		}
	}
	return false;
}

bool ParaEngine::CEffectFileOpenGL::SetTechniqueByIndex(int nIndex)
{
	if (m_nTechniqueIndex == nIndex)
		return true;
	else if ((int)m_techniques.size()>nIndex)
	{
		m_nTechniqueIndex = nIndex;
		return true;
	}
	else
		return false;
}

const ParaEngine::CEffectFileOpenGL::TechniqueDesc* ParaEngine::CEffectFileOpenGL::GetCurrentTechniqueDesc()
{
	static TechniqueDesc s_tech;
	if (m_nTechniqueIndex<(int)m_techniques.size())
		return &(m_techniques[m_nTechniqueIndex]);
	else
		return &s_tech;
}

bool ParaEngine::CEffectFileOpenGL::SetBoolean(int nIndex, bool value)
{
	PE_ASSERT(nIndex < (k_bBooleanMAX - k_bBoolean0));
	return SetProgramParams([&](GLProgram* program) {
		if (isParameterUsed((eParameterHandles)(k_bBoolean0 + nIndex)))
		{
			return setBool((eParameterHandles)(k_bBoolean0 + nIndex), value);
		}
		return false;
	});
}

void ParaEngine::CEffectFileOpenGL::SetShadowMapSize(int nsize)
{
	SetProgramParams([&](GLProgram* program) {
		if (isParameterUsed(k_nShadowmapSize))
		{
			setInt(k_nShadowmapSize, nsize);
		}
		return true;
	});
}

bool ParaEngine::CEffectFileOpenGL::MappingEffectUniforms(const std::vector<UniformInfo>& uniforms)
{
	if (!m_Effect) return false;

	static std::unordered_map<std::string, uint32> table;
	if(table.empty())
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
		table["LightParams"] = k_LightParams;

		table["materialBaseColor"] = k_material_base_color;
		table["materialMetallic"] = k_material_metallic;
		table["materialSpecular"] = k_material_specular;
		table["materialRoughness"] = k_material_roughness;
		table["materialEmissiveColor"] = k_material_emissive_color;
		table["materialEmissive"] = k_material_emissive;
		table["materialOpacity"] = k_material_opacity;
		table["materialNormalTexture"] = k_material_normal_texture;
		table["materialDiffuseTexture"] = k_material_diffuse_texture;
		table["materialUV"] = k_material_uv;
	}

	m_ID2Names.clear();
	static char numerals[] = { '0','1','2','3','4','5','6','7','8','9' };
	for (auto uniform : uniforms)
	{
		std::string sec = uniform.semantic;
		std::string name = uniform.name;
		std::string type = uniform.type;
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
			// OUTPUT_LOG("can't parse uniform %s , unknown semantic ", name.c_str(), sec.c_str());
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


static const char* EsTypeToString(EShType type)
{
	switch (type)
	{
	default:
		return "unkonw";
	case EShTypeVoid:
		return "void";
	case EShTypeBool:
		return "bool";
	case EShTypeBVec2:
		return "bvec2";
	case EShTypeBVec3:
		return "bvec3";
	case EShTypeBVec4:
		return "bvec4";
	case EShTypeInt:
		return "int";
	case EShTypeIVec2:
		return "ivec2";
	case EShTypeIVec3:
		return "ivec3";
	case EShTypeIVec4:
		return "vec4";
	case EShTypeFloat:
		return "float";
	case EShTypeVec2:
		return "vec2";
	case EShTypeVec3:
		return "vec3";
	case EShTypeVec4:
		return "vec4";
	case EShTypeMat2:
		return "mat2";
	case EShTypeMat2x3:
		return "mat2x3";
	case EShTypeMat2x4:
		return "mat2x4";
	case EShTypeMat3x2:
		return "mat3x2";
	case EShTypeMat3:
		return "mat3";
	case EShTypeMat3x4:
		return "mat3x4";
	case EShTypeMat4x2:
		return "mat4x2";
	case EShTypeMat4x3:
		return "mat4x3";
	case EShTypeMat4x4:
		return "mat4";
	case EShTypeSampler:
		return "sampler";
	case EShTypeSampler1D:
		return "sampler1D";
	case EShTypeSampler1DShadow:
		return "sampler1DShadow";
	case EShTypeSampler2D:
		return "sampler2D";
	case EShTypeSampler2DShadow:
		return "Sampler2DShadow";
	case EShTypeSampler3D:
		return "Sampler3D";
	case EShTypeSamplerCube:
		return "SamplerCube";
	case EShTypeSamplerRect:
		return "SamplerRect";
	case EShTypeSamplerRectShadow:
		return "SamplerRectShadow";
	case EShTypeSampler2DArray:
		return "Sampler2DArray";
	case EShTypeStruct:
		return "Struct";
	}
}


static void GetUniforms(ShHandle parser, std::vector<UniformInfo>& uniforms)
{
	int count = Hlsl2Glsl_GetUniformCount(parser);
	if (count > 0)
	{
		const ShUniformInfo* uni = Hlsl2Glsl_GetUniformInfo(parser);

		for (int i = 0; i < count; ++i)
		{
			UniformInfo info;
			info.name = uni[i].name;
			if (uni[i].semantic != nullptr) {
				info.semantic = uni[i].semantic;
			}
			info.type = EsTypeToString(uni[i].type);

			if (std::find(uniforms.begin(), uniforms.end(), info) == uniforms.end()) {
				uniforms.push_back(info);
			}
		}
	}
}


bool hlsl2glsl(const std::string& inCode, const std::string& enterpoint, EShLanguage toLang, ETargetVersion toVersion, std::string& outCode, std::vector<UniformInfo>& uniforms)
{

	Hlsl2Glsl_Initialize();

	ShHandle parser = Hlsl2Glsl_ConstructCompiler(toLang);

	auto linker = parser->GetLinker();
	linker->setUserAttribName(EAttribSemantic::EAttrSemPosition, "a_position");
	linker->setUserAttribName(EAttribSemantic::EAttrSemNormal, "a_normal");
	linker->setUserAttribName(EAttribSemantic::EAttrSemColor0, "a_color");
	linker->setUserAttribName(EAttribSemantic::EAttrSemColor1, "a_color2");
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex0, "a_texCoord");
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex1, "a_texCoord1");
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex2, "a_texCoord2");
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex3, "a_texCoord3");
	linker->setUserAttribName(EAttribSemantic::EAttrSemBlendWeight, "a_blendWeight");
	linker->setUserAttribName(EAttribSemantic::EAttrSemBlendIndices, "a_blendIndex");

	const char* sourceStr = inCode.c_str();
	const char* infoLog = nullptr;
	int opt = ETranslateOpBGRAVertexColor;
	int parseOk = Hlsl2Glsl_Parse(parser, sourceStr, toVersion, nullptr, opt);
	if (!parseOk) {
		infoLog = Hlsl2Glsl_GetInfoLog(parser);
		OUTPUT_LOG(infoLog);
		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return false;

	}
	int translateOk = Hlsl2Glsl_Translate(parser, enterpoint.c_str(), toVersion, opt);
	if (!translateOk) {
		infoLog = Hlsl2Glsl_GetInfoLog(parser);
		OUTPUT_LOG(infoLog);
		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return false;
	}
	outCode = Hlsl2Glsl_GetShader(parser);

	// remove #line and #version
	std::stringstream outss;
	std::istringstream iss(outCode);
	char buf[1024] = { 0 };
	while (iss.getline(buf, sizeof(buf))) {
		std::string line(buf);
		if (line.find("#line ") != std::string::npos ||
			line.find("#version ") != std::string::npos)
		{
			//skip
			continue;
		}
		outss << line << std::endl;
	}
	outCode = outss.str();

	GetUniforms(parser, uniforms);
	Hlsl2Glsl_DestructCompiler(parser);
	Hlsl2Glsl_Shutdown();
	return true;
}


bool ParaEngine::CEffectFileOpenGL::GeneratePasses()
{

	if (m_Effect == NULL)return false;
	auto techniques = m_Effect->getTechiques();
	if (techniques.empty()) {
		//std::cout << std::endl << "no techinique" << std::endl;
		OUTPUT_LOG("no techinique");
		return false;
	}

	for (int tecIndex = 0;tecIndex<techniques.size();tecIndex++)
	{
		auto tec = techniques[tecIndex];
		auto passes = tec->getPasses();
		if (passes.empty())
		{
			//std::cout << std::endl << "no pass" << std::endl;
			OUTPUT_LOG("no pass");
			return false;
		}

		std::vector<UniformInfo> uniforms;

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
				//std::cout << std::endl << "Vertex Shader Codeblock name can't be empty" << std::endl;
				OUTPUT_LOG("Vertex Shader Codeblock name can't be empty");
				return false;
			}

			if (ps_codeblock_name == "")
			{
				//std::cout << std::endl << "Pixel Shader Codeblock name can't be empty" << std::endl;
				OUTPUT_LOG("Pixel Shader Codeblock name can't be empty");
				return false;
			}


			// find code block
			std::string vscode = "";
			std::string pscode = "";
			auto codeblock = m_Effect->getCodeBlock();
			ETargetVersion targetVersion = ETargetGLSL_110;
#if PARAENGINE_MOBILE
			targetVersion = ETargetGLSL_ES_100;
#elif EMSCRIPTEN
			targetVersion = ETargetGLSL_ES_100;
#endif
			bool ret = hlsl2glsl(codeblock, vs_codeblock_name, EShLanguage::EShLangVertex, targetVersion, vscode, uniforms);
			if (!ret || vscode == "")
			{
				//std::cout << std::endl << "can't translate vertex shader " << vs_codeblock_name << "  shader:" << GetFileName() << std::endl;

				OUTPUT_LOG("can't translate vertex shader %s  shader: %s", vs_codeblock_name.c_str(), GetFileName().c_str());
				return false;
			}
			ret = hlsl2glsl(codeblock, ps_codeblock_name, EShLanguage::EShLangFragment, targetVersion, pscode, uniforms);
			if (!ret || pscode == "")
			{
				//std::cout << std::endl << "can't translate fragment shader " << ps_codeblock_name << "  shader:" << GetFileName() << std::endl;

				OUTPUT_LOG("can't translate fragment shader %s  shader: %s", ps_codeblock_name.c_str(), GetFileName().c_str());
				return false;
			}

			//std::cout << std::endl << "Compile Pass " << nPass << std::endl;        // compile
			OUTPUT_LOG("Compile Pass %d", nPass);

			//{
			//	auto size = vscode.size();
			//	size_t i = 0;
			//	while (size > 0)
			//	{
			//		auto len = (std::min)(size, (size_t)100);
			//		std::string s(vscode.c_str() + i, len);
			//		OUTPUT_LOG("%s", s.c_str());
			//		i += len;
			//		size -= len;
			//	}
			//}

			//{
			//	auto size = pscode.size();
			//	size_t i = 0;
			//	while (size > 0)
			//	{
			//		auto len = (std::min)(size, (size_t)100);
			//		std::string s(pscode.c_str() + i, len);
			//		OUTPUT_LOG("%s", s.c_str());
			//		i += len;
			//		size -= len;
			//	}
			//}

			if (initWithByteArrays(vscode.c_str(), pscode.c_str(), tecIndex,nPass))
			{
				if (link(tecIndex, nPass))
				{
					updateUniforms(tecIndex, nPass);
				}
				else
				{
					//std::cout << "[" << m_filename << "] link pass " << nPass << " failed!" << std::endl;
					OUTPUT_LOG("[%s] link pass %d failed", m_filename.c_str(), nPass);
					return false;
				}
			}
			else
			{
				//std::cout << "[" << m_filename << "] compile pass " << nPass << " failed!" << std::endl;
				OUTPUT_LOG("[%s] compile pass %d failed", m_filename.c_str(), nPass);
				return false;
			}
		}

		// Parse uniforms

		bool ret = MappingEffectUniforms(uniforms);
		if (!ret) {
			OUTPUT_LOG("[%s] Parse uniforms failed.\n", GetFileName().c_str());
		}
	}
	return true;
}

#endif
