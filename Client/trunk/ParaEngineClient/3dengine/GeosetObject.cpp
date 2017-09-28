#include "ParaEngine.h"
#include "AnimInstanceBase.h"
#include "ParaWorldAsset.h"
#include "ViewCullingObject.h"
#include "DummyAnimInstance.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "PhysicsWorld.h"
#include "IEnvironmentSim.h"
#include "terrain/GlobalTerrain.h"
#include "AIModuleNPC.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockCommon.h"
#include "OceanManager.h"
#ifdef USE_DIRECTX_RENDERER
#include "BVHSerializer.h"
#endif
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/BoneAnimProvider.h"
#include "CustomCharModelInstance.h"
#include "ParaXAnimInstance.h"
#include "BipedStateManager.h"
#include "BipedObject.h"
#include "ShapeOBB.h"
#include "ShapeAABB.h"
#include "PhysicsWorld.h"
#include "GeosetObject.h"
#include <algorithm>
using namespace ParaEngine;

ParaEngine::CGeosetObject::CGeosetObject()
	:mParent(nullptr)
{}

ParaEngine::CGeosetObject::~CGeosetObject()
{}

void ParaEngine::CGeosetObject::SetAssetFileName(const std::string & sFilename)
{
	attachEntity(CGlobals::GetAssetManager()->LoadParaX("",sFilename));
}

HRESULT ParaEngine::CGeosetObject::Draw(SceneState * sceneState)
{
	if(!ViewTouch()||GetOpacity()==0.f)
	{
		return E_FAIL;
	}
	EffectManager* pEffectManager=CGlobals::GetEffectManager();

	if(!pEffectManager->IsCurrentEffectValid()||mParent->GetPrimaryTechniqueHandle() < 0)
	{
		return E_FAIL;
	}

	int nLastEffectHandle=pEffectManager->GetCurrentTechHandle();
	if(nLastEffectHandle!=mParent->GetPrimaryTechniqueHandle())
	{
		pEffectManager->BeginEffect(mParent->GetPrimaryTechniqueHandle(),&(sceneState->m_pCurrentEffect));
	}

	// for models with particle systems
	sceneState->SetCurrentSceneObject(this);

	SetFrameNumber(sceneState->m_nRenderCount);

	// call Draw() of biped animation instance
	CAnimInstanceBase* pAI=mParent->GetAnimInstance();
	if(pAI)
	{
		pEffectManager->applyObjectLocalLighting(this);

		// draw the model
		Matrix4 mxWorld;
		Matrix4* mat=mParent->GetRenderMatrix(mxWorld,sceneState->GetRenderFrameCount());

		if(mParent->GetPrimaryTechniqueHandle()==1102&&!sceneState->IsShadowPass())
		{
#ifdef USE_DIRECTX_RENDERER
			CEffectFile* pEffectFile=pEffectManager->GetCurrentEffectFile();
			if(pEffectFile!=NULL)
			{
				RenderDevicePtr pd3dDevice=sceneState->m_pd3dDevice;
				pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE,0);
				_draw(sceneState,mat);

				pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE,0x0000000F);
				pd3dDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
				_draw(sceneState,mat);

				return S_OK;
			}
#endif
		}

		if(mParent->GetSelectGroupIndex()>=0&&!sceneState->IsShadowPass())
		{
			if((sceneState->m_nCurRenderGroup & (RENDER_SELECTION))==0)
			{
				// we will not draw selected objects during normal render pass, instead we render it during selection render pass. 
				// selection render pass is usually called after opache meshes are rendered. 
				return S_OK;
			}
#ifdef USE_DIRECTX_RENDERER
			// for selection render effect
			CEffectFile* pEffectFile=pEffectManager->GetCurrentEffectFile();
			if(pEffectFile!=0)
			{
				int nSelectionEffectStyle=mParent->GetSelectionEffect();
				if(nSelectionEffectStyle==RenderSelectionStyle_border)
				{
					// Let us render the border using the same technique as rendering text shadow.
					// i.e. render 4 times shifting 2 pixels around the border. 
					pEffectManager->BeginEffect(TECH_SIMPLE_MESH_NORMAL_BORDER,&(sceneState->m_pCurrentEffect));
					pEffectFile=pEffectManager->GetCurrentEffectFile();

					RenderDevicePtr pd3dDevice=sceneState->m_pd3dDevice;
					pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

					if(pEffectFile!=0)
					{
						// the border width in meters. 
						const float border_width=0.03f;

						for(int x=-1; x<=1; x+=2)
						{
							for(int y=-1; y<=1; y+=2)
							{
								Vector3 vOffsets(x*border_width,y*border_width,0.f);
								pEffectFile->GetDXEffect()->SetRawValue("g_offsets",&vOffsets,0,sizeof(Vector3));
								_draw(sceneState,mat);
							}
						}
					}

					pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);

					// change back to primary technique
					pEffectManager->BeginEffect(mParent->GetPrimaryTechniqueHandle(),&(sceneState->m_pCurrentEffect));
				}
				else if(nSelectionEffectStyle==RenderSelectionStyle_unlit)
				{
					bool bIsRendered=false;
					pEffectManager->BeginEffect(TECH_SIMPLE_MESH_NORMAL_UNLIT,&(sceneState->m_pCurrentEffect));
					pEffectFile=pEffectManager->GetCurrentEffectFile();
					if(pEffectFile!=0)
					{
						Vector3 vColorAdd(0.2f,0.2f,0.2f);
						pEffectFile->GetDXEffect()->SetRawValue("g_color_add",&vColorAdd,0,sizeof(Vector3));

						_draw(sceneState,mat);
						bIsRendered=true;
					}
					// change back to primary technique
					pEffectManager->BeginEffect(mParent->GetPrimaryTechniqueHandle(),&(sceneState->m_pCurrentEffect));
					if(bIsRendered)
						return S_OK;
				}
			}
			else
			{
				// TODO: find a way to render a single color with offsets with fixed function. 

				// change back to primary technique
				pEffectManager->BeginEffect(mParent->GetPrimaryTechniqueHandle(),&(sceneState->m_pCurrentEffect));
			}
