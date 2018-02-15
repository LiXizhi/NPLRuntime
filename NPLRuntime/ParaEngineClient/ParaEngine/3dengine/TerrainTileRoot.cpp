//-----------------------------------------------------------------------------
// Class:	CTerrainTileRoot
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedObject.h"
#include "SceneObject.h"
#include "BaseCamera.h"
#include "TerrainTileRoot.h"

using namespace ParaEngine;

CTerrainTileRoot::CTerrainTileRoot(void)
{

}

void CTerrainTileRoot::Cleanup()
{
	CTerrainTile::Cleanup();
	m_globalMeshNameMapping.clear();
}

/**
//------------------------------------------------------------------------------
// desc: get the smallest tile that contains the point (RECT)
// the root tile will not be attached any object with this object even it
// is logically belongs to it
// We will start searching for tiles in the level below the root tile
// (fAbsoluteX, fAbsoluteY). This function is called recursively on itself.
// input: fAbsoluteX, fAbsoluteY: center point of the point
//        fPtWidth, fPtHeight: the size of the point
// return: The tile is returned. this function will never return NULL.
sub tile and its locations.

^ Y
|______
|2 |3 |
-------
|0 |1 |
---------->x
*/
//------------------------------------------------------------------------------
CTerrainTile* CTerrainTileRoot::CreateTileByRect(FLOAT fAbsoluteX, FLOAT fAbsoluteY,
	FLOAT fPtWidth, FLOAT fPtHeight)
{
	/// the root tile will not be attached any object with this object even it
	/// is logically belongs to it
	/// We will start searching for tiles in the level below the root tile
	CTerrainTile* pTile = CreateSubTile(GetSubTileIndexByPoint(fAbsoluteX, fAbsoluteY));

	int nDepth = this->m_nDepth;

	float left = fAbsoluteX - fPtWidth / 2, top = fAbsoluteY - fPtHeight / 2,
		right = fAbsoluteX + fPtWidth / 2, bottom = fAbsoluteY + fPtHeight / 2;

	for (int n = 0; n<nDepth; n++)
	{
		float x1, y1, x2, y2;
		/* sub tile0 */
		x1 = pTile->m_fX - pTile->m_fRadius;
		y1 = pTile->m_fY - pTile->m_fRadius;
		x2 = pTile->m_fX;
		y2 = pTile->m_fY;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->CreateSubTile(0);
			continue;
		}
		/* sub tile1 */
		x1 += pTile->m_fRadius;
		y1 = y1;
		x2 += pTile->m_fRadius;
		y2 = y2;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->CreateSubTile(1);
			continue;
		}

		/* sub tile3 */
		x1 = x1;
		y1 += pTile->m_fRadius;
		x2 = x2;
		y2 += pTile->m_fRadius;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->CreateSubTile(3);
			continue;
		}

		/* sub tile2 */
		x1 -= pTile->m_fRadius;
		y1 = y1;
		x2 -= pTile->m_fRadius;
		y2 = y2;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->CreateSubTile(2);
			continue;
		}



		/* it should be attached to the current tile*/
		break;
	}

	return pTile;
}

