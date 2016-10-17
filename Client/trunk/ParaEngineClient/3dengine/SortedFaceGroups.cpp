//-----------------------------------------------------------------------------
// Class:	CSortedFaceGroups
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.12.10
// Desc: provides a simple way to render translucent mesh faces correctly. 
// when exporting mesh with translucent faces, we need to mark its material with "sort" in the name.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EffectManager.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXBone.h"
#include "SceneObject.h"

#include "SortedFaceGroups.h"
#include "memdebug.h"

namespace ParaEngine
{
	/**
	* for sorting face group from back to front. 
	*/
	struct FaceGroup_Back_To_Front
	{
		inline bool operator()(
			const CFaceGroupInstance& _Left, 
			const CFaceGroupInstance& _Right
			) const
		{
			return (_Left.m_facegroup->m_order>_Right.m_facegroup->m_order) || ((_Left.m_facegroup->m_order==_Right.m_facegroup->m_order) && (_Left.m_fToCameraDistSq) > (_Right.m_fToCameraDistSq));
		};
	};
}
using namespace ParaEngine;

ParaEngine::CFaceGroup::CFaceGroup()
{
	m_order = 0;
	m_alphaBlending = false;
	m_alphaTesting = false;
	m_disableZWrite = false;
	m_bHasLighting = false;
	m_bAdditive = false;
	m_bSkinningAni = false;

	m_pTexture = NULL;
	m_nNumTriangles = 0;

	m_vCenterPos = Vector3(0.f,0.f,0.f);
	m_stripLength = 0;
}

void ParaEngine::CFaceGroup::UpdateCenterPos()
{
	int nCount = (int) m_vertices.size();
	if(nCount>0)
	{
		m_vCenterPos = Vector3(0,0,0);
		for (int i=0; i<nCount; ++i)
		{
			m_vCenterPos += m_vertices[i];
		}
		m_vCenterPos /= (float)nCount;
	}
}

ParaEngine::CFaceGroupInstance::CFaceGroupInstance()
	:m_vUVOffset(0.f, 0.f), m_fToCameraDistSq(0.f), m_facegroup(NULL), m_fAlpha(1.f),m_UVRgbAnim(false),
	m_bones(NULL)
{
}

ParaEngine::CFaceGroupInstance::CFaceGroupInstance( const Matrix4* pMat, CFaceGroup* faceGroup )
	:m_vUVOffset(0.f, 0.f), m_fToCameraDistSq(0.f), m_facegroup(faceGroup), m_fAlpha(1.f),m_UVRgbAnim(false),
	m_bones(NULL)
{
	if(pMat)
		m_transform = *pMat;
}
//////////////////////////////////////////////////////////////////////////
//
// CSortedFaceGroups
//
//////////////////////////////////////////////////////////////////////////

CSortedFaceGroups::CSortedFaceGroups(void)
{
	m_sorted = false;

}

CSortedFaceGroups::~CSortedFaceGroups(void)
{
}


void ParaEngine::CSortedFaceGroups::Clear()
{
	m_sorted = false;
	m_sortedFaceGroups.clear();
}

void ParaEngine::CSortedFaceGroups::AddFaceGroup( const CFaceGroupInstance& facegroup )
{
	m_sortedFaceGroups.push_back(facegroup);
	m_sorted = false;
}

// Note:  vCameraPos should minors scene render origin.
void ParaEngine::CSortedFaceGroups::Sort( const Vector3& vCameraPos )
{
	m_sorted = true;

	if(m_sortedFaceGroups.empty())
		return;	

	//////////////////////////////////////////////////////////////////////////
	// calculate camera to face distances. 
	FaceGroups_Type::iterator itCur, itEnd = m_sortedFaceGroups.end();
	for (itCur = m_sortedFaceGroups.begin(); itCur!=itEnd; ++itCur)
	{
		CFaceGroupInstance& facegroup =  (*itCur);
		Vector3 vCenterPos(0,0,0);
		ParaVec3TransformCoord(&vCenterPos, &(facegroup.m_facegroup->m_vCenterPos), &(facegroup.m_transform));
		facegroup.m_fToCameraDistSq = (vCenterPos-vCameraPos).squaredLength();
	}

	//////////////////////////////////////////////////////////////////////////
	// sort faces according to the afore-calculated distances. 
	std::sort(m_sortedFaceGroups.begin(), m_sortedFaceGroups.end(), FaceGroup_Back_To_Front());
}