#endif
		}

		bool bUsePointTextureFilter=false;

		// apply block space lighting for object whose size is comparable to a single block size
		if(mParent->CheckAttribute(MESH_USE_LIGHT)&&!(sceneState->IsShadowPass()))
		{
			BlockWorldClient* pBlockWorldClient=BlockWorldClient::GetInstance();
			if(pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
			{
				Vector3 vPos=GetPosition();
				vPos.y+=0.1f;
				Uint16x3 blockId_ws(0,0,0);
				BlockCommon::ConvertToBlockIndex(vPos.x,vPos.y,vPos.z,blockId_ws.x,blockId_ws.y,blockId_ws.z);
				DWORD dwPositionHash=blockId_ws.GetHashCode();
				uint8_t brightness[2];
				pBlockWorldClient->GetBlockMeshBrightness(blockId_ws,brightness,2);
				// block light
				float fBlockLightness=Math::Max(pBlockWorldClient->GetLightBrightnessLinearFloat(brightness[0]),0.1f);
				sceneState->GetCurrentLightStrength().y=fBlockLightness;
				// sun light
				float fSunLightness=Math::Max(pBlockWorldClient->GetLightBrightnessLinearFloat(brightness[1]),0.1f);
				sceneState->GetCurrentLightStrength().x=fSunLightness;

				float fLightness=Math::Max(fBlockLightness,fSunLightness*pBlockWorldClient->GetSunIntensity());
				//if(m_fLastBlockLight!=fLightness)
				//{
				//	float fMaxStep=(float)(sceneState->dTimeDelta*0.5f);
				//	if(dwPositionHash==m_dwLastBlockHash||m_dwLastBlockHash==0)
				//		m_fLastBlockLight=fLightness;
				//	else
				//		Math::SmoothMoveFloat1(m_fLastBlockLight,fLightness,fMaxStep);

				//	fLightness=m_fLastBlockLight;
				//}
				//else
				//{
				//	m_dwLastBlockHash=dwPositionHash;
				//}
				sceneState->GetLocalMaterial().Ambient=(LinearColor(fLightness*0.7f,fLightness*0.7f,fLightness*0.7f,1.f));
				sceneState->GetLocalMaterial().Diffuse=(LinearColor(fLightness*0.4f,fLightness*0.4f,fLightness*0.4f,1.f));

				sceneState->EnableLocalMaterial(true);

				bUsePointTextureFilter=bUsePointTextureFilter||pBlockWorldClient->GetUsePointTextureFiltering();
			}
		}

		if(bUsePointTextureFilter)
		{
			pEffectManager->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
			pEffectManager->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
		}
		else
		{
			pEffectManager->SetSamplerState(0,D3DSAMP_MINFILTER,pEffectManager->GetDefaultSamplerState(0,D3DSAMP_MINFILTER));
			pEffectManager->SetSamplerState(0,D3DSAMP_MAGFILTER,pEffectManager->GetDefaultSamplerState(0,D3DSAMP_MAGFILTER));
		}

		CApplyObjectLevelParamBlock p(mParent->GetEffectParamBlock());
		_draw(sceneState,mat,p.GetParamsBlock());

		sceneState->EnableLocalMaterial(false);
	}

	return S_OK;
}