CTerrainTile* CTerrainTileRoot::GetTileByRect(FLOAT fAbsoluteX, FLOAT fAbsoluteY,
	FLOAT fPtWidth, FLOAT fPtHeight)
{
	CTerrainTile* pTile = this;
	CTerrainTile* pTileFinal = pTile;

	int nDepth = this->m_nDepth;

	float left = fAbsoluteX - fPtWidth / 2, top = fAbsoluteY - fPtHeight / 2,
		right = fAbsoluteX + fPtWidth / 2, bottom = fAbsoluteY + fPtHeight / 2;

	while (pTile != NULL)
	{
		pTileFinal = pTile;
		float x1, y1, x2, y2;
		/* sub tile0 */
		x1 = pTile->m_fX - pTile->m_fRadius;
		y1 = pTile->m_fY - pTile->m_fRadius;
		x2 = pTile->m_fX;
		y2 = pTile->m_fY;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->m_subtiles[0];
			continue;
		}
		/* sub tile1 */
		x1 += pTile->m_fRadius;
		y1 = y1;
		x2 += pTile->m_fRadius;
		y2 = y2;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->m_subtiles[1];
			continue;
		}

		/* sub tile3 */
		x1 = x1;
		y1 += pTile->m_fRadius;
		x2 = x2;
		y2 += pTile->m_fRadius;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->m_subtiles[3];
			continue;
		}

		/* sub tile2 */
		x1 -= pTile->m_fRadius;
		y1 = y1;
		x2 -= pTile->m_fRadius;
		y2 = y2;

		if ((x1<left && left<x2 && y1<top && top<y2) &&
			(x1<right && right<x2 && y1<bottom && bottom<y2))
		{
			pTile = pTile->m_subtiles[2];
			continue;
		}

		/* it should be attached to the current tile*/
		break;
	}

	return pTileFinal;
}
CTerrainTile* CTerrainTileRoot::CreateTileByPoint(FLOAT fAbsoluteX, FLOAT fAbsoluteY)
{
	CTerrainTile* pTile = this;
	for (int n = 0; n <= m_nDepth; n++)
	{
		int nX = (int)(fAbsoluteX>pTile->m_fX) ? 1 : 0;
		int nY = (int)(fAbsoluteY>pTile->m_fY) ? 1 : 0;
		int nIndex = ((nY << 1) + nX);
		pTile = pTile->CreateSubTile(nIndex);
		PE_ASSERT(pTile != NULL);
	}
	return pTile;
}

CTerrainTile* CTerrainTileRoot::GetTileByPoint(FLOAT fAbsoluteX, FLOAT fAbsoluteY)
{
	CTerrainTile* pTile = this;
	CTerrainTile* pTileFinal = pTile;

	if (fAbsoluteX>0 && fAbsoluteY>0 && fAbsoluteX<(m_fX + m_fRadius) && fAbsoluteY<(m_fY + m_fRadius))
	{
		while (pTile != NULL)
		{
			pTileFinal = pTile;
			int nX = (int)(fAbsoluteX>pTile->m_fX) ? 1 : 0;
			int nY = (int)(fAbsoluteY>pTile->m_fY) ? 1 : 0;
			int nIndex = ((nY << 1) + nX);
			pTile = pTile->m_subtiles[nIndex];
		}
	}
	else
		return NULL;

	return pTileFinal;
}

CTerrainTile* CTerrainTileRoot::GetAdjacentTile(CTerrainTile* pTile, DIRECTION nDirection)
{
	int nY = (int)(nDirection / 3);
	int nX = nDirection - nY * 3;
	float fX = pTile->m_fX + ((nX - 1) << 1)*pTile->m_fRadius;
	float fY = pTile->m_fY + ((nY - 1) << 1)*pTile->m_fRadius;

	CTerrainTile* pNeighbourTile = GetTileByPoint(fX, fY);
	/*if(pNeighbourTile->m_fRadius!=pTile->m_fRadius){
	return pNeighbourTile;
	}
	return NULL;*/
	return pNeighbourTile;
}

CTerrainTile* CTerrainTileRoot::GetAdjacentTile(const Vector3& vPos, DIRECTION nDirection, float fRadius)
{
	if (fRadius <= 0.f)
		fRadius = m_fSmallestTileRadius;
	int nY = (int)(nDirection / 3);
	int nX = nDirection - nY * 3;
	float fX = vPos.x + ((nX - 1) << 1)*fRadius;
	float fY = vPos.z + ((nY - 1) << 1)*fRadius;

	CTerrainTile* pNeighbourTile = GetTileByPoint(fX, fY);
	return pNeighbourTile;
}

