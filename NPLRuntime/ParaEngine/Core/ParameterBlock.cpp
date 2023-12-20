//-----------------------------------------------------------------------------
// Class:	CParameterBlock
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.6.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef PARAENGINE_CLIENT
#include "DirectXEngine.h"
#include "ShadowMap.h"
#endif
#include "ParaWorldAsset.h"
#include "effect_file.h"
#include "EffectManager.h"
#include "ViewportManager.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "BlockEngine/BlockWorldClient.h"
#include "SunLight.h"

#include "ParameterBlock.h"

using namespace ParaEngine;
using namespace std;

namespace ParaEngine
{
	const string g_paramTypes[CParameter::PARAM_LAST] = {
		"unknown",
		"int",
		"float", 
		"boolean",
		"vector2",
		"vector3",
		"vector4",
		"matrix43",
		"rawbytes",
		"string",
		"texture",
	};
}

CParameter::operator bool() {
	return !!(*((BOOL*)GetRawData()));
}

bool CParameterBlock::AddParameter( const CParameter& p )
{
	map <string, CParameter>::iterator it = m_params.find(p.GetName());
	if(it!=m_params.end())
	{
		it->second = p;
	}
	else
	{
		m_params[p.GetName()] = p;
	}
	return true;
}

bool CParameterBlock::ApplyToEffect( CEffectFile* pEffectFile )
{
#if defined(USE_OPENGL_RENDERER) || defined(USE_DIRECTX_RENDERER)
	if(!IsEmpty() && pEffectFile!=0 && pEffectFile->IsValid())
	{
		ParamIterator itCur, itEnd = EndIter();
		for (itCur = BeginIter(); itCur!=itEnd; ++itCur)
		{
			CParameter& p = itCur->second;
			if(p.m_type == CParameter::PARAM_TEXTURE_ENTITY)
			{
#if defined(USE_OPENGL_RENDERER)
				// if it is texture parameter, the name must be numeric. 
				const char* name = p.GetName().c_str();
				TextureEntity* pTextureEntity = (TextureEntity*)p;
				if (pTextureEntity != NULL && pTextureEntity->GetTexture())
				{
					// use predefined texture
					int nIndex = (int)(name[0] - '0');
					((CEffectFileOpenGL*)pEffectFile)->setTexture(nIndex, pTextureEntity);
				}
#endif
#if defined(USE_DIRECTX_RENDERER)
				// if it is texture parameter, the name must be numeric. 
				const char* name = p.GetName().c_str();
				TextureEntity* pTextureEntity = (TextureEntity*)p;
				if(pTextureEntity!= NULL && pTextureEntity->GetTexture())
				{
					if(name[0]>='0' && name[0]<='9')
					{
						// use predefined texture
						int nIndex = (int)(name[0] - '0');
						pEffectFile->UnLockTexture(nIndex);
						//pEffectFile->setTexture(nIndex, pTextureEntity->GetTexture());
						/** 2008/1/10 by LXZ: since set texture is outside BeginPass() and EndPass(); we will set them by calling effect set texture, instead of d3d set texture. 

						2006/5/12
						- [ATI bug Workaround]:  ATI card texture problem for shaders may be caused by the following.
						sampler and texture states are not properly matched to GPU sampler register S#. 
						so for sampler states that will change by the SetSamplerState() call during the execution of shaders, one should explicitly specify sampler registers in HLSL. 
						Moreover, I have no idea who caused the following bug on ATI card (X1300,etc): the behavior of d3d SetTexture and that of the effect SetTexture are different.The effect SetTexture call 
						is totally unpredicatable on ATI cards. It seems to crash when setting multiple times between BeginPass() and EndPass(). 
						The effect's SetTexture can set the shader sampler texture even outside BeginPass() and EndPass() except for ATI card, and d3d SetTexture only works inside the BeginPass() and EndPass()
						for both Nvidia and ATI cards. In order to workaround the ATI bug, the problem is solved by using d3d SetTexture instead of effect SetTexture(which is preferred); moreover, I have to set ALL textures 
						between BeginPass() and EndPass() of any effect file, this may be inefficient since duplicated SetTexture calls may be called for the same sampler. 

						*/
						pEffectFile->GetDXEffect()->SetTexture(pEffectFile->GetTextureHandle(nIndex), pTextureEntity->GetTexture());
						pEffectFile->LockTexture(nIndex);
					}
					else
					{
						// use custom texture
						pEffectFile->GetDXEffect()->SetTexture(name, pTextureEntity->GetTexture());
					}
				}
#endif
			}
			else
			{
				pEffectFile->SetRawValue(p.GetName().c_str(), p.GetRawData(), 0, p.GetRawDataLength());
			}
		}
	}

#endif
	return true;
}