void ParaEngine::CSortedFaceGroups::Render()
{
	if(m_sortedFaceGroups.empty())
		return;	

	if(!m_sorted)
	{
		OUTPUT_LOG("calling CSortedFaceGroups::render without sorting it first. \n");
		return;
	}
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	
	if ( pEffect == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		// TODO: render with fixed function pipeline
		bool bl = false, at= false, bDisableBackFacing = false,zm=false;
		bool bHasLighting = true;
		bool bAdditive = false;
		Vector2 vLastUVOffset(0.f,0.f);


		mesh_vertex_normal* vb_vertices = NULL;
		int nNumLockedVertice;
		int nNumFinishedVertice = 0;
		// pd3dDevice->SetRenderState( D3DRS_FOGENABLE,      FALSE);

		DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
		pd3dDevice->SetStreamSource( 0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal) );

		FaceGroups_Type::iterator itCur, itEnd = m_sortedFaceGroups.end();
		for (itCur = m_sortedFaceGroups.begin(); itCur!=itEnd; ++itCur)
		{
			CFaceGroupInstance& facegroup =  (*itCur);
			//////////////////////////////////////////////////////////////////////////
			// set up render attributes for this face group
			DeviceTexturePtr_type pTexture = NULL;
			if( facegroup.m_facegroup->m_pTexture )
			{
				pTexture = facegroup.m_facegroup->m_pTexture->GetTexture();

				if(pTexture)
				{
					CGlobals::GetEffectManager()->applySurfaceMaterial( &(facegroup.m_facegroup->m_material) );

					//////////////////////////////////////////////////////////////////////////
					// lighting
					if(bHasLighting != facegroup.m_facegroup->m_bHasLighting)
					{
						bHasLighting = !bHasLighting;
						//CGlobals::GetEffectManager()->EnableSunLight(bHasLighting);
					}

					/// set blending and alpha testing
					bool blend = facegroup.m_facegroup->m_alphaBlending;
					bool atest = facegroup.m_facegroup->m_alphaTesting; 
					bool zmask = facegroup.m_facegroup->m_disableZWrite;
					bool bUseAdditive = facegroup.m_facegroup->m_bAdditive;
					/** turn on or off z write: z mask means disable z write */
					if(bUseAdditive && !bAdditive){
						if(blend)
						{
							pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
							pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
						}
						else
						{
							pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
							pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
						}
						bAdditive = true;
					}
					else if(!bUseAdditive && bAdditive){
						pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
						pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
						bAdditive = false;
					}

					/** if either blending or alpha testing is enabled, we will disable back face culling.*/
					if((blend || atest) && (!bDisableBackFacing))
					{
						CGlobals::GetEffectManager()->SetCullingMode(false);
						bDisableBackFacing = true;
					}
					else if((!blend && !atest) && bDisableBackFacing)
					{
						CGlobals::GetEffectManager()->SetCullingMode(true);
						bDisableBackFacing = false;
					}

					/** turn on and off blending. */
					if (blend && !bl ) 
					{
						pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
						bl = true;
					}
					else if (!blend && bl) 
					{
						pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
						bl = false;
					}

					/** turn on and off alpha testing. */
					if (atest && !at) 
					{
						pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
						at = true;
					}
					else if (!atest && at) 
					{
						pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
						at = false;
					}
					/** turn on or off z write: z mask means disable z write */
					if(zmask && !zm){
						pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
						zm = true;
					}
					else if(!zmask && zm){
						pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
						zm = false;
					}

					/** uv offset */
					if(vLastUVOffset != facegroup.m_vUVOffset)
					{
						vLastUVOffset = facegroup.m_vUVOffset;

						Matrix4 texMat;
						// interesting that it uses _31 and _32 instead of _41, _42 for the UV translation.
						texMat = Matrix4::IDENTITY;
						texMat._31 = vLastUVOffset.x;
						texMat._32 = vLastUVOffset.y;
						pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 	D3DTTFF_COUNT2);
						pd3dDevice->SetTransform( D3DTS_TEXTURE0, texMat.GetConstPointer() );
					}
					pd3dDevice->SetTexture( 0, pTexture);
				}
			}
			if(pTexture)
			{
				//////////////////////////////////////////////////////////////////////////
				// draw the face group. 

				// set world transform. 
				CGlobals::GetRenderDevice()->SetTransform( D3DTS_WORLD, facegroup.m_transform.GetConstPointer());

				int nTotalVertices = facegroup.m_facegroup->m_nNumTriangles*3;
				do
				{
					if( (nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice), 
						(void**)(&vb_vertices))) > 0)
					{
						int nLockedNum = nNumLockedVertice/3;

						for(int i=0;i<nNumLockedVertice;++i)
						{
							vb_vertices[i].p = facegroup.m_facegroup->m_vertices[i];
							vb_vertices[i].n = facegroup.m_facegroup->m_normals[i];
							vb_vertices[i].uv = facegroup.m_facegroup->m_UVs[i];
						}

						pBufEntity->Unlock();

						RenderDevice::DrawPrimitive( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLELIST,pBufEntity->m_dwBase,nLockedNum);

						if((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
						{
							nNumFinishedVertice += nNumLockedVertice;
						}
						else
							break;
					}
					else 
						break;
				}while(1);
			}
		}
	
		if(bl)
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		if(at)
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		if(zm)
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		if(bDisableBackFacing)
			CGlobals::GetEffectManager()->SetCullingMode(true);
		if(bAdditive){
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		//if(!bHasLighting)
			//CGlobals::GetEffectManager()->EnableSunLight(true);
		if(vLastUVOffset != Vector2(0.f, 0.f))
		{
			// disable texture transformation in fixed function.
			pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 	D3DTTFF_DISABLE );
		}
		// pd3dDevice->SetRenderState( D3DRS_FOGENABLE, CGlobals::GetScene()->IsFogEnabled());
