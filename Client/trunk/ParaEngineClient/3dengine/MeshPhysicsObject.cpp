//-----------------------------------------------------------------------------
// Class:	CMeshPhysicsObject
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "MeshObject.h"
#include "MeshEntity.h"
#include "PhysicsWorld.h"
#include "XRefObject.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "terrain/GlobalTerrain.h"
#include "DynamicAttributeField.h"
#include "util/StringHelper.h"

#include "MeshPhysicsObject.h"
#include "memdebug.h"
using namespace ParaEngine;

// use the fast version printf
#define My_snprinf StringHelper::fast_snprintf

namespace ParaEngine
{
	/** whether force export mesh physics */
	bool g_bForceExportMeshPhysics = false;
}
std::string CMeshPhysicsObject::ToString(DWORD nMethod)
{
#ifndef MAX_LINE
#define MAX_LINE	500
#endif
	/** -- sample script generated:
	asset = ParaAsset.LoadStaticMesh("", "sample/trees/tree1.x");
	player = ParaScene.CreateMeshPhysicsObject("", asset, 1,1,1, true, "0.193295,0,0,0,0.187032,-0.0488078,0,0.0488078,0.187032,0,0,0");
	player:SetPosition(148,120.156,95);player:SetFacing(0);sceneLoader:AddChild(player);
	*/
	string sScript;
	char line[MAX_LINE+1];
	MeshEntity * pModel = (m_pMeshObject)?m_pMeshObject->m_ppMesh.get():NULL;
	
	if(pModel)
	{
		Matrix4 mat;
		float fOBB_X, fOBB_Y, fOBB_Z, fFacing;
		m_pMeshObject->GetLocalTransform(&mat);
		
		// tricky: when saving mesh object, we will use the bounding box of its view clipping object if any. 
		if(pModel->GetPrimaryTechniqueHandle() > 0)
		{
			// the view clipping object is usually a sphere that best contains the transformed mesh object's obb. 
			GetViewClippingObject()->GetBoundingBox(&fOBB_X, &fOBB_Y, &fOBB_Z, &fFacing);
		}
		else
		{
			m_pMeshObject->GetBoundingBox(&fOBB_X, &fOBB_Y, &fOBB_Z, &fFacing);
		}
		fFacing = GetFacing();

		//////////////////////////////////////////////////////////////////////////
		// write creator
		if( (nMethod&CBaseObject::NPL_DONOT_OUTPUT_ASSET) ==0)
		{
			My_snprinf(line, MAX_LINE, "asset=ParaAsset.LoadStaticMesh(\"\",[[%s]]);\n", 
				pModel->GetKey().c_str());
			sScript.append(line);
		}
		
		bool bSaveName = false;

		/// currently I only save name if it begins with "g_".
		const string& sName = GetIdentifier();
		if(sName.size()>2 && sName[1] == '_' && sName[0] == 'g')
			bSaveName = true;

		if(bSaveName)
		{
			My_snprinf(line, MAX_LINE, "player=cpmesh(\"%s\",asset,",sName.c_str());
			sScript.append(line);
		}
		else
		{
			sScript.append("player=cpmesh(\"\", asset, ");
		}

		My_snprinf(line, MAX_LINE, "%f,%f,%f, %s, \
\"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\");\n", fOBB_X, fOBB_Y, fOBB_Z, ((g_bForceExportMeshPhysics || IsPhysicsEnabled()) ? "true":"false"),
			mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43);
		sScript.append(line);
		//////////////////////////////////////////////////////////////////////////
		// write replaceable textures
		int nNumReplaceableTextures = GetNumReplaceableTextures();
		if(nNumReplaceableTextures>0)
		{
			for(int i=0;i<nNumReplaceableTextures;++i)
			{
				TextureEntity* pTex1 =  GetReplaceableTexture(i);
				if( pTex1)
				{
					TextureEntity* pTex2 =  GetDefaultReplaceableTexture(i);
					if(pTex2 && pTex2->IsValid() && pTex2!=pTex1)
					{
						// set the replaceable texture if it is different from the default one. 
						string sFileName = pTex1->GetKey();
						if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
						{
							CPathReplaceables::GetSingleton().EncodePath(sFileName, sFileName, "WORLD");
						}
						My_snprinf(line, MAX_LINE, "player:SetReplaceableTexture(%d,ParaAsset.LoadTexture(\"\",\"%s\",1));\n", i, sFileName.c_str());
						sScript.append(line);
					}
				}
			}
		}
		// write if it is a big mesh
		if(CheckAttribute(OBJ_BIG_STATIC_OBJECT))
		{
			My_snprinf(line, MAX_LINE, "player:SetAttribute(8192, true);\n");
			sScript.append(line);
		}
		

		const char* sHomeZoneName = GetHomeZoneName();
		if(sHomeZoneName && sHomeZoneName[0]!= '\0')
		{
			My_snprinf(line, MAX_LINE, "player:SetHomeZone(\"%s\");\n", sHomeZoneName);
			sScript.append(line);
		}
		//////////////////////////////////////////////////////////////////////////
		// write dynamic attributes
		/* test code cause mesh to save something 
		CVariable tmp;
		tmp = "string test \" asdad\" hoho";

		SetDynamicField("URL", &tmp);
		tmp = 25.2341233;
		SetDynamicField("double", &tmp);
		tmp = 11;
		SetDynamicField("int", &tmp);
		*/
		int nFieldCount = GetDynamicFieldCount();
		if(nFieldCount>0)
		{
			sScript.append("att = player:GetAttributeObject();\n");
			for (int i=0;i<nFieldCount; ++i)
			{
				const char* name = GetDynamicFieldNameByIndex(i);
				if(name)
				{
					CDynamicAttributeField* pField = GetDynamicField(name);
					if(pField)
					{
						My_snprinf(line, MAX_LINE, "att:SetDynamicField(\"%s\", ", name);
						sScript.append(line);
						pField->ToNPLString(sScript);
						sScript.append(");\n");
					}
				}
			}
		}
		// write physics group if it is not the default value. 
		if(GetPhysicsGroup() != 0)
		{
			My_snprinf(line, MAX_LINE, "player:SetPhysicsGroup(%d);\n", GetPhysicsGroup());
			sScript.append(line);
		}
		// write the shadow caster if it is not the default value. 
		if(!IsShadowCaster())
		{
			My_snprinf(line, MAX_LINE, "player:SetField("");\n", GetPhysicsGroup());
			sScript.append("player:SetField(\"ShadowCaster\",false);\n");
		}
		// write the shadow receiver if it is not the default value. 
		if(IsShadowReceiver())
		{
			My_snprinf(line, MAX_LINE, "player:SetField("");\n", GetPhysicsGroup());
			sScript.append("player:SetField(\"ShadowReceiver\",true);\n");
		}

		//////////////////////////////////////////////////////////////////////////
		// write positions, facing, and attach
		Vector3 vPos = GetPosition();
		if(fFacing!=0.f)
		{
			My_snprinf(line, MAX_LINE, "player:SetFacing(%f);", fFacing);
			sScript.append(line);
		}
		My_snprinf(line, MAX_LINE, "player:SetPosition(%f,%f,%f);%s(player);\n", 
			vPos.x, vPos.y, vPos.z, (nMethod&CBaseObject::NPL_CREATE_IN_LOADER)?"sceneLoader:AddChild":"ParaScene.Attach");
		sScript.append(line);
	}
	else
	{
		OUTPUT_LOG("mesh entity not found when converting mesh physics object to string.");
	}
	return sScript;
}

