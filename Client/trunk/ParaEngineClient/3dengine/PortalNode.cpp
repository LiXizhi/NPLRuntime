//-----------------------------------------------------------------------------
// Class:	CPortalNode
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.9.9
//-----------------------------------------------------------------------------

/** Portal rendering algorithm in ParaEngine

I used a simplified version of portal rendering, because we are dealing  mostly with user generated dynamic scenes. 

The portal scene optimization process is this:
- first, the user create many static meshes in the scene without using portals.
- then, it optimize the scene by adding zones at enclosed areas (like indoor rooms), each zone has a defining convex volume and scene nodes inside it. 
	Zones without portals behave the same as normal static mesh in quad tree space. 
- third, it add portals to the scene, and for each zone, define all portals to connecting to it. 
- finally, saves all mesh, portals, zones and their relationships to disk

The portal scene loading process is this: 
- load all meshes, portals, and zones. and load their relationships from tile on load script.

The portal scene rendering process is this: 
- get all visible objects in the current view frustum in several post render queues.
	such as characterPool, bigMeshPool, smallMeshPool, portalPool, zonePool,
	Note: if object has a home zone, it only needs to be tested roughly against current view frustum.
- For each character in characterPool
		if(character has a home zone and still inside the home zone) then
		else
			add the character to its containing zoom by testing against all zones in the zonePool.
		end
  end
- Find the zone where camera eye position is, by testing all zones in zonePool. if it eye is not in any zone, make zone(NULL, infiniteFrustum). and call RenderZone(zone, infiniteFrustum(eyePosition)) Function
	function RenderZone(zone, PCFrustum) 
		PE_ASSERT(zone)
		for each object(character, mesh) in the pools
			if(object.m_nOccluded>=0 and zone == object.homeZone) then
				if(object intersects with or inside the PCFrustum) then
					object.m_nOccluded = -1;
				elseif(object.m_nOccluded == 0) then
					object.m_nOccluded = 1;
				end
			end
		end
		
		zone.m_nOccluded = -1;
		for each portal in zone 
			if(portal intersects with or inside the PCFrustum) then
				local pTargetZone = portal.GetZone(zone);
				if(pTargetZone==0 or pTargetZone.m_nOccluded==0) then
					local newPCFrustum = PCFrustum + 4 * portal planes.
					RenderZone(pTargetZone, newPCFrustum)
				end
			end
		end
	end

LIMITATIONS:
	1. only the outer world zone can contain other zones
	2. the camera can see into a room via at most one portal. in other words, there can not be two doors in to the same room with positive dot product. 
		if there have two such doors, one of the door contents are ignored which renders the result undefined. 
*/
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "BaseCamera.h"
#include "StringHelper.h"
#include "util/regularexpression.h"
#include "ZoneNode.h"
#include "PortalNode.h"

using namespace ParaEngine;

CPortalNode::CPortalNode(void)
	:m_bOpen(true), m_LocalsUpToDate(false)
{
	// create a default door.
	m_Corners[0] = Vector3(-0.5f,0,0);
	m_Corners[1] = Vector3(-0.5f,1.f,0);
	m_Corners[2] = Vector3(0.5f,1.f,0);
	m_Corners[3] = Vector3(0.5f,0,0);

	SetMyType(_PC_Portal);
	m_dwAttribute = OBJ_VOLUMN_FREESPACE;
}

CPortalNode::~CPortalNode(void)
{
}

std::string ParaEngine::CPortalNode::ToString( DWORD nMethod )
{
#ifndef MAX_LINE
#define MAX_LINE	500
#endif
	/** -- sample script generated:
	player = ParaScene.CreatePortal("portal_001", "zone_000", "zone_001", 255, 0, 255, "-1,0,0;-1,1,0;1,1,0;1,0,0;");
	ParaScene.Attach(player);
	*/
	string sScript;
	char line[MAX_LINE+1];

	/*string sHomeZone, sTargetZone;
	CZoneNode* pZone = GetZone(NULL);
	if(pZone)
	{
		sHomeZone = pZone->GetName();
		pZone = GetZone(pZone);
		if(pZone)
		{
			sTargetZone = pZone->GetName();
		}
	}*/
	Vector3 vPos = GetPosition();
	snprintf(line, MAX_LINE, "player = ParaScene.CreatePortal(\"%s\", \"%s\", \"%s\",\"", 
		GetName().c_str(), GetHomeZone(), GetTargetZone());
	sScript.append(line);

	sScript.append(GetPortalPoints());
	
	snprintf(line, MAX_LINE, "\",%f,%f,%f,%f);\nplayer:SetPosition(%f,%f,%f);ParaScene.Attach(player);\n", 
		GetWidth(), GetHeight(), GetDepth(), GetFacing(), vPos.x, vPos.y, vPos.z);
	sScript.append(line);
	return sScript;
}

