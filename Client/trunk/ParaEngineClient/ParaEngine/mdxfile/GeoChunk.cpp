//----------------------------------------------------------------------
// Class:	CGeoChunk
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer
/**
Fix0.1:dynamic array allocation
Fix0.2: replaceable texture(team glow) is implemented.
Fix0.3: a single dynamic buffer is used to render animation meshes. This buffer
is managed centrally by the ParaEngine Asset manager.
*/

#include "DxStdAfx.h"
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "GeoChunk.h"
#include "MDXModel.h"
#include "math.h"
#include "ShadowVolume.h"
#include "EdgeBuilder.h"
#include "mdx.h"
#include "ParaEngineXFileDef.h"
#include "memdebug.h"
using namespace ParaEngine;

extern float myEPSILON;

CGeoChunk::CGeoChunk()
	:matrixes(NULL),
	AnimAlphas(NULL),
	vertices(NULL),
	normals(NULL),
	triangles(NULL),
	groups(NULL),
	vertexGroups(NULL),
	matrixGroups(NULL),
	animExtents(NULL),
	uvs(NULL)
{
}

CGeoChunk::~CGeoChunk(void)
{
	if(matrixes!=NULL)
		delete [] matrixes;

	if(AnimAlphas!=NULL)
		delete [] AnimAlphas;

	if(normals!=NULL)
		delete [] normals;
	if(uvs!=NULL)
		delete [] uvs;
	if(triangles!=NULL)
		delete [] triangles;
	if(vertexGroups!=NULL)
		delete [] vertexGroups;
	if(groups!=NULL)
		delete [] groups;
	if(matrixGroups!=NULL)
		delete [] matrixGroups;
	if(animExtents!=NULL)
		delete [] animExtents;
	if(vertices != NULL)
		delete [] vertices;
}

void CGeoChunk::CalcGroupMatrix(CMDXModel *model)
{
	if(matrixes==NULL){
		matrixes=new D3DXMATRIX[numGroups];
		AnimAlphas=new float[numGroups];
	}

	int index=0;
	for(int i=0;i<numGroups;i++)
	{
		int matrixCount=groups[i];
		model->skeleton.GetTransformMatrix(matrixes[i],AnimAlphas[i],&matrixGroups[index],matrixCount);
		/*if(AnimAlphas[i]<0.5f)
			AnimAlphas[i]=AnimAlphas[i];*/
		index+=matrixCount;
	}
}

