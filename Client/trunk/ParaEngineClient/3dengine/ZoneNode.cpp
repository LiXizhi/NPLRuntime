//-----------------------------------------------------------------------------
// Class:	CZoneNode
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.9.9
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "PortalNode.h"
#include "BaseCamera.h"
#include "util/regularexpression.h"
#include "StringHelper.h"
#include "ZoneNode.h"

using namespace ParaEngine;

CZoneNode::CZoneNode(void)
{
	SetMyType(_PC_Zone);
	m_dwAttribute = OBJ_VOLUMN_FREESPACE;
}

CZoneNode::~CZoneNode(void)
{
	
}
std::string ParaEngine::CZoneNode::ToString( DWORD nMethod )
{
#ifndef MAX_LINE
#define MAX_LINE	500
#endif
	/** -- sample script generated:
	player = ParaScene.CreateZone("zone_001", 1, 255, 0, 255, "1,0,0;0,0,0;-1,0,0;0,1,0;0,0,1,0,0,-1");
	ParaScene.Attach(player);
	*/
	string sScript;
	char line[MAX_LINE+1];

	Vector3 vPos = GetPosition();
	snprintf(line, MAX_LINE, "player = ParaScene.CreateZone(\"%s\",\"", 
		GetName().c_str());
	sScript.append(line);

	sScript.append(GetZonePlanes());

	snprintf(line, MAX_LINE, "\",%f,%f,%f,%f);\nplayer:SetPosition(%f,%f,%f);ParaScene.Attach(player);\n", 
		GetWidth(), GetHeight(), GetDepth(), GetFacing(), vPos.x, vPos.y, vPos.z);
	sScript.append(line);
	return sScript;
}

void ParaEngine::CZoneNode::addNode( CBaseObject * pNode )
{
	if(pNode)
	{
		AddReference(pNode, 3);
	}
}

void ParaEngine::CZoneNode::removeNode( CBaseObject * pNode )
{
	if(pNode)
	{
		DeleteReference(pNode);
	}
}

void ParaEngine::CZoneNode::addPortal( CPortalNode * pPortal)
{
	if(pPortal)
	{
		// this is tricky, we will pre-pend portals to reference if tag is less than 0, otherwise it is append.
		AddReference(pPortal, -1);
	}
}

void ParaEngine::CZoneNode::removePortal( CPortalNode * pPortal)
{
	if(pPortal)
	{
		DeleteReference(pPortal);
	}
}

bool ParaEngine::CZoneNode::Contains( CBaseObject * pNode, bool bStrict )
{
	Vector3 pVecBounds[8];
	int nNumVertices;
	pNode->GetRenderVertices(pVecBounds, &nNumVertices);
	
	Vector3 vOffset = GetRenderOffset();
	// For each extra active culling plane, see if the entire aabb is on the negative side
	// If so, object is not visible
	vector<Plane>::const_iterator pit = m_planes.begin();
	while ( pit != m_planes.end() )
	{
		const Plane& plane = *pit;
		
		for (int i=0;i<nNumVertices;i++)
		{
			Vector3 vPos = pVecBounds[i] - vOffset;
			if (plane.getDistance(vPos)<0)
			{
				return false;
			}
		}
		pit++;
	}
	return true;
}

bool ParaEngine::CZoneNode::Contains( const Vector3& vPos )
{
	Vector3 vPt = vPos - m_vPos;
	// For each extra active culling plane, see if the point is on the negative side. If so, object is not visible
	vector<Plane>::const_iterator pit = m_planes.begin();
	while ( pit != m_planes.end() )
	{
		if ((*pit).getDistance(vPt)<0)
			return false;
		pit++;
	}
	return true;
}