CMeshPhysicsObject::CMeshPhysicsObject(void)
 :m_nPhysicsGroup(0), m_dwPhysicsMethod(0), m_pMeshObject(NULL), m_bIsPersistent(true)
{
	/// set house type, so that it is a container, yet free space
	SetMyType(_House);
}

CMeshPhysicsObject::~CMeshPhysicsObject(void)
{
	SAFE_DELETE(m_pMeshObject);
}

void CMeshPhysicsObject::Cleanup()
{
	UnloadPhysics();
	SAFE_DELETE(m_pMeshObject)
}

void CMeshPhysicsObject::CreateMeshObject()
{
	
}

HRESULT CMeshPhysicsObject::InitObject(MeshEntity* ppMesh, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics, const Matrix4& localTransform)
{
	SetBoundingBox(fOBB_X, fOBB_Y, fOBB_Z, 0);
	
	/// create the mesh object
	m_pMeshObject = new CMeshObject();
	if(m_pMeshObject)
	{
		m_pMeshObject->InitObject(ppMesh, NULL, m_vPos); 
		m_pMeshObject->SetMyType(_DummyMesh);
		m_pMeshObject->SetBoundingBox(fOBB_X, fOBB_Y, fOBB_Z, 0);

		m_pMeshObject->SetLocalTransform(localTransform);
	}
	if(!bApplyPhysics)
		m_dwPhysicsMethod = PHYSICS_FORCE_NO_PHYSICS;

	SetParamsFromAsset();

	return S_OK;
}