void CGeoChunk::Read(TypePointer inP,int inSize)
{
	TypePointer p(inP);

	int n = 0;

	while((p.c < inP.c+inSize) && (n<8))
	{
		switch(TAG(*p.dw))
		{
		case 'VRTX': // vertex
			{
				p.dw++;
				numVertices = *p.dw++;
				vertices = new D3DXVECTOR3[numVertices];
				memcpy(vertices, (D3DXVECTOR3*)p.c, sizeof(D3DXVECTOR3)*numVertices);
				p.c += numVertices*12; 
				n++;
			}
			break;

		case 'NRMS': // normal
			{
				p.dw++;
				int NumNormals = *p.dw++;

				if(numVertices != NumNormals)
				{
					return;
				}
				normals = new D3DXVECTOR3[NumNormals];
				memcpy(normals, (D3DXVECTOR3*)p.c, sizeof(D3DXVECTOR3)*NumNormals);
				
				p.c += NumNormals*12; 
				n++;
			}
			break;

		case 'PTYP': // PLANE_TYPE
			{
				p.dw++;
				int size = *p.dw++;

				if(*p.dw != 4) // == Triangles
				{
					return;
				}

				p.c += size*4; 
				++n;
			}
			break;

		case 'PCNT': // primitives count
			{
				p.dw++;
				int size = *p.dw++;

				if(size == 1)
					numTriangles = (*p.dw++)/3;
				else
				{
					numTriangles = size;
					p.c += size*4;
					//p.dw++;
				}

				n++;
			}
			break;

		case 'PVTX': // primitives vertices
			{
				p.dw++;
				int size = *p.dw++;
				triangles = new short[size];
				memcpy(triangles, (short*)p.p, sizeof(short)*size);

				if(size/3 != numTriangles)
				{
					return;
				}

				p.c += size*sizeof(short);
				n++;
			}
			break;

		case 'GNDX': // vertex group indices
			// links every vertex to a matrix
			{
				p.dw++;
				int size = *p.dw++;
				if(numVertices != size)
				{
					return;
				}

				vertexGroups = new char[size];
				memcpy(vertexGroups, (char*)p.c, size);

				p.c += size; 
				n++;
			}
			break;

		case 'MTGC': // group matrix counts
			// this is the number of vertices defined by GNDX for each matrix
			{
				p.dw++;
				numGroups = *p.dw++;
				groups = new int[numGroups];
				memcpy(groups, p.c, numGroups*4);

				p.c += numGroups*4; 
				n++;
			}
			break;

		case 'MATS': // matrices
			{
				p.dw++;
				numMatrixGroups = *p.dw++;
				matrixGroups = new int[numMatrixGroups];
				memcpy(matrixGroups, (int*)p.p, numMatrixGroups*4);

				p.c += numMatrixGroups*4;
				n++;
			}
			break;
		default:
			{
				p.dw++;
				int size = *p.dw++;
				p.c += size; 
			}
			break;
		}
	}

	if(p.c < inP.c+inSize)
	{
		materialID = *p.dw++;
		*p.dw++;
		*p.dw++;

		boundsRadius=*p.f++;
		mins = *((D3DXVECTOR3*)p.p);
		p.c+=sizeof(D3DXVECTOR3);
		maxs = *((D3DXVECTOR3*)p.p);
		p.c+=sizeof(D3DXVECTOR3);

		numAnimExtent=*p.dw++;

		animExtents=(AnimExtent*)p.p;
		animExtents = new AnimExtent[numAnimExtent];
		memcpy(animExtents, p.c, numAnimExtent*sizeof(AnimExtent));

		p.c+=numAnimExtent*sizeof(AnimExtent);

		if(TAG(*p.dw) == 'UVAS' )
		{
			p.dw++;
			*p.dw++; // no skip

			if(TAG(*p.dw) == 'UVBS' )
			{
				p.dw++;
				int size = *p.dw++;

				uvs = new D3DXVECTOR2[size];
				memcpy(uvs, p.c, size*8);

				p.c += size*8;
			}
			else
				return;
		}
		else
			return;
	}
	else
		return;

}

D3DXVECTOR3* CGeoChunk::GetMaximumExtent()
{
	return &maxs;
}
D3DXVECTOR3* CGeoChunk::GetMinimumExtent()
{
	return &mins;
}