void CTerrainTileRoot::ResetTerrain(FLOAT fRadius, int nDepth)
{
	Cleanup();
	m_fRadius = fRadius;
	m_nDepth = nDepth;
	m_fX = fRadius;
	m_fY = fRadius;

	if (nDepth >= 0)
		m_fSmallestTileRadius = m_fRadius / powf(2.f, (float)(nDepth + 1));
	else
		m_fSmallestTileRadius = m_fRadius;
}

CTerrainTile* CTerrainTileRoot::AttachGlobalObject(CBaseObject* obj)
{
	if (obj != NULL)
	{
		m_listSolidObj.push_back(obj);
		// add to map list
		m_namemap[obj->GetIdentifier()] = obj;

		Vector3 vPos = obj->GetPosition();

		// object is always attached to the deepest terrain tile as a point
		// Note:2009.8.26: I used to use ViewCullingObject of obj to attach, but for async-loading, I will use just the object position.
		IGameObject* gameObj = obj->QueryIGameObject();
		CTerrainTile*  pTile = CreateTileByPoint(vPos.x, vPos.z);

		if (gameObj != NULL){
			gameObj->UpdateTileContainer();
			gameObj->On_Attached();
		}
		return obj->GetTileContainer();
	}
	return NULL;
}

CTerrainTile* CTerrainTileRoot::AttachLocalObject(CBaseObject* obj)
{
	if (obj == NULL)
		return NULL;
	Vector3 vPos = obj->GetPosition();

	// object is always attached to the deepest terrain tile as a point
	// Note:2009.8.26: I used to use ViewCullingObject of obj to attach, but for async-loading, I will use just the object position.
	IGameObject* gameObj = obj->QueryIGameObject();
	CTerrainTile*  pTile = NULL;
	pTile = CreateTileByPoint(vPos.x, vPos.z);

	if (pTile != NULL)
	{
		if (obj->CheckVolumnField(OBJ_BIG_STATIC_OBJECT))
		{
			float width, height, facing;
			obj->GetBoundRect(&width, &height, &facing);
			if (facing != 0.0f)
			{
				float radius = obj->GetRadius();
				height = width = radius * 2;
			}

			// for big object, we will attach to the best fit terrain tile. 
			pTile = CreateTileByRect(vPos.x, vPos.z, width, height);
		}


		if (obj->CheckVolumnField(OBJ_VOLUMN_FREESPACE))
		{
			//TODO: if the object is already in the tile, it will be deleted and re-attached.
			//pTile->m_listFreespace.remove(obj);
			pTile->m_listFreespace.push_back(obj);
		}
		else
		{
			//TODO: if the object is already in the tile, it will be deleted and re-attached.
			//pTile->m_listSolidObj.remove(obj);
			pTile->m_listSolidObj.push_back(obj);
		}

		if (!obj->GetIdentifier().empty())
		{
			// we add it to the container tile 
			const string& sName = obj->GetIdentifier();
			pTile->m_namemap[sName] = obj;

			// we will also add it to root tile name mapping, if it begins with "g_", "s_"
			if (sName.size()>2 && sName[1] == '_' && (sName[0] == 'g' || sName[0] == 's'))
			{
				m_globalMeshNameMapping[sName] = obj;
			}
		}
		obj->SetTileContainer(pTile);
		if (gameObj != NULL)
		{
			gameObj->On_Attached();
		}
		return pTile;
	}
	else
		return NULL;
}