void CMeshPhysicsObject::SetOnAssetLoaded(const char* sCallbackScript)
{
	CBaseObject::SetOnAssetLoaded(sCallbackScript);
	if(m_pMeshObject && m_pMeshObject->m_ppMesh.get() && m_pMeshObject->m_ppMesh->GetPrimaryTechniqueHandle() > 0)
	{
		ActivateScript(Type_OnAssetLoaded);
	}
}

void CMeshPhysicsObject::SetParamsFromAsset()
{
	if(m_pMeshObject && m_pMeshObject->m_ppMesh.get() && m_pMeshObject->m_ppMesh->GetPrimaryTechniqueHandle() > 0)
	{
		bool bApplyPhysics = true;
		// mesh asset ending with _a or _b will be forced no physics initially. It will override the bApplyPhysics input parameter.
		// file name with "_r" means it is shadow receiver.
		const string& filename = m_pMeshObject->m_ppMesh->GetFileName();
		int nSize = (int)filename.size();
		for (int i=0;i<5;++i)
		{
			// Index of the character '_'
			int nMarkIndex = nSize-4-i*2;
			if(nMarkIndex>=0 && filename[nMarkIndex] == '_')
			{
				char symbol=filename[nMarkIndex+1];
				// if model file name end with "_a", no physics are applied..
				switch (symbol)
				{
				case 'b':
				case 'a':
				case 'p':
					{
						bApplyPhysics = false;
						break;
					}
				case 'e':
					{
						SetShadowCaster(false);
						break;
					}
				case 'r':
					{
						SetShadowCaster(false);
						SetShadowReceiver(true);
						break;
					}
				}
			}
			else
				break;
		}
	
		if(m_dwPhysicsMethod == 0)
		{
			if(bApplyPhysics)
				m_dwPhysicsMethod = PHYSICS_LAZY_LOAD;
			else
				m_dwPhysicsMethod = PHYSICS_FORCE_NO_PHYSICS;
		}
		else if (IsPhysicsEnabled() && ((m_dwPhysicsMethod&PHYSICS_ALWAYS_LOAD) > 0))
		{
			LoadPhysics();
		}

		//if(GetTileContainer())
		//{
		//	// needs to reattach the object to the scene, since the bounding volume is known. 
		//}

		ActivateScript(Type_OnAssetLoaded);
	}
}

CMeshObject* CMeshPhysicsObject::GetMeshObject()
{
	return m_pMeshObject;
}
void CMeshPhysicsObject::SetLocalTransform(const Matrix4& localTransform)
{
	if(m_pMeshObject)
	{
		m_pMeshObject->SetLocalTransform(localTransform);
	}
}

void CMeshPhysicsObject::GetLocalTransform(Matrix4* localTransform)
{
	if(m_pMeshObject)
	{
		m_pMeshObject->GetLocalTransform(localTransform);
	}
}

void CMeshPhysicsObject::SetRotation(const Quaternion& quat)
{
	if(m_pMeshObject)
	{
		m_pMeshObject->SetRotation(quat);
	}
}

void CMeshPhysicsObject::GetRotation(Quaternion* quat)
{
	if(m_pMeshObject)
	{
		m_pMeshObject->GetRotation(quat);
	}
}

void CMeshPhysicsObject::Rotate(float x, float y, float z)
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	Matrix4 mat1;
	if(x!=0.f)
		mat = (*ParaMatrixRotationX(&mat1, x))*mat;
	if(y!=0.f)
		mat = (*ParaMatrixRotationY(&mat1, y))*mat;
	if(z!=0.f)
		mat = (*ParaMatrixRotationZ(&mat1, z))*mat;

	SetLocalTransform(mat);
}

