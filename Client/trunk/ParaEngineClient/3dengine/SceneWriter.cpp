//-----------------------------------------------------------------------------
// Class:	CSceneWriter
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.2
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ShapeAABB.h"
#include "AssetEntity.h"
#include "TerrainTileRoot.h"
#include "SceneWriter.h"
#include "memdebug.h"
using namespace ParaEngine;

#define MAX_BUFFER_SIZE		2048

CSceneWriter::CSceneWriter(CParaFile& file)
:m_file(file), m_buffer(NULL), m_nbufferSize(0),m_nMaxBufferSize(MAX_BUFFER_SIZE), m_nEncoding(NPL_TEXT), m_bInsideManagedLoader(false)
{
	
}

CSceneWriter::~CSceneWriter(void)
{
	SAFE_DELETE(m_buffer);
}

void CSceneWriter::Close()
{
	Flush();
	m_file.close();
}

void CSceneWriter::Flush()
{
	if(m_nbufferSize>0){
		m_file.write(m_buffer, m_nbufferSize);
		m_nbufferSize = 0;
	}
}


DWORD CSceneWriter::GetEncoding(){
	return m_nEncoding;
}


void CSceneWriter::SetEncoding(DWORD dwEncoding){
	m_nEncoding = dwEncoding;
}


void CSceneWriter::Write(const string& str)
{
	int nSize = (int)(str.size());
	if(nSize>0)
		Write(str.c_str(), nSize);
}

void CSceneWriter::Write(const char* pData, int nSize)
{
	if(nSize>m_nMaxBufferSize)
	{
		m_file.write(pData, nSize);
	}
	else
	{
		if((m_nbufferSize + nSize)>m_nMaxBufferSize)
			Flush();
		if(m_buffer==0)
			m_buffer = new char[m_nMaxBufferSize];

		if(m_buffer!=0)
		{
			memcpy(m_buffer+m_nbufferSize, pData, nSize);
			m_nbufferSize+=nSize;
		}
	} 
}

void CSceneWriter::BeginManagedLoader(const string& sLoaderName)
{
	const char sManagedLoader1[] = "local sceneLoader = ParaScene.GetObject(\"<managed_loader>";
	const char sManagedLoader2[] = "\");\r\n\
if (sceneLoader:IsValid() == true) then \r\n\
	ParaScene.Attach(sceneLoader);\r\n\
	return\r\n\
end\r\n\
sceneLoader = ParaScene.CreateManagedLoader(\"";
const char sManagedLoader3[] = "\");\r\n\
local player, asset, playerChar,att;\r\n\
local cpmesh=ParaScene.CreateMeshPhysicsObject;\r\n";

	Write(sManagedLoader1, sizeof(sManagedLoader1)-1);
	Write(sLoaderName);
	Write(sManagedLoader2, sizeof(sManagedLoader2)-1);
	Write(sLoaderName);
	Write(sManagedLoader3, sizeof(sManagedLoader3)-1);
	Flush();
}

void CSceneWriter::EndManagedLoader()
{
	const char sManagedLoader1[] = "ParaScene.Attach(sceneLoader);\r\n";
	Write(sManagedLoader1, sizeof(sManagedLoader1)-1);
	Flush();
}

/** whether the AABB contains the point. Only test the x,z components*/
inline bool AABBContainsPoint2D(const CShapeAABB& aabb, const Vector3& vPt){
	return ( (aabb.GetMin(0) < vPt.x) && (aabb.GetMin(2) < vPt.z ) &&
		(aabb.GetMax(0) >= vPt.x) && (aabb.GetMax(2) >= vPt.z ));
}