bool CTerrainTileRoot::DetachObject(CBaseObject * obj)
{
	CTerrainTile* pTile = obj->GetTileContainer();

	bool bSuccess = false;
	if (obj->IsGlobal()) // if it is a global object. 
	{
		// if the object is the current player, we will set the current player to the next object
		// so that the camera does not lose focus.
		if (CGlobals::GetScene()->GetCurrentPlayer() == obj)
		{
			CBaseCamera* pCam = CGlobals::GetScene()->GetCurrentCamera();
			if (pCam)
			{
				CBipedObject* pNext = CGlobals::GetScene()->GetNextPlayer((CBipedObject*)obj);
				if (pNext)
				{
					CGlobals::GetScene()->SetCurrentPlayer(pNext);
					pCam->SetFollowTarget(pNext);
				}
				else
					return false; // can not detach the last global object from the scene
			}
		}

		// delete the global object on the root tile.
		{
			auto itEndCP = m_listSolidObj.end();
			for (auto itCurCP = m_listSolidObj.begin(); itCurCP != itEndCP; ++itCurCP)
			{
				if ((*itCurCP) == obj)
				{
					// erase from name mapping of that tile
					map<string, CBaseObject*>::iterator iter = m_namemap.find(obj->GetIdentifier());
					if (iter != m_namemap.end() && iter->second == obj)
						m_namemap.erase(iter);
					// erase from tile
					obj->AddToAutoReleasePool();
					m_listSolidObj.erase(itCurCP);
					bSuccess = true;
					break;
				}
			}
		}
		bSuccess = true;
	}
	else if (pTile) // it is not a global object. 
	{
		ObjectRefArray_type * pObjectlist = NULL;

		if (obj->CheckVolumnField(OBJ_VOLUMN_FREESPACE))
			pObjectlist = &(pTile->m_listFreespace);
		else
			pObjectlist = &(pTile->m_listSolidObj);

		auto itEndCP = pObjectlist->end();

		for (auto itCurCP = pObjectlist->begin(); itCurCP != itEndCP; ++itCurCP)
		{
			if ((*itCurCP) == obj)
			{
				const string& sName = obj->GetIdentifier();
				if (!sName.empty())
				{
					// erase from name mapping of that tile
					map<string, CBaseObject*>::iterator iter = pTile->m_namemap.find(sName);
					if (iter != pTile->m_namemap.end() && iter->second == obj)
						pTile->m_namemap.erase(iter);

					// we will also delete it from root tile name mapping, if it begins with "g_" or "s_"
					if (sName.size()>2 && sName[1] == '_' && (sName[0] == 'g' || sName[0] == 's'))
					{
						iter = m_globalMeshNameMapping.find(sName);
						if (iter != m_globalMeshNameMapping.end() && iter->second == obj)
							m_globalMeshNameMapping.erase(iter);
					}
				}

				// erase from tile
				obj->AddToAutoReleasePool();
				pObjectlist->erase(itCurCP);
				bSuccess = true;
				break;
			}
		}
	}
	if (bSuccess)
	{
		obj->SetTileContainer(NULL);

		IGameObject* pGameObject = obj->QueryIGameObject();
		if (pGameObject != NULL)
		{
			pGameObject->On_Detached();
		}
	}
	return bSuccess;
}

CBaseObject* CTerrainTileRoot::GetLocalObject(const string& sName, const Vector3& vPos, DWORD dwFlag)
{
	if (sName.empty())
		return NULL;
	CTerrainTile* pTile = GetTileByPoint(vPos.x, vPos.z);
	return pTile->GetObject(sName);
}

CBaseObject* CTerrainTileRoot::GetGlobalObject(const string& sName)
{
	CBaseObject* pObj = GetObject(sName);
	if (pObj == 0)
	{
		if (sName.size()>2 && sName[1] == '_' && (sName[0] == 'g' || sName[0] == 's'))
		{
			map<string, CBaseObject*>::iterator iter = m_globalMeshNameMapping.find(sName);
			if (iter != m_globalMeshNameMapping.end())
				pObj = iter->second;
		}
	}
	return pObj;
}