void CMeshPhysicsObject::SetScaling(float s)
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	float fScalingX,fScalingY,fScalingZ;
	Math::GetMatrixScaling(mat, &fScalingX,&fScalingY,&fScalingZ);
	Matrix4 mat1;
	ParaMatrixScaling(&mat1, s/fScalingX,s/fScalingY,s/fScalingZ);
	mat = mat1*mat;
	SetLocalTransform(mat);
}

float CMeshPhysicsObject::GetScaling()
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	float fScalingX,fScalingY,fScalingZ;
	Math::GetMatrixScaling(mat, &fScalingX,&fScalingY,&fScalingZ);

	return max(max(fScalingX,fScalingY), fScalingZ);
}

void CMeshPhysicsObject::Reset()
{
	SetLocalTransform(*CGlobals::GetIdentityMatrix());
}


void CMeshPhysicsObject::LoadPhysics()
{
	if( m_dwPhysicsMethod > 0 && IsPhysicsEnabled() && (GetStaticActorCount()==0) && m_pMeshObject )
	{
		/** only load physics when the construction percentage is 100% or it is a shadow receiver.*/
		if( m_pMeshObject->GetCtorPercentage()>=1.f || IsShadowReceiver())
		{ 
			MeshEntity* ppMesh = m_pMeshObject->m_ppMesh.get();
			if(ppMesh==0 || ppMesh->GetAABBMax().x <= 0.f)
			{
				EnablePhysics(false); // disable physics forever, if failed loading physics data
				return;
			}
			Matrix4 matWorld;
			m_pMeshObject->GetViewClippingObject()->GetWorldTransform(matWorld);
			IParaPhysicsActor* pActor = CGlobals::GetPhysicsWorld()->CreateStaticMesh(m_pMeshObject->m_ppMesh.get(), matWorld, m_nPhysicsGroup, &m_staticActors, this);
			if(pActor!=NULL)
			{
			}
			{
				//
				// for static x-reference objects, add static actors if any
				// 
				int nTotal = m_pMeshObject->GetXRefInstanceCount();	
				for (int i=0;i<nTotal;++i)
				{
					XRefObject* pEntity = m_pMeshObject->GetXRefInstanceByIndex(i);
					if(pEntity && pEntity->GetModelAsset() && pEntity->GetModelAsset()->GetType()== AssetEntity::mesh)
					{
						MeshEntity* ppMesh = (MeshEntity* )pEntity->GetModelAsset();
						int nSize = (int)ppMesh->GetFileName().size();
						bool bApplyPhysics = true;
						for (int i=0;i<5;++i)
						{
							// Index of the character '_'
							int nMarkIndex = nSize-4-i*2;
							if(nMarkIndex>=0 && (ppMesh->GetFileName())[nMarkIndex] == '_')
							{
								char symbol=(ppMesh->GetFileName())[nMarkIndex+1];
								// if model file name end with "_a", no physics are applied..
								if (symbol== 'b' || symbol== 'a' || symbol== 'p')
								{
									bApplyPhysics = false;
								}
							}
							else
								break;
						}

						if(bApplyPhysics)
						{
							Matrix4 mat;
							pEntity->GetMatrix(mat, &matWorld);
							pActor =  CGlobals::GetPhysicsWorld()->CreateStaticMesh( ppMesh, mat, m_nPhysicsGroup, &m_staticActors, this);
							if(pActor)
							{
							}
						}
					}
				}
			}
			if(m_staticActors.empty())
			{
				// disable physics forever, if no physics actors are loaded. 
				EnablePhysics(false); 
			}
		}
	}
}

void CMeshPhysicsObject::UnloadPhysics()
{
	int nSize = (int)m_staticActors.size();
	if(nSize>0)
	{
		for (int i=0;i<nSize;++i)
		{
			CGlobals::GetPhysicsWorld()->ReleaseActor(m_staticActors[i]);
		}
		m_staticActors.clear();
	}
}

void CMeshPhysicsObject::CompressObject(CompressOption option)
{
	if(option & CBaseObject::ReleasePhysics)
		UnloadPhysics();
	if(m_pMeshObject)
		m_pMeshObject->CompressObject(option);
}