const string& ParaEngine::CParameter::GetTypeAsString()
{
	return g_paramTypes[m_type];
}

ParaEngine::CParameter::PARAMETER_TYPE ParaEngine::CParameter::SetTypeByString( const char* sType )
{
	if(sType== 0)
		return m_type;

	for (int i=0; i<PARAM_LAST; ++i)
	{
		if(g_paramTypes[i] == sType)
		{
			m_type = (PARAMETER_TYPE) i;
			break;
		}
	}
	return m_type;
}

bool ParaEngine::CParameter::SetValueByString( const char* sValue, int nType /*= -1*/ )
{
	if(sValue== 0)
		return false;
	if(nType>= 0 && nType < PARAM_LAST)
		m_type = (PARAMETER_TYPE) nType;
	switch(m_type)
	{
	case PARAM_INT:
		{
			int value;
			if(sscanf(sValue, "%d", &value)>0)
			{
				*this = value;
			}
			break;
		}
	case PARAM_BOOLEAN:
		{
			int value;
			if(sscanf(sValue, "%d", &value)>0)
			{
				*this = (bool)(value>0);
			}
			else
			{
				string sFalse = "false";
				*this = (bool)(sFalse != sValue);
			}
			break;
		}
	case PARAM_FLOAT:
		{
			float value;
			if(sscanf(sValue, "%f", &value)>0)
			{
				*this = value;
			}
			break;
		}
	case PARAM_VECTOR2:
		{
			Vector2 value;
			if(sscanf(sValue, "%f, %f", &value.x, &value.y)>1)
			{
				*this = value;
			}
			break;
		}
	case PARAM_VECTOR3:
		{
			Vector3 value;
			if(sscanf(sValue, "%f, %f, %f", &value.x, &value.y, &value.z)>2)
			{
				*this = value;
			}
			break;
		}
	case PARAM_VECTOR4:
		{
			Vector4 value;
			if(sscanf(sValue, "%f, %f, %f, %f", &value.x, &value.y, &value.z, &value.w)>3)
			{
				*this = value;
			}
			break;
		}
	case PARAM_MATRIX:
		{
			// TODO: need to convert from row to column matrix: LiXizhi. 2009.4.19
			Matrix4 mat; 
			if(sscanf(sValue,  "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23,&mat._31, &mat._32, &mat._33,&mat._41, &mat._42, &mat._43) >=12)
			{
				mat._14 = 0;
				mat._24 = 0;
				mat._34 = 0;
				mat._44 = 1;
			}
			else
			{
				mat = Matrix4::IDENTITY;
			}
			*this = *((Matrix4*)(&mat));
			break;
		}
	case PARAM_STRING:
		{
			*this = sValue;
			break;		
		}
	case PARAM_TEXTURE_ENTITY:
		{
			TextureEntity* pEntity = CGlobals::GetAssetManager()->LoadTexture("", sValue, TextureEntity::StaticTexture);
			*this = pEntity;
			break;
		}

	default:
		return false;
	}
	return true;
}

std::string ParaEngine::CParameter::GetValueByString()
{
	string sValue;

#define TMP_BUFFER_SIZE		500
	char tmp[TMP_BUFFER_SIZE];
	memset(tmp, 0, sizeof(tmp));

	switch(m_type)
	{
	case PARAM_INT:
		{
			int value = (int)(*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%d", value);
			sValue = tmp;
			break;
		}
	case PARAM_BOOLEAN:
		{
			bool value = (bool)(*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%d", value?1:0);
			sValue = tmp;
			break;
		}
	case PARAM_FLOAT:
		{
			float value = (float)(*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%f", value);
			sValue = tmp;
			break;
		}
	case PARAM_VECTOR2:
		{
			Vector2 value = (*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%f,%f", value.x, value.y);
			sValue = tmp;
			break;
		}
	case PARAM_VECTOR3:
		{
			Vector3 value = (*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%f,%f,%f", value.x, value.y,value.z);
			sValue = tmp;
			break;
		}
	case PARAM_VECTOR4:
		{
			Vector4 value = (*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%f,%f,%f,%f", value.x, value.y,value.z,value.w);
			sValue = tmp;
			break;
		}
	case PARAM_MATRIX:
		{
			Matrix4 mat = (Matrix4)(*this);
			snprintf(tmp, TMP_BUFFER_SIZE, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43);
			sValue = tmp;
			break;
		}
	case PARAM_STRING:
		{
			sValue = m_sStringValue;
			break;
		}
	case PARAM_TEXTURE_ENTITY:
		{
			TextureEntity* pEntity = (TextureEntity*)(*this);
			if(pEntity)
			{
				sValue = pEntity->GetKey();
			}
			break;
		}

	default:
		break;
	}
	return sValue;
}

const string & ParaEngine::CParameter::GetValueAsConstString()
{
	if (m_type == PARAM_STRING)
		return m_sStringValue;
	m_sStringValue = GetValueByString();
	return m_sStringValue;
}

void CParameterBlock::SetParamByStringValue(const char* sParamName, const char* sValue_)
{
#if defined(USE_OPENGL_RENDERER) || defined(USE_DIRECTX_RENDERER)
	const std::string sValue = sValue_;

	if (sValue.find("mat4") == 0)
	{
		Matrix4 mat;

		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());
		if (sValue == "mat4ProjectionInverse")
		{
			mat = pProj->inverse();
		}
		else if (sValue == "mat4Projection")
		{
			mat = *pProj;
		}
		else if (sValue == "mat4ModelView")
		{
			ParaMatrixMultiply(&mat, pWorld, pView);
		}
		else if (sValue == "mat4World")
		{
			mat = *pWorld;
		}
		else if (sValue == "mat4WorldInverse")
		{
			mat = pWorld->inverse();
		}
		else if (sValue == "mat4View")
		{
			mat = *pView;
		}
		else if (sValue == "mat4ViewInverse")
		{
			mat = pView->inverse();
		}
		else if (sValue == "mat4ModelViewInverse")
		{
			Matrix4 mat_;
			ParaMatrixMultiply(&mat_, pWorld, pView);
			mat = mat_.inverse();
		}
		// FIXME: maybe the next 2 matrix calculation here got something wrong!
		else if (sValue == "mat4WorldViewProjection")
		{
			Matrix4 ViewProj;
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			ParaMatrixMultiply(&mat, pWorld, pView);
		}
		else if (sValue == "mat4WorldViewProjectionInverse")
		{
			Matrix4 ViewProj;
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			Matrix4 mat_;
			ParaMatrixMultiply(&mat_, pWorld, pView);
			mat = mat_.inverse();
		}
#if defined(USE_DIRECTX_RENDERER)
		else if (sValue == "mat4ShadowMapTex")
		{
			ParaMatrixMultiply(&mat, pWorld, CGlobals::GetEffectManager()->GetTexViewProjMatrix());
		}
		else if (sValue == "mat4ShadowMapViewProj")
		{
			ParaMatrixMultiply(&mat, pWorld, CGlobals::GetEffectManager()->GetShadowMap()->GetViewProjMatrix());
		}
#endif
		else
		{
			mat = Matrix4::IDENTITY;
		}
		// it is column matrix, so transpose it. 
		mat = mat.transpose();
		SetParameter(sParamName, mat);
	}
	else if (sValue.find("vec3") != string::npos)
	{
		Vector3 v(0, 0, 0);
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		CBaseCamera* pCamera = pScene->GetCurrentCamera();
		if (sValue == "vec3cameraPosition")
		{
			v = pCamera->GetRenderEyePosition() - pScene->GetRenderOrigin();
		}
		else if (sValue == "vec3SunDirection")
		{
			v = -CGlobals::GetScene()->GetSunLight().GetSunDirection();
		}
		else if (sValue == "vec3SunColor")
		{
			LinearColor vColor = CGlobals::GetScene()->GetSunLight().GetSunColor();
			v.x = vColor.r;
			v.y = vColor.g;
			v.z = vColor.b;
		}
		else if (sValue == "vec3SunAmbient")
		{
			LinearColor vColor = CGlobals::GetScene()->GetSunLight().GetSunAmbient();
			v.x = vColor.r;
			v.y = vColor.g;
			v.z = vColor.b;
		}
		else if (sValue == "vec3BlockLightColor")
		{
			LinearColor vColor = BlockWorldClient::GetInstance()->GetBlockLightColor();
			v.x = vColor.r;
			v.y = vColor.g;
			v.z = vColor.b;
		}
		else if (sValue == "vec3FogColor")
		{
			LinearColor vColor = CGlobals::GetScene()->GetFogColor();
			v.x = vColor.r;
			v.y = vColor.g;
			v.z = vColor.b;
		}
		SetParameter(sParamName, v);
	}
	else if (sValue.find("vec2") != string::npos)
	{
		Vector2 v(0, 0);
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		CBaseCamera* pCamera = pScene->GetCurrentCamera();

		if (sValue == "vec2ViewportScale")
		{
			CGlobals::GetViewportManager()->GetActiveViewPort()->GetViewportTransform(&v, NULL);
		}
		else if (sValue == "vec2ViewportOffset")
		{
			CGlobals::GetViewportManager()->GetActiveViewPort()->GetViewportTransform(NULL, &v);
		}
		else if (sValue == "vec2ScreenSize")
		{
			v.x = (float)CGlobals::GetViewportManager()->GetWidth();
			v.y = (float)CGlobals::GetViewportManager()->GetHeight();
		}
#if defined(USE_DIRECTX_RENDERER)
		else if (sValue == "vec2ShadowMapSize")
		{
			v.x = (float)(CGlobals::GetEffectManager()->GetShadowMap()->GetShadowMapTexelSize());
			v.y = (float)(1 / v.x);
		}
#endif
		SetParameter(sParamName, v);
	}
	else if (sValue.find("float") != string::npos)
	{
		float v = 0.f;
		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		CBaseCamera* pCamera = pScene->GetCurrentCamera();
		if (sValue == "floatFOV")
		{
			v = pCamera->GetFieldOfView();
		}
		else if (sValue == "floatTanHalfFOV")
		{
			v = tanf(pCamera->GetFieldOfView() / 2);
		}
		else if (sValue == "floatViewAspect")
		{
			v = pCamera->GetAspectRatio();
		}
		else if (sValue == "floatCameraFarPlane")
		{
			v = pCamera->GetFarPlane();
		}
		else if (sValue == "floatCameraNearPlane")
		{
			v = pCamera->GetNearPlane();
		}
		else if (sValue == "floatShadowFactor")
		{
			v = CGlobals::GetScene()->GetSunLight().GetShadowFactor();
		}
		else if (sValue == "floatTimeOfDaySTD")
		{
			v = CGlobals::GetScene()->GetSunLight().GetTimeOfDaySTD();
		}
		else if (sValue == "floatShadowRadius")
		{
			v = CGlobals::GetScene()->GetShadowRadius();
		}

		SetParameter(sParamName, v);
	}
#endif
}

CApplyObjectLevelParamBlock::CApplyObjectLevelParamBlock(CParameterBlock* pBlock) :m_pBlock(pBlock)
{
	if (m_pBlock)
	{
		CParameter* pParam = NULL;
		pParam = m_pBlock->GetParameter("ztest");
		if (pParam)
		{
			bool bZTest = (bool)(*pParam);
			m_bLastZEnabled = CGlobals::GetEffectManager()->IsZTestEnabled();
			CGlobals::GetEffectManager()->EnableZTest(bZTest);
		}
	}
}

CApplyObjectLevelParamBlock::~CApplyObjectLevelParamBlock()
{
	if (m_pBlock)
	{
		CParameter* pParam = NULL;
		pParam = m_pBlock->GetParameter("ztest");
		if (pParam)
		{
			CGlobals::GetEffectManager()->EnableZTest(m_bLastZEnabled);
		}
	}
}

ParaEngine::CParameterBlock* CApplyObjectLevelParamBlock::GetParamsBlock()
{
	return m_pBlock;
}