void ParaEngine::CPortalNode::Open()
{
	m_bOpen = true;
}

void ParaEngine::CPortalNode::Close()
{
	m_bOpen = false;
}

Vector3 & ParaEngine::CPortalNode::getCorner( int nIndex )
{
	return m_Corners[nIndex];
}
void ParaEngine::CPortalNode::SetCorner( int nIndex, const Vector3& vPos )
{
	m_Corners[nIndex] = vPos;
}

Vector3 & ParaEngine::CPortalNode::getDerivedCorner( int nIndex )
{
	return m_DerivedCorners[nIndex];
}

CZoneNode * ParaEngine::CPortalNode::GetZone( CZoneNode * pCurZone /*= NULL*/ )
{
	int nSize = GetZoneCount();
	for (int i=0;i<nSize;++i)
	{
		RefListItem* pZone = GetRefObject(i);
		if(pZone && pZone->m_object!=pCurZone)
		{
			return (CZoneNode*) (pZone->m_object);
		}
	}
	return NULL;
}

int ParaEngine::CPortalNode::GetZoneCount()
{
	return GetRefObjNum();
}

HRESULT ParaEngine::CPortalNode::Draw( SceneState * sceneState )
{
	Vector3 vPos = GetRenderOffset();

	DWORD color = 0xffff00ff;
	static const short pIndexBuffer[] = {
		0,1,1,2,2,3,3,0, 
		2,0,1,3,
		4,5
	};

	LINEVERTEX pVertices[6];
	int nNumVertices = 6;

	for(int i=0;i<4;++i)
	{
		pVertices[i].p = m_Corners[i]+vPos;
		pVertices[i].color = color;
	}
	int nLenCount = 7;

	{
		// get the portal normal for direction rendering (a 0.4 meter line from the center of the portal along the portal direction)
		pVertices[4].p = (pVertices[2].p + pVertices[0].p)/2;
		pVertices[4].color = color;

		Vector3 Edge0 = pVertices[1].p - pVertices[0].p;
		Vector3 Edge1 = pVertices[2].p - pVertices[0].p;
		Vector3 n;
		n = Edge0.crossProduct(Edge1);
		ParaVec3Normalize(&n, &n);
		pVertices[5].p = pVertices[4].p + n * 0.4f;
		pVertices[5].color = color;
	}

#ifdef USE_DIRECTX_RENDERER
	// set render state
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
	if(pd3dDevice)
	{
		DrawBoundingBox(sceneState, color);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		pd3dDevice->SetMaterial((D3DMATERIAL9*)&(sceneState->GetGlobalMaterial()));
		pd3dDevice->SetTexture(0,NULL);

		pd3dDevice->SetTransform(D3DTS_WORLD, CGlobals::GetIdentityMatrix()->GetConstPointer());
		pd3dDevice->SetFVF(LINEVERTEX::FVF);

		RenderDevice::DrawIndexedPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN,D3DPT_LINELIST, 0, 
			nNumVertices, nLenCount, pIndexBuffer, D3DFMT_INDEX16,pVertices, sizeof(LINEVERTEX));
	}
#endif
	return S_OK;
}

Vector3 & ParaEngine::CPortalNode::getDerivedDirection( void )
{
	return m_DerivedDirection;
}

Vector3 & ParaEngine::CPortalNode::getDerivedCP( void )
{
	return m_DerivedCP;
}

void ParaEngine::CPortalNode::calcDirectionAndRadius( void )
{
	Vector3 radiusVector;
	Vector3 side1, side2;
	
	// first calculate local direction
	side1 = m_Corners[1] - m_Corners[0];
	side2 = m_Corners[2] - m_Corners[0];
	m_Direction = side1.crossProduct(side2);
	m_Direction.normalise();
	
	// calculate local cp
	m_LocalCP = Vector3(0,0,0);
	for (int i=0;i<4;i++)
	{
		m_LocalCP += m_Corners[i];
	}
	m_LocalCP *= 0.25f;


	// locals are now up to date
	m_LocalsUpToDate = true;
}