void CMeshPhysicsObject::SetYaw(FLOAT fYaw)
{
	m_fYaw = fYaw;
	if(m_pMeshObject)
	{
		m_pMeshObject->SetYaw(m_fYaw);
	}
}
void CMeshPhysicsObject::SetPosition(const DVector3 & v)
{
	m_vPos = v;
	if(m_pMeshObject)
	{
		m_pMeshObject->SetPosition(m_vPos);
	}
}

IViewClippingObject* CMeshPhysicsObject::GetViewClippingObject()
{
	if(m_pMeshObject)
	{
		return m_pMeshObject->GetViewClippingObject();
	}
	return this;
}


void ParaEngine::CMeshPhysicsObject::Animate( double dTimeDelta, int nRenderNumber/*=0 */ )
{
	if(m_pMeshObject)
	{
		m_pMeshObject->Animate(dTimeDelta, nRenderNumber);
	}
}

HRESULT CMeshPhysicsObject::Draw( SceneState * sceneState)
{
	if (!m_pMeshObject || !IsVisible())
		return S_OK;
	if (CheckAttribute(OBJ_CUSTOM_RENDERER)){
		ViewTouch();
		return S_OK;
	}

	bool bCheckAsset = (m_pMeshObject->GetPrimaryTechniqueHandle()<0);

	sceneState->SetCurrentSceneObject(this);

	m_pMeshObject->SetPosition(m_vPos);
	m_pMeshObject->SetFacing(m_fYaw);

	if(CheckAttribute(MESH_USE_LIGHT))
		m_pMeshObject->SetAttribute(MESH_USE_LIGHT, true);

	SetFrameNumber(sceneState->m_nRenderCount);
	//sceneState->pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	HRESULT hr = m_pMeshObject->Draw(sceneState);
	//sceneState->pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	if(bCheckAsset && m_pMeshObject->GetPrimaryTechniqueHandle()>0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();
	}
	return hr;
}

Matrix4* CMeshPhysicsObject::GetRenderMatrix(Matrix4& out, int nRenderNumber)
{
	if(m_pMeshObject)
	{
		m_pMeshObject->SetPosition(m_vPos);
		m_pMeshObject->SetFacing(m_fYaw);
		return m_pMeshObject->GetRenderMatrix(out, nRenderNumber);
	}
	return &out;
}

AssetEntity* CMeshPhysicsObject::GetPrimaryAsset()
{
	if(m_pMeshObject)
		return (AssetEntity*) (m_pMeshObject->m_ppMesh.get());
	else 
		return NULL;
}
void CMeshPhysicsObject::EnablePhysics(bool bEnable)
{
	if(!bEnable){
		UnloadPhysics();
		m_dwPhysicsMethod |= PHYSICS_FORCE_NO_PHYSICS;
	}
	else
	{
		m_dwPhysicsMethod &= (~PHYSICS_FORCE_NO_PHYSICS);
		if((m_dwPhysicsMethod&PHYSICS_ALWAYS_LOAD)>0)
			LoadPhysics();
	}
}

bool CMeshPhysicsObject::IsPhysicsEnabled()
{
	return !((m_dwPhysicsMethod & PHYSICS_FORCE_NO_PHYSICS)>0);
}

int CMeshPhysicsObject::GetPrimaryTechniqueHandle()
{
	if(m_pMeshObject)
		return m_pMeshObject->GetPrimaryTechniqueHandle();
	else
		return CBaseObject::GetPrimaryTechniqueHandle();
}

void CMeshPhysicsObject::SetPrimaryTechniqueHandle(int nHandle)
{
	if(m_pMeshObject)
		m_pMeshObject->SetPrimaryTechniqueHandle(nHandle);
	else
		CBaseObject::SetPrimaryTechniqueHandle(nHandle);
}
float CMeshPhysicsObject::GetCtorPercentage()
{
	if(m_pMeshObject)
		return m_pMeshObject->GetCtorPercentage();
	else
		return CBaseObject::GetCtorPercentage();
}
void CMeshPhysicsObject::SetCtorPercentage(float fPercentage)
{
	if(m_pMeshObject)
		m_pMeshObject->SetCtorPercentage(fPercentage);
	else
		CBaseObject::SetCtorPercentage(fPercentage);
}