CBaseObject* CTerrainTileRoot::GetObjectByViewBox(const CShapeAABB& viewbox)
{
	// get the position of AABB 
	Vector3 vPos;
	viewbox.GetCenter(vPos);
	// TODO: What if it is a big object? for big object, it is attached to the best fit terrain tile. 
	//       pTile  = CreateTileByRect(vPos.x,vPos.z, width, height); 
	// Simple assume it is a point object, and get the deepest tile by vPos. 
	CTerrainTile*  pTile = GetTileByPoint(vPos.x, vPos.z);

	// it will pick the best candidate with the smallest posDiff and sizeDif. In order to be considered a candidate, 
	// their size and pos dif should be at least smaller than the initial threshold value. i.e. 0.05f;
	CBaseObject* pCandidate = NULL;
	float fPosDiffSq_candidate = 0.05f;
	float fSizeDiffSq_candidate = 0.05f;
	if (pTile != 0)
	{
		/* search from solid objects.*/
		{
			for (auto& pObj : pTile->m_listSolidObj)
			{
				Vector3 vItemPos = pObj->GetPosition();
				float fOBB_X, fOBB_Y, fOBB_Z, fFacing;
				pObj->GetBoundingBox(&fOBB_X, &fOBB_Y, &fOBB_Z, &fFacing);
				float fPosDiffSq = (vItemPos - vPos).squaredLength();
				if (fPosDiffSq<(fPosDiffSq_candidate + fSizeDiffSq_candidate))
				{
					Vector3 vExtent;
					viewbox.GetExtents(vExtent);
					float fSizeDiffSq = (vExtent - Vector3(fOBB_X / 2, fOBB_Y / 2, fOBB_Z / 2)).squaredLength();
					if ((fPosDiffSq + fSizeDiffSq)<(fPosDiffSq_candidate + fSizeDiffSq_candidate))
					{
						pCandidate = pObj;
						fPosDiffSq_candidate = fPosDiffSq;
						fSizeDiffSq_candidate = fSizeDiffSq;
					}
				}
			}
		}

		/** search for free space objects */
		{
			for (auto& pObj : pTile->m_listFreespace)
			{
				Vector3 vItemPos = pObj->GetPosition();
				float fOBB_X, fOBB_Y, fOBB_Z, fFacing;
				pObj->GetBoundingBox(&fOBB_X, &fOBB_Y, &fOBB_Z, &fFacing);
				float fPosDiffSq = (vItemPos - vPos).squaredLength();
				if (fPosDiffSq<(fPosDiffSq_candidate + fSizeDiffSq_candidate))
				{
					Vector3 vExtent;
					viewbox.GetExtents(vExtent);
					float fSizeDiffSq = (vExtent - Vector3(fOBB_X / 2, fOBB_Y / 2, fOBB_Z / 2)).squaredLength();
					if ((fPosDiffSq + fSizeDiffSq)<(fPosDiffSq_candidate + fSizeDiffSq_candidate))
					{
						pCandidate = pObj;
						fPosDiffSq_candidate = fPosDiffSq;
						fSizeDiffSq_candidate = fSizeDiffSq;
					}
				}
			}
		}
	}
	return pCandidate;
}

CBaseObject* CTerrainTileRoot::GetLocalObject(const Vector3& vPos, float fEpsilon)
{
	CTerrainTile* pTile = m_subtiles[GetSubTileIndexByPoint(vPos.x, vPos.z)];

	if (pTile)
	{
		return pTile->GetLocalObject(vPos, fEpsilon);
	}

	return NULL;
}

CBaseObject* ParaEngine::CTerrainTileRoot::GetLocalObject(const Vector3& vPos, const std::string& sName, float fEpsilon/*=0.01f*/)
{
	CTerrainTile* pTile = m_subtiles[GetSubTileIndexByPoint(vPos.x, vPos.z)];

	if (pTile)
	{
		return pTile->GetLocalObject(vPos, sName, fEpsilon);
	}

	return NULL;
}

IAttributeFields* ParaEngine::CTerrainTileRoot::GetChildAttributeObject(const std::string& sName)
{
	return CTerrainTile::GetChildAttributeObject(sName);
}

IAttributeFields* ParaEngine::CTerrainTileRoot::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	return CTerrainTile::GetChildAttributeObject(nRowIndex, nColumnIndex);
}

int ParaEngine::CTerrainTileRoot::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return CTerrainTile::GetChildAttributeObjectCount(nColumnIndex);
}

int ParaEngine::CTerrainTileRoot::GetChildAttributeColumnCount()
{
	return CTerrainTile::GetChildAttributeColumnCount();
}