HRESULT ParaEngine::CZoneNode::Draw( SceneState * sceneState )
{
	Vector3 vPos = GetRenderOffset();
	DWORD color = 0xff00ffff;

	DrawBoundingBox(sceneState, color);
	if(!m_planes.empty())
	{
		// render rays from origin to plane.
		static vector<LINEVERTEX> vertices;
		static vector<short> IndexBuffer;

		int nPlaneCount = (int)(m_planes.size());
		if((int)IndexBuffer.size() < nPlaneCount*2)
		{
			IndexBuffer.resize(nPlaneCount*2);
			for (int i=0;i<nPlaneCount;++i)
			{
				IndexBuffer[i*2] = 0;
				IndexBuffer[i*2+1] = i+1;
			}
		}
		if((int)vertices.size()<(nPlaneCount+1))
		{
			vertices.resize(nPlaneCount+1);
		}
		vertices[0].p = vPos;
		vertices[0].color = color;
		for(int i=0;i<nPlaneCount;++i)
		{
			vertices[i+1].p = vPos + m_planes[i].normal*(-m_planes[i].d);
			vertices[i+1].color = color;
		}
#ifdef USE_DIRECTX_RENDERER
		// set render state
		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		if(pd3dDevice)
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetMaterial((D3DMATERIAL9*)&(sceneState->GetGlobalMaterial()));
			pd3dDevice->SetTexture(0,NULL);

			pd3dDevice->SetTransform(D3DTS_WORLD, Matrix4::IDENTITY.GetConstPointer());
			pd3dDevice->SetFVF(LINEVERTEX::FVF);

			RenderDevice::DrawIndexedPrimitiveUP(CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN,D3DPT_LINELIST, 0, 
				nPlaneCount*2, nPlaneCount, &(IndexBuffer[0]), D3DFMT_INDEX16,&(vertices[0]), sizeof(LINEVERTEX));
		}
#endif
	}
	return S_OK;
}

DVector3 ParaEngine::CZoneNode::GetPosition()
{
	return m_vPos;
}

void ParaEngine::CZoneNode::SetPosition(const DVector3 & v)
{
	m_vPos = v;
}

void ParaEngine::CZoneNode::AutoGenZonePlanes()
{
	int nNumber = 4;
	float x,y,z,facing;
	GetBoundingBox(&x,&y,&z,&facing);
	Vector3 vPt[6];
	vPt[0] =  Vector3(1,0,0);
	vPt[1] =  Vector3(-1,0,0);
	vPt[2] =  Vector3(0,0,1);
	vPt[3] =  Vector3(0,0,-1);
	vPt[4] =  Vector3(0,-1,0);
	vPt[5] =  Vector3(0,1,0);

	if(facing != 0)
	{
		Matrix4 m;
		ParaMatrixRotationY( &m, facing );
		for(int i=0; i<nNumber;i++)
		{
			vPt[i] = vPt[i]*m;
		}
	}

	GetPlanes().clear();
	GetPlanes().push_back(Plane(vPt[0], x/2.f));
	GetPlanes().push_back(Plane(vPt[1], x/2.f));
	GetPlanes().push_back(Plane(vPt[2], z/2.f));
	GetPlanes().push_back(Plane(vPt[3], z/2.f));
	GetPlanes().push_back(Plane(vPt[4], y));
	GetPlanes().push_back(Plane(vPt[5], 0.f));
}

void ParaEngine::CZoneNode::AutoFillZone()
{
	// TODO: 
}

void ParaEngine::CZoneNode::SetZonePlanes( const char* sBoundingVolumes )
{
	if(sBoundingVolumes && sBoundingVolumes[0]!='\0')
	{
		// not really tested, after porting to C++ 11 regex
		regex re("(\\-?[\\d\\.]+),(\\-?[\\d\\.]+),(\\-?[\\d\\.]+);");
		std::string input(sBoundingVolumes);  

		GetPlanes().clear();

		sregex_iterator iter(input.begin(), input.end(), re), iterEnd;
		for (auto it = iter; it != iterEnd; it++)
		{
			smatch result = *it;
			string sX = result.str(1);
			string sY = result.str(2);
			string sZ = result.str(3);
			Vector3 vPt((float)StringHelper::StrToFloat(sX.c_str()), (float)StringHelper::StrToFloat(sY.c_str()), (float)StringHelper::StrToFloat(sZ.c_str()));
			Vector3 vNorm(0, 1.f, 0);
			float fDistance = vPt.squaredLength();
			if(fDistance > 0.01f)
			{
				fDistance = sqrtf(fDistance); 
				vNorm = vPt.normalisedCopy();
				// add negative sign so that it is always pointing inward. 
				vNorm = -vNorm;
			}
			else
			{
				fDistance = 0.f;
			}
			GetPlanes().push_back(Plane(vNorm, fDistance));
		}

	}
	else
	{
		AutoGenZonePlanes();
	}
}