void ParaEngine::CGeosetObject::_draw(SceneState * sceneState,Matrix4 * mxWorld,CParameterBlock* params)
{
	auto* pAI=static_cast<CParaXAnimInstance*>(mParent->GetAnimInstance());
	Matrix4 mat;
	if(pAI->UpdateWorldTransform(sceneState,mat,*mxWorld))
	{
		CGlobals::GetWorldMatrixStack().push(mat);
		// draw model
		RenderDevicePtr  pd3dDevice=CGlobals::GetRenderDevice();

		float fCameraToObjectDist=sceneState->GetCameraToCurObjectDistance();
		// scale the model
		Matrix4 mat2;
		mat2=CGlobals::GetWorldMatrixStack().SafeGetTop();
		CGlobals::GetWorldMatrixStack().push(mat2);

		// draw this model
		int nIndex=(sceneState && sceneState->IsLODEnabled())?mEntity->GetLodIndex(fCameraToObjectDist):0;
		CParaXModel* pModel=mEntity->GetModel(nIndex);

		if(pModel)
		{
			pModel->m_CurrentAnim=pAI->m_CurrentAnim;
			pModel->m_NextAnim=pAI->m_NextAnim;
			pModel->m_BlendingAnim=pAI->m_BlendingAnim;
			pModel->blendingFactor=pAI->m_fBlendingFactor;
			pModel->animate(sceneState,nullptr,pAI);

			std::map<string,CParameter*> save_params;
			auto my_params=GetEffectParamBlock();
			if(params&&my_params)
			{
				for(auto iter=my_params->BeginIter();iter!=my_params->EndIter();++iter)
				{
					auto const & name=iter->first;
					auto const & param=iter->second;
					if(params->GetParameter(name))
					{
						save_params[name]=new CParameter(*params->GetParameter(name));
					}
					else
					{
						save_params[name]=nullptr;
					}
					params->SetParameter(name,param);
				}
			}
			pModel->draw(sceneState,params?params:my_params);
			if(params&&my_params)
			{
				for(auto iter=save_params.begin();save_params.end()!=iter;++iter)
				{
					auto const & name=iter->first;
					auto const & param=iter->second;
					if(param)
					{
						params->SetParameter(name,*param);
						delete param;
					}
					else
						params->m_params.erase(name);
				}
			}
		}


		// pop matrix
		CGlobals::GetWorldMatrixStack().pop();
		CGlobals::GetWorldMatrixStack().pop();
	}
}