#endif
	}
	else
	{
		// render with shader technique
		UINT cPasses = (UINT)pEffect->totalPasses();
		bool bl=false,at= false, bDisableBackFacing = false,zm=false;
		bool bEnvMap = false, bReflectionMap = false, bNormalMap = false, bHasLightMap = false, bHasLighting = true;
		bool bAdditive = false;
		Vector2 vLastUVOffset(0.f,0.f);
		bool restoreUVRgbAnim = false;
		float fLastAlpha = 1.f;

		mesh_vertex_normal* vb_vertices = NULL;
		int nNumLockedVertice;
		int nNumFinishedVertice = 0;
		DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
		pd3dDevice->SetStreamSource( 0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal) );

		if(pEffect->begin())
		{
			for( UINT p = 0; p < cPasses; ++p )
			{
				pEffect->BeginPass( p );
				
				FaceGroups_Type::iterator itCur, itEnd = m_sortedFaceGroups.end();
				for (itCur = m_sortedFaceGroups.begin(); itCur!=itEnd; ++itCur)
				{
					CFaceGroupInstance& facegroup =  (*itCur);
					//////////////////////////////////////////////////////////////////////////
					// set up render attributes for this face group
					DeviceTexturePtr_type pTexture = 0;
					if( facegroup.m_facegroup->m_pTexture )
					{
						pTexture = facegroup.m_facegroup->m_pTexture->GetTexture();

						if(pTexture)
						{
							pEffect->applySurfaceMaterial( &(facegroup.m_facegroup->m_material) );

							//////////////////////////////////////////////////////////////////////////
							// lighting
							if(bHasLighting != facegroup.m_facegroup->m_bHasLighting)
							{
								bHasLighting = !bHasLighting;
								pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled() && bHasLighting);
							}

							/// set blending and alpha testing
							bool blend = facegroup.m_facegroup->m_alphaBlending;
							bool atest = facegroup.m_facegroup->m_alphaTesting; 
							bool zmask = facegroup.m_facegroup->m_disableZWrite;
							bool bUseAdditive = facegroup.m_facegroup->m_bAdditive;
							/** turn on or off z write: z mask means disable z write */
							if(bUseAdditive && !bAdditive)
							{
								if(blend)
								{
									pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
									pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
								}
								else
								{
									pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
									pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
								}
								bAdditive = true;
							}
							else if(!bUseAdditive && bAdditive)
							{
								pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
								pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
								bAdditive = false;
							}

							/** if either blending or alpha testing is enabled, we will disable back face culling.*/
							if((blend || atest) && (!bDisableBackFacing))
							{
								CGlobals::GetEffectManager()->SetCullingMode(false);
								bDisableBackFacing = true;
							}
							else if((!blend && !atest) && bDisableBackFacing)
							{
								CGlobals::GetEffectManager()->SetCullingMode(true);
								bDisableBackFacing = false;
							}

							/** turn on and off blending. */
							if (blend && !bl ) 
							{
								pEffect->EnableAlphaBlending(blend);
								bl = true;
							}
							else if (!blend && bl) 
							{
								pEffect->EnableAlphaBlending(blend);
								bl = false;
							}

							/** turn on and off alpha testing. */
							if (atest && !at) 
							{
								pEffect->EnableAlphaTesting(atest);
								at = true;
							}
							else if (!atest && at) 
							{
								pEffect->EnableAlphaTesting(atest);
								at = false;
							}
							/** turn on or off z write: z mask means disable z write */
							if(zmask && !zm){
								pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
								zm = true;
							}
							else if(!zmask && zm){
								pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
								zm = false;
							}

							/** uv offset */
							if(vLastUVOffset != facegroup.m_vUVOffset)
							{
								vLastUVOffset = facegroup.m_vUVOffset;
								if(facegroup.m_facegroup->m_stripLength > 0)
									vLastUVOffset = Vector2(0,0);

								Vector4 vOffset(vLastUVOffset.x, vLastUVOffset.y, 0.f, 0.f);
								pEffect->setParameter(CEffectFile::k_ConstVector0, (const float*)&vOffset);

								//support texture uv rgb animation --clayman 2011.8.8
								if(facegroup.m_UVRgbAnim)
								{
									pEffect->setBool(CEffectFile::k_bBoolean7,TRUE);
									restoreUVRgbAnim = true;
								}
							}

							//alpha value   --clayman
							if(fLastAlpha != facegroup.m_fAlpha)
							{
								fLastAlpha = facegroup.m_fAlpha;
								pEffect->setFloat(CEffectFile::k_opacity, fLastAlpha);
							}							

							pEffect->setTexture( 0, pTexture);
						}
					}
					if(pTexture)
					{
						//////////////////////////////////////////////////////////////////////////
						// draw the face group. 

						// set world transform. 
						CGlobals::GetWorldMatrixStack().push(facegroup.m_transform);
						pEffect->applyWorldMatrices();
						CGlobals::GetWorldMatrixStack().pop();
						// commit texture and transform changes to device. 
						pEffect->CommitChanges();
						
						int nTotalVertices = facegroup.m_facegroup->m_nNumTriangles*3;
						do
						{
							if( (nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice), 
								(void**)(&vb_vertices))) > 0)
							{
								int nLockedNum = nNumLockedVertice/3;

								//calc texture strip coordinate
								float a = 0;
								float b = 0;
								bool enableTexStripEffect = facegroup.m_facegroup->m_stripLength > 0;
								if(enableTexStripEffect)
								{
									float stripLen = facegroup.m_facegroup->m_stripLength;
									float stripOffset = facegroup.m_vUVOffset.y;
									if(stripOffset > 0 && (stripOffset-1 < stripLen))
									{
										a = 1.0f/stripLen;
										b = 1- stripOffset / stripLen;
									}
								}

								for(int i=0;i<nNumLockedVertice;++i)
								{
									
									vb_vertices[i].n = facegroup.m_facegroup->m_normals[i];
									vb_vertices[i].uv = facegroup.m_facegroup->m_UVs[i];
									
									if(facegroup.m_facegroup->m_bSkinningAni)
									{
										Vector3 vertexPos;
										vertexPos.x = facegroup.m_facegroup->m_vertices[i].x;
										vertexPos.y = facegroup.m_facegroup->m_vertices[i].y;
										vertexPos.z = facegroup.m_facegroup->m_vertices[i].z;

										uint32 packedBoneIdx = facegroup.m_facegroup->m_boneIndices[i];
										uint32 packedWeights = facegroup.m_facegroup->m_vertexWeights[i];
										
										float weight = packedWeights % 256 * (1/255.0f);
										int boneIdx = packedBoneIdx % 256;
										Vector3 v = (vertexPos * facegroup.m_bones[boneIdx].mat) * weight;
										for (int b=1; b<4; b++)
										{
											weight = ((packedWeights >> (b*8)) % 256) * (1/255.0f);
											if(weight < 0.00001f)
												break;

											boneIdx = (packedBoneIdx >> (b*8)) % 256;
											v += (vertexPos * facegroup.m_bones[boneIdx].mat) * weight;
										}
										vb_vertices[i].p = v;
									}
									else
										vb_vertices[i].p = facegroup.m_facegroup->m_vertices[i];


									if(enableTexStripEffect)
									{
										Vector2 uv = facegroup.m_facegroup->m_UVs[i];
										uv.y = a * fabs(uv.y) + b;
										vb_vertices[i].uv = uv;
									}
								}

								pBufEntity->Unlock();

								
								if(enableTexStripEffect)
								{
									pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
									pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
								}

								if (pBufEntity->IsMemoryBuffer())
									RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
								else
									RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

								if(enableTexStripEffect)
								{
									pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
									pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
								}

								if((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
								{
									nNumFinishedVertice += nNumLockedVertice;
								}
								else
									break;
							}
							else 
								break;
						}while(1);
					}
				}
				pEffect->EndPass();
			}
			pEffect->end();

			if(bl)
				pEffect->EnableAlphaBlending(false);
			if(at)
				pEffect->EnableAlphaTesting(false);
			if(zm)
				pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			if(bAdditive){
				pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			if(bDisableBackFacing)
				CGlobals::GetEffectManager()->SetCullingMode(true);
			if(!bHasLighting)
				pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled());

			if(vLastUVOffset != Vector2(0.f, 0.f))
			{
				pEffect->setParameter(CEffectFile::k_ConstVector0, (const float*)&Vector4::ZERO);
			}
			if(restoreUVRgbAnim)
			{
				pEffect->setBool(CEffectFile::k_bBoolean7,FALSE);
			}
			if(fLastAlpha != 1.f)
			{
				pEffect->setFloat(CEffectFile::k_opacity, 1.f);
			}
		}
	}
}