bool ParaEngine::CMeshPhysicsObject::HasAlphaBlendedObjects()
{
	if (m_pMeshObject)
		return m_pMeshObject->HasAlphaBlendedObjects();
	else
		return CBaseObject::HasAlphaBlendedObjects();
}

bool CMeshPhysicsObject::IsTransparent()
{
	if(m_pMeshObject)
		return m_pMeshObject->IsTransparent();
	else
		return CBaseObject::IsTransparent();
}

void CMeshPhysicsObject::SetTransparent(bool bIsTransparent)
{
	if(m_pMeshObject)
		m_pMeshObject->SetTransparent(bIsTransparent);
	else
		CBaseObject::SetTransparent(bIsTransparent);
}

int ParaEngine::CMeshPhysicsObject::GetNumReplaceableTextures()
{
	if(m_pMeshObject)
		return m_pMeshObject->GetNumReplaceableTextures();
	else
		return 0;
}

TextureEntity* ParaEngine::CMeshPhysicsObject::GetDefaultReplaceableTexture( int ReplaceableTextureID )
{
	if(m_pMeshObject)
		return m_pMeshObject->GetDefaultReplaceableTexture(ReplaceableTextureID);
	else
		return NULL;
}

TextureEntity* ParaEngine::CMeshPhysicsObject::GetReplaceableTexture( int ReplaceableTextureID )
{
	if(m_pMeshObject)
		return m_pMeshObject->GetReplaceableTexture(ReplaceableTextureID);
	else
		return NULL;
}

bool ParaEngine::CMeshPhysicsObject::SetReplaceableTexture( int ReplaceableTextureID, TextureEntity* pTextureEntity )
{
	if(m_pMeshObject)
		return m_pMeshObject->SetReplaceableTexture(ReplaceableTextureID, pTextureEntity);
	else
		return false;
}

void ParaEngine::CMeshPhysicsObject::AutoSelectTechnique()
{
	if(m_pMeshObject)
	{
		m_pMeshObject->AutoSelectTechnique();

		int nEffectHandle = GetPrimaryTechniqueHandle();
		
		if(nEffectHandle > 0)
		{
			if( nEffectHandle == (IsShadowReceiver() ? TECH_SIMPLE_MESH_NORMAL : TECH_SIMPLE_MESH_NORMAL_SHADOW) )
			{
				if(CGlobals::GetScene()->IsShadowMapEnabled())
					SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_SHADOW);
				else
					SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL);
			}
		}
	}
}

void ParaEngine::CMeshPhysicsObject::OnSelect( int nGroupID )
{
	SetSelectGroupIndex(nGroupID);
	if(m_pMeshObject)
		return m_pMeshObject->OnSelect(nGroupID);
}

void ParaEngine::CMeshPhysicsObject::OnDeSelect()
{
	SetSelectGroupIndex(-1);
	if(m_pMeshObject)
		return m_pMeshObject->OnDeSelect();
}

CParameterBlock* ParaEngine::CMeshPhysicsObject::GetEffectParamBlock( bool bCreateIfNotExist /*= false*/ )
{
	if(m_pMeshObject)
		return m_pMeshObject->GetEffectParamBlock(bCreateIfNotExist);
	return NULL;
}

Matrix4* ParaEngine::CMeshPhysicsObject::GetAttachmentMatrix( Matrix4& pOut, int nAttachmentID/*=0*/ , int nRenderNumber)
{
	if(m_pMeshObject)
		return m_pMeshObject->GetAttachmentMatrix(pOut, nAttachmentID, nRenderNumber);
	return NULL;
}

bool ParaEngine::CMeshPhysicsObject::HasAttachmentPoint( int nAttachmentID/*=0*/ )
{
	if(m_pMeshObject)
		return m_pMeshObject->HasAttachmentPoint(nAttachmentID);
	return false;
}

void ParaEngine::CMeshPhysicsObject::SetAnimation( int nAnimID )
{
	if(m_pMeshObject)
		m_pMeshObject->SetAnimation(nAnimID);
}

int ParaEngine::CMeshPhysicsObject::GetAnimation()
{
	if(m_pMeshObject)
		return m_pMeshObject->GetAnimation();
	return 0;
}