const char* ParaEngine::CZoneNode::GetZonePlanes()
{
	static string g_result;
	g_result.clear();
	char line[MAX_LINE+1];
	vector<Plane>::const_iterator itCur, itEnd = GetPlanes().end();
	for (itCur = GetPlanes().begin(); itCur!=itEnd; ++itCur)
	{
		const Plane& plane = (*itCur);
		Vector3 v = -plane.normal*fabsf(plane.d);
		snprintf(line, MAX_LINE, "%f,%f,%f;", v.x,v.y,v.z);
		g_result.append(line);
	}
	return g_result.c_str();
}

bool CZoneNode::TestCollisionSphere(CBaseCamera* pCamera, float fFarPlaneDistance)
{
	Vector3 vCenter = GetRenderOffset();
	vCenter.y += GetHeight()/2.f;
	float fRadius = GetRadius();

	CCameraFrustum* pFrustum = pCamera->GetObjectFrustum();
	float fToNearPlane = pFrustum->GetPlane(0).PlaneDotCoord(vCenter);
	if( (fToNearPlane+fRadius+pCamera->GetNearPlane()) >= 0.f && fToNearPlane < (fFarPlaneDistance+fRadius) )
	{
		return (pFrustum->GetPlane(2).PlaneDotCoord(vCenter) >= -fRadius)
			&& (pFrustum->GetPlane(3).PlaneDotCoord(vCenter) >= -fRadius)
			&& (pFrustum->GetPlane(4).PlaneDotCoord(vCenter) >= -fRadius)
			&& (pFrustum->GetPlane(5).PlaneDotCoord(vCenter) >= -fRadius);
	}
	else
		return false;
}

bool CZoneNode::TestCollision(CBaseCamera* pCamera)
{
	PE_ASSERT(pCamera!=0);
	Vector3 pVecBounds[8];
	int nCount=0;
	GetRenderVertices(pVecBounds, &nCount);
	CCameraFrustum* pFrustum = pCamera->GetObjectFrustum();

	// near plane is shifted to eye position. This will allow the portal to pass the collision test when it is potentially visible.
	{
		int iPoint;
		for( iPoint = 0; iPoint < nCount; iPoint++ )
		{
			if ((pFrustum->GetPlane(0).PlaneDotCoord((pVecBounds[iPoint])) + pCamera->GetNearPlane()) > 0.f)
			{
				// the point is inside the plane
				break;
			}
		}
		if( iPoint>= nCount)
			return false;
	}

	// Check boundary vertices against all 6 frustum planes, 
	// and store result (1 if outside) in a bit field
	for( int iPlane = 1; iPlane < 6; iPlane++ )
	{
		int iPoint;
		for( iPoint = 0; iPoint < nCount; iPoint++ )
		{
			if (pFrustum->GetPlane(iPlane).PlaneDotCoord(pVecBounds[iPoint]) > 0.f)
			{
				// the point is inside the plane
				break;
			}
		}
		if( iPoint>= nCount)
			return false;
	}
	return true;
}

int ParaEngine::CZoneNode::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CBaseObject::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass!=NULL);
	pClass->AddField("zoneplanes", FieldType_String, (void*)SetZonePlanes_s, (void*)GetZonePlanes_s, NULL, NULL, bOverride);
	pClass->AddField("AutoGenZonePlanes", FieldType_void, (void*)AutoGenZonePlanes_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("AutoFillZone", FieldType_void, (void*)AutoFillZone_s, NULL, NULL, NULL, bOverride);
	return S_OK;
}