void CSceneWriter::WriteRegion(const CShapeAABB& aabb)
{
	list<CBaseObject*> objList;
	queue_CTerrainTilePtr_Type queueTiles;
	CTerrainTile* pTile = CGlobals::GetScene()->GetRootTile();
	/// breadth first transversing the scene(the root tile is ignored)
	/// pTile is now the root tile. object attached to it are never rendered directly
	bool bQueueTilesEmpty = false;

	float fRegionRadius = aabb.GetSize();
	Vector3 vRegionCenter;
	aabb.GetCenter(vRegionCenter);

	while(bQueueTilesEmpty == false)
	{
		/// add other tiles
		for(int i=0; i<MAX_NUM_SUBTILE; i++)
		{
			if(pTile->m_subtiles[i])
			{
				/// rough culling algorithm using the quad tree terrain tiles
				/// test against a sphere round the eye
				if(pTile->m_subtiles[i]->TestCollisionSphere(& (vRegionCenter), fRegionRadius))
				{
					queueTiles.push( pTile->m_subtiles[i] );
				}		
			}
		}

		/// go down the quad tree terrain tile to render objects
		if(queueTiles.empty())
		{
			/// even we know that the tile is empty, we still need to see if there is anything in the queueNode for rendering
			/// so when both queue are empty, we can exit the main rendering transversing loop
			bQueueTilesEmpty = true;
		}
		else
		{
			/// We will now push objects in the current terrain tile to a queue object for further view clipping.
			pTile = queueTiles.front();
			queueTiles.pop();
			Vector3 vPos;
			{
				for (auto& pObject : pTile->m_listSolidObj)
				{
					if (pObject->IsPersistent())
					{
						vPos = pObject->GetPosition();
						if(AABBContainsPoint2D(aabb, vPos))
							objList.push_back(pObject);
					}
				}
			}
			{
				for (auto& pObject : pTile->m_listFreespace)
				{
					if (pObject->IsPersistent())
					{
						vPos = pObject->GetPosition();
						if(AABBContainsPoint2D(aabb, vPos))
							objList.push_back(pObject);
					}
				}
			}
		}
	}
	
	/// sort by asset
	objList.sort(CmpBaseObjectAsset<CBaseObject*>());
	

	/// write objects
	const char sManagedLoader1[] = "	asset = ParaAsset.LoadStaticMesh(\"\", \"";
	const char sManagedLoader2[] = "\");\r\n";
	
	AssetEntity * pLastAsset = NULL;

	list< CBaseObject* >::iterator itCurCP, itEndCP = objList.end();
	for( itCurCP = objList.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		CBaseObject* pObj = (*itCurCP);
		DWORD dwType =  (DWORD)pObj->GetType();
		if(pLastAsset!=pObj->GetPrimaryAsset())
		{
			pLastAsset = pObj->GetPrimaryAsset();
			if(pLastAsset!=NULL && pLastAsset->GetType() == AssetEntity::mesh)
			{
				Write(sManagedLoader1, sizeof(sManagedLoader1)-1);
				Write(pLastAsset->GetKey());
				Write(sManagedLoader2, sizeof(sManagedLoader2)-1);
			}
		}
		
		switch(dwType)
		{
		case CBaseObject::MeshPhysicsObject:
			Write(pObj->ToString(CBaseObject::NPL_CREATE_IN_LOADER|CBaseObject::NPL_DONOT_OUTPUT_ASSET));
			break;
		case CBaseObject::MeshObject:
			Write(pObj->ToString(CBaseObject::NPL_CREATE_IN_LOADER|CBaseObject::NPL_DONOT_OUTPUT_ASSET));
			break;
		case CBaseObject::LightObject:
		case CBaseObject::ZoneNode:
		case CBaseObject::PortalNode:
			Write(pObj->ToString(CBaseObject::NPL_CREATE_IN_LOADER|CBaseObject::NPL_DONOT_OUTPUT_ASSET));
			break;
		case CBaseObject::BipedObject:
			// Write(pObj->ToString(CBaseObject::NPL_CREATE_IN_LOADER|CBaseObject::NPL_DONOT_OUTPUT_ASSET));
			// TODO: only export static biped object. 
			break;
		default:
			OUTPUT_LOG("warning: unknown object type %d during scene writing.\n", dwType);
			break;
		}
	}

	Flush();
}