void ParaEngine::CMeshPhysicsObject::SetAnimFrame(int nFrame)
{
	if(m_pMeshObject)
		m_pMeshObject->SetAnimFrame(nFrame);
}

int ParaEngine::CMeshPhysicsObject::GetAnimFrame()
{
	if(m_pMeshObject)
		return m_pMeshObject->GetAnimFrame();
	return 0;
}

void ParaEngine::CMeshPhysicsObject::SetUseGlobalTime(bool bUseGlobalTime)
{
	if(m_pMeshObject)
		m_pMeshObject->SetUseGlobalTime(bUseGlobalTime);
}

bool ParaEngine::CMeshPhysicsObject::IsUseGlobalTime()
{
	if(m_pMeshObject)
		return m_pMeshObject->IsUseGlobalTime();
	return false;
}

int ParaEngine::CMeshPhysicsObject::GetPhysicsGroup()
{
	return m_nPhysicsGroup;
}

int ParaEngine::CMeshPhysicsObject::GetStaticActorCount()
{
	return (int)m_staticActors.size();
}

bool ParaEngine::CMeshPhysicsObject::CanHasPhysics()
{
	return true;
}

void ParaEngine::CMeshPhysicsObject::SetPhysicsGroup(int nGroup)
{
	PE_ASSERT(0<=nGroup && nGroup<32);
	if(m_nPhysicsGroup != nGroup)
	{
		m_nPhysicsGroup = nGroup;
		UnloadPhysics();
	}
}

void ParaEngine::CMeshPhysicsObject::SetBoundingBox( float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing )
{
	CTileObject::SetBoundingBox(fOBB_X, fOBB_Y, fOBB_Z, fFacing);
	if(m_pMeshObject)
		m_pMeshObject->SetBoundingBox(fOBB_X, fOBB_Y, fOBB_Z, fFacing);
}

void ParaEngine::CMeshPhysicsObject::SetBoundRect( FLOAT fWidth ,FLOAT fHeight, FLOAT fFacing )
{
	CTileObject::SetBoundRect(fWidth, fHeight, fFacing);
	if(m_pMeshObject)
		m_pMeshObject->SetBoundRect(fWidth, fHeight, fFacing);
}

void ParaEngine::CMeshPhysicsObject::SetRadius( FLOAT fRadius )
{
	CTileObject::SetRadius(fRadius);
	if(m_pMeshObject)
		m_pMeshObject->SetRadius(fRadius);
}

void ParaEngine::CMeshPhysicsObject::SetAlwaysLoadPhysics(bool bEnable)
{
	if (bEnable)
	{
		m_dwPhysicsMethod |= PHYSICS_ALWAYS_LOAD;
	}
	else
	{
		m_dwPhysicsMethod &= (~PHYSICS_ALWAYS_LOAD);
	}
}

bool ParaEngine::CMeshPhysicsObject::ViewTouch()
{
	if(!m_pMeshObject)
		return false;

	if((m_pMeshObject->GetPrimaryTechniqueHandle()<0))
	{
		if(m_pMeshObject->ViewTouch())
		{
			// in case the asset is loaded successfully, we shall set the primary asset. 
			SetParamsFromAsset();
			return true;
		}
		return false;
	}
	return true;
}

bool ParaEngine::CMeshPhysicsObject::IsPersistent()
{
	return m_bIsPersistent;
}

void ParaEngine::CMeshPhysicsObject::SetPersistent( bool bPersistent )
{
	m_bIsPersistent = bPersistent;
}

void ParaEngine::CMeshPhysicsObject::SetFaceCullingDisabled( bool bDisableFaceCulling )
{
	if(m_pMeshObject)
		m_pMeshObject->SetFaceCullingDisabled(bDisableFaceCulling);
}

bool ParaEngine::CMeshPhysicsObject::IsFaceCullingDisabled()
{
	if(m_pMeshObject)
		return m_pMeshObject->IsFaceCullingDisabled();
	else
		return false;
}

int ParaEngine::CMeshPhysicsObject::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	CTileObject::InstallFields(pClass, bOverride);

	pClass->AddField("FaceCullingDisabled", FieldType_Bool, (void*)SetFaceCullingDisabled_s, (void*)IsFaceCullingDisabled_s, NULL, "", bOverride);

	return S_OK;
}