void CGeoChunk::Render(CMDXModel *model,const AnimInfo& animInfo)
{
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
	CMaterial* mat = model->materialMap.materials[materialID<0 ? 0 : materialID];

	CalcGroupMatrix(model);

	int numLayers = (int)mat->layers.size();
	for(int l=0;l<numLayers;l++)
	{
		// LiXizhi: turn off light except for the last layer
		// Is it a rule for all mdx models?
		//if(l<mat->numLayers-1)
		//	pd3dDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
		//else
		//	pd3dDevice->SetRenderState(D3DRS_LIGHTING,TRUE);

		float alpha=mat->getFrameAlpha(animInfo.currentFrame,l);

		if(alpha < myEPSILON)
			continue;
		else
		{
			if(myEPSILON == 0.0f)
				alpha = 1.0f;
		}
		if(AnimAlphas[vertexGroups[triangles[0]]] < myEPSILON)
		{
			continue;
		}

		LPDIRECT3DTEXTURE9 pTexture=model->texture.GetBindTexture(mat->layers[l]->textureID);

		// TODO: handle team glow here.
		// <see blp.cpp: LoadBlp for more information on replaceable bitmap>
		// replaceable bitmap are shared bitmaps. However, I do not use shared bitmap in Paraengine.
		// so in the ReplaceIDWithName(), The teamcolor and glow are simply set to NULL texture.
		//if(model->texture.bitmaps[mat->layers[l].textureID].replaceableID == 1) // ! team glow
		//{
		//	
		//}

		BOOL bHasTexture;
		if(pTexture==0)
			bHasTexture = false;
		else
			bHasTexture = true;

		pd3dDevice->SetTexture(0, pTexture);


		//(0:none;1:transparent;2:blend;3:additive)
		if(mat->layers[l]->filterMode == 1)
		// transparent
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x0000000BE);
			pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		}else
		if(mat->layers[l]->filterMode == 2)
		// blend
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	
		}else
		// additive
		if(mat->layers[l]->filterMode == 3 )
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		}
		else
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		}

		/* 
		* fill vertex buffer with new data and draw it 
		*/

		// the color of the model when no texture is specified;
		D3DXCOLOR colorModel = *(model->GetModelColor()); 

		my_vertex* vb_vertices;
		int nNumLockedVertice;
		int nNumFinishedVertice = 0;
		DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM_DIF);
		do
		{
			if( (nNumLockedVertice = pBufEntity->Lock((numTriangles*3 - nNumFinishedVertice), 
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice/3;
				int index;

				for(int i=0;i<nLockedNum;++i)
				{
					for(int k=0; k<3; ++k)
					{
						int nVB = 3*i+k;
						int n = nNumFinishedVertice+nVB;
						
						int v0=triangles[n];
						index = vertexGroups[v0];

						D3DXVec3TransformCoord(&vb_vertices[nVB].v,&vertices[v0],&matrixes[index]);
						D3DXVec3TransformNormal(&vb_vertices[nVB].n,&normals[v0],&matrixes[index]);

						vb_vertices[nVB].tu = uvs[v0].x;
						vb_vertices[nVB].tv = uvs[v0].y;

						vb_vertices[nVB].colour = (bHasTexture==FALSE ? colorModel : 
							D3DCOLOR_COLORVALUE(1.0f,1.0f, 1.0f, 1.0f));
					}
				}

				pBufEntity->Unlock();

				DirectXPerf::DrawPrimitive( pd3dDevice, DirectXPerf::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLELIST,pBufEntity->m_dwBase,nLockedNum);

				if((numTriangles*3 - nNumFinishedVertice) > nNumLockedVertice)
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

/**
Build the shadow volume for this mesh
*/
void CGeoChunk::BuildShadowVolume(CMDXModel *model,const AnimInfo& animInfo, ShadowVolume * pShadowVolume, LightParams* pLight, D3DXMATRIX* mxWorld)
{
	CMaterial* mat = model->materialMap.materials[materialID<0 ? 0 : materialID];

	CalcGroupMatrix(model);

	bool bCanCastShadow = false; // we will only cast shadows from chunks that does not contain blending or additive layers
	int numLayers = (int)mat->layers.size();
	for(int l=0;l<numLayers;l++)
	{
		float alpha=mat->getFrameAlpha(animInfo.currentFrame,l);

		if(alpha < myEPSILON)
			continue;
		else
		{
			if(myEPSILON == 0.0f)
				alpha = 1.0f;
		}
		if(AnimAlphas[vertexGroups[triangles[0]]] < myEPSILON)
		{
			continue;
		}
		
		//(0:none;1:transparent;2:blend;3:additive)
		if((mat->layers[l]->filterMode == 1) || (mat->layers[l]->filterMode == 0))
		{
			bCanCastShadow = true;
			break;
		}
	}
	if(!bCanCastShadow)
		return;

	/** build shadow volume based on this geochunk
	*/

	// TODO: light's direction relative to the object.
	// here we assume the light is a directional light.
	D3DXVECTOR3 vLight = pLight->Direction; 
	float fRange = pLight->Range;
	vLight = -vLight;
	/// transform the local matrices to global matrices.
	for(int k=0; k<numGroups; k++)
		matrixes[k] *= (*mxWorld);
	
    // Allocate a temporary edge list
	hash_set <EdgeHash, hash_compare_edge> m_edgeTable;

	DWORD dwNumFaces = numTriangles;
    DWORD dwNumEdges = 0;
	
	D3DXVECTOR3 * pVertices = NULL;
	DWORD dwNumVertices = 0;
	int nUseCap = model->m_nUseShadowCap | 
		((pShadowVolume->m_shadowMethod == ShadowVolume::SHADOW_Z_FAIL)?1:0);
	if(nUseCap>0)
	{
		pShadowVolume->ReserveNewBlock(&pVertices, numTriangles*3);
	}
	
	// For each face
    for( DWORD i=0; i<dwNumFaces; i++ )
    {
        WORD wFace0 = triangles[3*i+0];
        WORD wFace1 = triangles[3*i+1];
        WORD wFace2 = triangles[3*i+2];

        D3DXVECTOR3 v0 = vertices[wFace0];
        D3DXVECTOR3 v1 = vertices[wFace1];
        D3DXVECTOR3 v2 = vertices[wFace2];
		
		// transform the vertices so that they are in world coordicate system.
		D3DXVec3TransformCoord(&v0,&v0,&matrixes[vertexGroups[wFace0]]);
		D3DXVec3TransformCoord(&v1,&v1,&matrixes[vertexGroups[wFace1]]);
		D3DXVec3TransformCoord(&v2,&v2,&matrixes[vertexGroups[wFace2]]);
		
        // Transform vertices or transform light?
		// we use vertice transform, it may be more accurate to use light transform
        D3DXVECTOR3 vCross1(v2-v1);
        D3DXVECTOR3 vCross2(v1-v0);
        D3DXVECTOR3 vNormal;
        D3DXVec3Cross( &vNormal, &vCross1, &vCross2 );

        if( D3DXVec3Dot( &vNormal, &vLight ) >= 0.0f )
        {
			CEdgeBuilder::AddEdge( m_edgeTable, dwNumEdges, wFace0, wFace1 );
			CEdgeBuilder::AddEdge( m_edgeTable, dwNumEdges, wFace1, wFace2 );
			CEdgeBuilder::AddEdge( m_edgeTable, dwNumEdges, wFace2, wFace0 );
			
			if(nUseCap>0)
			{
				pVertices[dwNumVertices++] = v0;
				pVertices[dwNumVertices++] = v2;
				pVertices[dwNumVertices++] = v1;
			}
	    }
    }

	if(nUseCap>0)
	{
		// commit shadow volume front cap vertices
		pShadowVolume->CommitBlock(dwNumVertices);
		dwNumVertices = 0;
		pVertices = NULL;
	}

	/** 
	build shadow volume for the edge array 
	Interestingly, the extrusion of geometries for point light sources and 
	infinite directional light sources are different. see below.
	*/
	
	if(pLight->bIsDirectional)
	{
		/**
		 infinite directional light sources would extrude all silhouette edges to 
		 a single point at infinity.
		*/
		pShadowVolume->ReserveNewBlock(&pVertices, dwNumEdges*3);
		D3DXVECTOR3 v3 = D3DXVECTOR3(mxWorld->_41, mxWorld->_42, mxWorld->_43) + pLight->Direction * pLight->Range;
		
		hash_set <EdgeHash, hash_compare_edge>::iterator itCurCP, itEndCP = m_edgeTable.end();
	
		// first shutdown all connections
		for( itCurCP = m_edgeTable.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			int index1 = (*itCurCP).m_v0;
			int index2 = (*itCurCP).m_v1;

			D3DXVECTOR3 v1 = vertices[index1];
			D3DXVECTOR3 v2 = vertices[index2];
			
			// transform the vertices so that they are in world coordicate system.
			D3DXVec3TransformCoord(&v1,&v1,&matrixes[vertexGroups[index1]]);
			D3DXVec3TransformCoord(&v2,&v2,&matrixes[vertexGroups[index2]]);
			
     		// Add a quad (two triangles) to the vertex list
			pVertices[dwNumVertices++] = v1;
			pVertices[dwNumVertices++] = v2;
			pVertices[dwNumVertices++] = v3;
		}
		pShadowVolume->CommitBlock(dwNumVertices);
	}
#ifdef POINTLIGHT_SUPPORT
	else // TODO: for point light, the listed method is not correct
	{
		/**
		Point light sources would extrude the silhouette edges exactly point for point 
		*/

		pShadowVolume->ReserveNewBlock(&pVertices, dwNumEdges*6);
		for( i=0; i<dwNumEdges; i++ )
		{
			int index1 = pEdges[2*i+0];
			int index2 = pEdges[2*i+1];

			D3DXVECTOR3 v1 = vertices[index1];
			D3DXVECTOR3 v2 = vertices[index2];
			
			// transform the vertices so that they are in world coordicate system.
			D3DXVec3TransformCoord(&v1,&v1,&matrixes[vertexGroups[index1]]);
			D3DXVec3TransformCoord(&v2,&v2,&matrixes[vertexGroups[index2]]);
			
			D3DXVECTOR3 v3 = v1 - vLight*fRange;
			D3DXVECTOR3 v4 = v2 - vLight*fRange;

     		// Add a quad (two triangles) to the vertex list
			pVertices[dwNumVertices++] = v1;
			pVertices[dwNumVertices++] = v2;
			pVertices[dwNumVertices++] = v3;

			pVertices[dwNumVertices++] = v2;
			pVertices[dwNumVertices++] = v4;
			pVertices[dwNumVertices++] = v3;
		}
		pShadowVolume->CommitBlock(dwNumVertices);
	}
#endif
	
}
