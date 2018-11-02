#pragma once
#include "util/ParaMemPool.h"
#include "unordered_ref_array.h"
#include "IGameObject.h"
#include "AttributeModelProxy.h"

#include <list>
#include <map>

namespace ParaEngine
{
class CBaseObject;
class CBipedObject;
class IGameObject;
typedef unordered_ref_array<CBaseObject*> ObjectRefArray_type;

typedef FixedSizedAllocator<IGameObject::WeakPtr_type>	DL_Allocator_IGameWeakPtr;
typedef std::list<IGameObject::WeakPtr_type, DL_Allocator_IGameWeakPtr>	VisitorList_type;

/** a tile in the latticed terrain class*/
class CTerrainTile : public IAttributeFields
{
public:
	CTerrainTile();
	CTerrainTile(float x, float y, float r);
	virtual ~CTerrainTile(void);
	ATTRIBUTE_DEFINE_CLASS(CTerrainTile);
	/** get attribute by child object. used to iterate across the attribute field hierarchy. */
	virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
	/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
	virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
	/** we support multi-dimensional child object. by default objects have only one column. */
	virtual int GetChildAttributeColumnCount();
	virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

public:
	virtual void Cleanup();
	/// -- data structuring
	#define 	MAX_NUM_SUBTILE		4
	CTerrainTile*			m_subtiles[MAX_NUM_SUBTILE];

	/// solid objects list. Mobile objects might collide with this kind of objects
	ObjectRefArray_type		m_listSolidObj;
	/// Free space objects list, might contains a rendering hierarchy. 
	/// they has nothing to do with collision detection
	ObjectRefArray_type		m_listFreespace;
	/// mobile game objects that is moving in this region
	VisitorList_type		m_listVisitors;
	/// global name mapping.
	map<string, CBaseObject*> m_namemap;
	unordered_ref_array<IAttributeFields*> m_children;

	/// the central position of the terrain.
	float	m_fX, m_fY;
	/// the radius of the entire terrain (half the length of the square terrain).
	float	m_fRadius;
public:
	/// get object position in the world space
	void GetPosition(Vector3 *pV);
	bool TestCollisionSphere(const Vector3* pvCenter, FLOAT radius);
	bool TestCollisionObject(CBaseObject* pObj);
	float GetSphereCollisionDepth(Vector3* pvCenter, FLOAT radius, bool bSolveDepth = false);
	float GetObjectCollisionDepth(CBaseObject* pObj);

	int	GetSubTileIndexByPoint(FLOAT fAbsoluteX, FLOAT fAbsoluteY);
	/**
	* return the indice of child tiles which contain or intersect with a rectangular.
	* @param fAbsoluteX: Center point.x of the rect.
	* @param fAbsoluteY:Center point.y of the rect.
	* @param fWidth: the width of the rect.
	* @param fHeight:the height of the rect.
	* @return: a bit mask telling which child tile intersects with the rect.
	*  if (return value & (1<<nIndex))>0, then child tile of nIndex is hit by the rect
	*/
	int	GetChildTileIndexByRect(FLOAT fAbsoluteX, FLOAT fAbsoluteY, FLOAT fWidth, FLOAT fHeight);
	CTerrainTile*	CreateSubTile(int nIndex);
	/**
	* find a object by its name. 
	* @param pSearchString: the name of the object to be searched. The name may end with "*"
	*	in which case "*" will match any character(s). 
	*	e.g "Li*" will match both "LiXizhi" and "LiTC".In case there is multiple objects, the first found object is returned.
	*		"Li" will only match "Li"
	* @param search_mode: The default value is 0, which means that only global characters are searched.
	* @param reserved: currently it must be 0
	*/
	CBaseObject* SearchObject(const char* pSearchString, int search_mode=0, int reserved = 0);
	/** get the object by its name. If there have been several objects with the same name,
	* the most recently attached object is returned.
	* @param sName: exact name of the object
	*/
	CBaseObject* GetObject(const string& sName);

	/** destroy an object by its name. If there are multiple object with the same name, they will all be deleted.
	* @param sName: exact name of the object
	*/
	void DestroyObjectByName(const char* sName);

	/**
	* get the first local object,whose position is very close to vPos. This function will search for the first (local mesh) object throughout the hierachy of the scene.
	* this function is kind of slow, please do not call on a per frame basis. Use GetObjectByViewBox() to get an object faster. 
	* @param vPos: world position of the local mesh object
	* @param fEpsilon: if a mesh is close enough to vPos within this value. 
	* @return: NULL if not found
	*/
	CBaseObject* GetLocalObject(const Vector3& vPos, float fEpsilon=0.01f);
	CBaseObject* GetLocalObject(const Vector3& vPos, const std::string& sName, float fEpsilon=0.01f);

	/** add a new visitor to the terrain tile.
	* @param bCheckDuplicate: if true, no duplicate is allowed. */
	void AddVisitor(IGameObject* pObj, bool bCheckDuplicate= false);
	/** delete a visitor to the terrain tile. Return true if found and removed. */
	bool RemoveVisitor(IGameObject* pObj);
};

}