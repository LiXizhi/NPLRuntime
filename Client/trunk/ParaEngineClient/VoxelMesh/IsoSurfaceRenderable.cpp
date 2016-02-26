//-----------------------------------------------------------------------------
// Class:	IsoSurfaceRenderable
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "effect_file.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "IsoSurfaceBuilder.h"
#include "IsoSurfaceRenderable.h"

using namespace ParaEngine;


IsoSurfaceRenderable::IsoSurfaceRenderable()
: mSurfaceFlags(0)
{
}

void IsoSurfaceRenderable::initialize(IsoSurfaceBuilder *builder)
{
	mSurfaceFlags = builder->mSurfaceFlags;
	//DynamicRenderable::initialize(RenderOperation::OT_TRIANGLE_LIST, true);
}

void IsoSurfaceRenderable::createVertexDeclaration()
{
	//VertexDeclaration* vertexDeclaration = mRenderOp.vertexData->vertexDeclaration;
	//size_t offset = 0;

	//// Add mandatory position element to vertex declaration
	//mPositionElement = &vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_POSITION);
	//offset += VertexElement::getTypeSize(VET_FLOAT3);

	//if (mSurfaceFlags & IsoSurfaceBuilder::GEN_NORMALS)
	//{
	//	// Add optional normal element to vertex declaration
	//	mNormalElement = &vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
	//	offset += VertexElement::getTypeSize(VET_FLOAT3);
	//}

	//if (mSurfaceFlags & IsoSurfaceBuilder::GEN_VERTEX_COLOURS)
	//{
	//	// Add optional diffuse colour element to vertex declaration
	//	mDiffuseElement = &vertexDeclaration->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);
	//	offset += VertexElement::getTypeSize(VET_COLOUR);
	//}

	//if (mSurfaceFlags & IsoSurfaceBuilder::GEN_TEX_COORDS)
	//{
	//	// Add optional texture coordinates element to vertex declaration
	//	mTexCoordsElement = &vertexDeclaration->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	//	offset += VertexElement::getTypeSize(VET_FLOAT2);
	//}
}

void IsoSurfaceRenderable::fillHardwareBuffers(IsoSurfaceBuilder *builder)
{
	// Ensure that the hardware buffers are large enough
	PrepareBuffers((int)builder->mIsoVertices.size(), 3*(int)builder->mIsoTriangles.size());
	if(m_nVertexBufSize == 0)
		return;

	// Fill hardware vertex buffer
	if(lock())
	{
		{
			//
			// fill vertex buffer
			//

			Vector3* vPos = &(m_Positions[0]);
			Vector3* vNorm = &(m_Normals[0]);

			Vector2* pUV = &(m_UVs[0]);
			DWORD* pColor =  &(m_Colors[0]);

			IsoSurfaceBuilder::IsoVertexVector::iterator itEnd = builder->mIsoVertices.end();
			for (IsoSurfaceBuilder::IsoVertexVector::iterator itCur = builder->mIsoVertices.begin(); itCur != itEnd; ++itCur)
			{
				int nVertexIndex = *itCur;
				// Write position element
				*vPos = builder->mIsoVertexPositions[nVertexIndex];
				vPos++;
				
				if (mSurfaceFlags & IsoSurfaceBuilder::GEN_NORMALS)
				{
					// Write normal element
					*vNorm = builder->mIsoVertexNormals[nVertexIndex];
					vNorm++;
				}

				if (mSurfaceFlags & IsoSurfaceBuilder::GEN_VERTEX_COLOURS)
				{
					*pColor = builder->mIsoVertexColours[nVertexIndex];
					pColor++;
				}

				if (mSurfaceFlags & IsoSurfaceBuilder::GEN_TEX_COORDS)
				{
					// Write texture coordinates element
					*pUV = builder->mIsoVertexTexCoords[nVertexIndex];
					pUV++;
				}
			}
		}
		
		
		//
		// Fill index buffer
		//
		{
			unsigned short* pIndex = &(m_indices[0]);
			IsoSurfaceBuilder::IsoTriangleVector::iterator itEnd = builder->mIsoTriangles.end();
			for (IsoSurfaceBuilder::IsoTriangleVector::iterator itCur = builder->mIsoTriangles.begin(); itCur != itEnd; ++itCur)
			{
				*pIndex++ = static_cast<unsigned short>(builder->mIsoVertexIndices[itCur->vertices[0]]);
				*pIndex++ = static_cast<unsigned short>(builder->mIsoVertexIndices[itCur->vertices[1]]);
				*pIndex++ = static_cast<unsigned short>(builder->mIsoVertexIndices[itCur->vertices[2]]);
			}
		}
		unlock();
	}
	
	mAABB = builder->mDataGrid->getBoxSize();
}

void IsoSurfaceRenderable::deleteGeometry()
{
	/// ...and delete geometry.
}


HRESULT ParaEngine::IsoSurfaceRenderable::Render( SceneState * pSceneState, LPDIRECT3DDEVICE9 pd3dDevice )
{
	pd3dDevice->SetTransform(D3DTS_WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());

	DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
	pd3dDevice->SetStreamSource( 0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal) );

	if(m_texture != 0)
		pd3dDevice->SetTexture(0, m_texture->GetTexture());

	// render
	DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,0, m_nVertexBufSize, 0, m_nIndexBufSize/3);


	return S_OK;
}

HRESULT ParaEngine::IsoSurfaceRenderable::Render( SceneState * pSceneState, CEffectFile *pEffect )
{
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	/// apply surface materials
	bool bEnableLight = pSceneState->GetScene()->IsLightEnabled();
	if(bEnableLight)
	{
		ParaMaterial mat = pSceneState->GetGlobalMaterial();
		mat.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
		pEffect->applySurfaceMaterial(&mat);
	}
	else
	{
		ParaMaterial mat = pSceneState->GetGlobalMaterial();
		mat.Ambient = LinearColor(0.6f,0.6f,0.6f,1.f ); // shall we use ambient to simulate lighting, when lighting is disabled.
		mat.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
		pEffect->applySurfaceMaterial(&mat);
	}

	DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
	pd3dDevice->SetStreamSource( 0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal) );

	//////////////////////////////////////////////////////////////////////////
	// programmable pipeline
	if(pEffect->begin())
	{
		if(pEffect->BeginPass(0))
		{
			if(m_texture != 0)
				pEffect->setTexture(0, m_texture->GetTexture());

			pEffect->CommitChanges();
			// render
			DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,0, m_nVertexBufSize, 0, m_nIndexBufSize/3);

			pEffect->EndPass(0);
		}
		pEffect->end();	
	}
	return S_OK;
}

void IsoSurfaceRenderable::SetTexture( TextureEntity* pTexture )
{
	m_texture = pTexture;
}