void ParaEngine::CPortalNode::updateDerivedValues()
{
	// make sure local values are up to date
	if (!m_LocalsUpToDate)
	{
		calcDirectionAndRadius();
	}
	int numCorners = 4;

	// to the render coordinate system. 
	Vector3 vPos = GetRenderOffset();

	for (int i=0;i<numCorners;i++)
	{
		m_DerivedCorners[i] =  m_Corners[i] + vPos;
	}
	m_DerivedDirection = m_Direction;
	m_DerivedCP = (m_DerivedCorners[0] + m_DerivedCorners[2])/2;
}

void ParaEngine::CPortalNode::GetVertices( Vector3 * pVertices, int* nNumber )
{
	*nNumber = 4;
	for (int i=0;i<4;i++)
	{
		pVertices[i] =  m_Corners[i] + m_vPos;
	}
}

void ParaEngine::CPortalNode::UpdateFrameNumber( int nFrameNumber )
{
	CBaseObject::UpdateFrameNumber(nFrameNumber);
	updateDerivedValues();
}

void ParaEngine::CPortalNode::SetHomeZone( const char* sName )
{
	if(sName == NULL || sName[0]=='\0')
	{
		CZoneNode* pNode = NULL;
		int nSize = GetZoneCount();
		for (int i=0;i<nSize;++i)
		{
			RefListItem* pZone = GetRefObject(i);
			if(pZone && (pZone->m_tag==1))
			{
				pNode = (CZoneNode*)(pZone->m_object);
			}
		}
		if(pNode)
		{
			pNode->removePortal(this);
		}
	}
	else
	{
		CZoneNode* pNode = CGlobals::GetScene()->CreateGetZoneNode(sName);
		pNode->addPortal(this);

		// remove redundant nodes
		RefListItem* pZone = NULL;
		while ((pZone=GetRefObject(2))!=0)
			((CZoneNode*)(pZone->m_object))->removePortal(this);

		// set the home and target node tag
		int nSize = GetZoneCount();
		for (int i=0;i<nSize && ((pZone=GetRefObject(i))!=0);++i)
		{
			pZone->m_tag = (pZone->m_object==pNode) ? 1:2;
		}
	}
}

const char* ParaEngine::CPortalNode::GetHomeZone()
{
	CZoneNode* pNode = NULL;
	
	// tag==1
	int nSize = GetZoneCount();
	for (int i=0;i<nSize;++i)
	{
		RefListItem* pZone = GetRefObject(i);
		if(pZone && (pZone->m_tag==1 || pNode==NULL))
		{
			pNode = (CZoneNode*)(pZone->m_object);
		}
	}

	if(pNode)
		return pNode->GetName().c_str();
	return CGlobals::GetString().c_str();
}

void ParaEngine::CPortalNode::SetTargetZone( const char* sName )
{
	if(sName == NULL || sName[0]=='\0')
	{
		CZoneNode* pNode = NULL;
		int nSize = GetZoneCount();
		for (int i=0;i<nSize;++i)
		{
			RefListItem* pZone = GetRefObject(i);
			if(pZone && (pZone->m_tag==2))
			{
				pNode = (CZoneNode*)(pZone->m_object);
			}
		}
		if(pNode)
		{
			pNode->removePortal(this);
		}
	}
	else
	{
		CZoneNode* pNode = CGlobals::GetScene()->CreateGetZoneNode(sName);
		pNode->addPortal(this);

		// remove redundant nodes
		RefListItem* pZone = NULL;
		while ((pZone=GetRefObject(2))!=0)
			((CZoneNode*)(pZone->m_object))->removePortal(this);

		// set the home and target node tag
		int nSize = GetZoneCount();
		for (int i=0;i<nSize && ((pZone=GetRefObject(i))!=0);++i)
		{
			pZone->m_tag = (pZone->m_object==pNode) ? 2:1;
		}
	}
}

const char* ParaEngine::CPortalNode::GetTargetZone()
{
	CZoneNode* pNode = NULL;

	// tag==2
	int nSize = GetZoneCount();
	if(nSize>=2)
	{
		for (int i=nSize-1;i>=0;--i)
		{
			RefListItem* pZone = GetRefObject(i);
			if(pZone && (pZone->m_tag==2 || pNode==NULL))
			{
				pNode = (CZoneNode*)(pZone->m_object);
			}
		}
	}

	if(pNode)
		return pNode->GetName().c_str();
	return CGlobals::GetString().c_str();
}

void ParaEngine::CPortalNode::SetPortalPoints( const char* sVertices )
{
	if(sVertices && sVertices[0]!='\0')
	{
		// not really tested, after porting to C++ 11 regex
		regex re("(\\-?[\\d\\.]+),(\\-?[\\d\\.]+),(\\-?[\\d\\.]+);");
		string strVertices = sVertices;
		sregex_iterator iter(strVertices.begin(), strVertices.end(), re), iterEnd;
		int i = 0;
		for (auto it = iter; it != iterEnd; it++)
		{
			smatch result = *it;
			string sX = result.str(1);
			string sY = result.str(2);
			string sZ = result.str(3);
			Vector3 vPt((float)StringHelper::StrToFloat(sX.c_str()), (float)StringHelper::StrToFloat(sY.c_str()), (float)StringHelper::StrToFloat(sZ.c_str()));
			if (i < 4)
				SetCorner(i++, vPt);
		}
		calcDirectionAndRadius();

	}
	else
	{
		AutoGenPortalPoints();
	}
}

const char* ParaEngine::CPortalNode::GetPortalPoints()
{
	static string g_result;
	g_result.clear();
	char line[MAX_LINE+1];
	for (int i=0;i<4;++i)
	{
		Vector3 v = getCorner(i);
		snprintf(line, MAX_LINE, "%f,%f,%f;", v.x,v.y,v.z);
		g_result.append(line);
	}
	return g_result.c_str();
}

// it will generate 4 portal points according to the shape of the bounding box. it will automatically use the thinnest side as the portal plane. 
void ParaEngine::CPortalNode::AutoGenPortalPoints()
{
	int nNumber = 4;
	Vector3 vPt[4];
	float x,y,z,facing;
	GetBoundingBox(&x,&y,&z,&facing);
	if( x>z && y>z ) 
	{
		vPt[0] = Vector3(-0.5f,0,0);
		vPt[1] = Vector3(-0.5f,1.f,0);
		vPt[2] = Vector3(0.5f,1.f,0);
		vPt[3] = Vector3(0.5f,0,0);
		for(int i=0;i<4;++i)
		{
			vPt[i].x *= x;
			vPt[i].y *= y;
		}
	}
	else if(x>y && z>y)
	{
		vPt[0] = Vector3(-0.5f,0,-0.5f);
		vPt[1] = Vector3(-0.5f,0,0.5f);
		vPt[2] = Vector3(0.5f,0,0.5f);
		vPt[3] = Vector3(0.5f,0,-0.5f);
		for(int i=0;i<4;++i)
		{
			vPt[i].x *= x;
			vPt[i].z *= z;
		}
	}
	else if( y>x && z>x ) 
	{
		vPt[0] = Vector3(0,0,-0.5f);
		vPt[1] = Vector3(0,1.f,-0.5f);
		vPt[2] = Vector3(0,1.f,0.5f);
		vPt[3] = Vector3(0,0,0.5f);
		for(int i=0;i<4;++i)
		{
			vPt[i].y *= y;
			vPt[i].z *= z;
		}
	}

	if(facing != 0)
	{
		Matrix4 m;
		ParaMatrixRotationY( &m, facing );
		for(int i=0; i<nNumber;i++)
		{
			ParaVec3TransformCoord( &vPt[i], &vPt[i], &m);
		}
	}

	for(int i=0; i<nNumber;i++)
	{
		SetCorner(i, vPt[i]);
	}

	calcDirectionAndRadius();
}

bool CPortalNode::TestCollisionSphere(CBaseCamera* pCamera, float fFarPlaneDistance)
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

bool CPortalNode::TestCollision(CBaseCamera* pCamera)
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
			if (pFrustum->GetPlane(iPlane).PlaneDotCoord((pVecBounds[iPoint])) > 0.f)
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

int ParaEngine::CPortalNode::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CTileObject::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass!=NULL);
	pClass->AddField("homezone", FieldType_String, (void*)SetHomeZone_s, (void*)GetHomeZone_s, NULL, NULL, bOverride);
	pClass->AddField("targetzone", FieldType_String, (void*)SetTargetZone_s, (void*)GetTargetZone_s, NULL, NULL, bOverride);
	pClass->AddField("portalpoints", FieldType_String, (void*)SetPortalPoints_s, (void*)GetPortalPoints_s, NULL, NULL, bOverride);
	pClass->AddField("AutoGenPortalPoints", FieldType_void, (void*)AutoGenPortalPoints_s, NULL, NULL, NULL, bOverride);

	return S_OK;
}