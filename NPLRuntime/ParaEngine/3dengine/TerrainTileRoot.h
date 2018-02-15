#pragma once
#include "TerrainTile.h"
#include "BaseObject.h"

namespace ParaEngine
{
	/** the root of the quad-tree terrain tiles. When Tile Object is attached to the scene,
	* they are automatically attached to this quad-tree. Once attached, a strong reference of the object is kept,
	* once detached, object will be added to the auto release pool, which will be deleted some time in the next frame if reference count is zero.
	*/
	class CTerrainTileRoot : public CTerrainTile
	{
	public:
		CTerrainTileRoot(void);
		virtual ~CTerrainTileRoot(void){};

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CTerrainTileRoot; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CTerrainTileRoot"; return name; }

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		virtual void Cleanup();
	public:
		
		/// Reset Terrain
		void ResetTerrain(float fRadius, int nDepth);

		/// These values are used by Terrain TILE.
		enum DIRECTION
		{
			/// Offset along negative x and y axes
			DIR_SOUTHWEST = 0,
			/// Offset along negative y axis
			DIR_SOUTH = 1,
			/// Offset along positive x and negative y axes
			DIR_SOUTHEAST = 2,
			/// Offset along negative x axis
			DIR_WEST = 3,
			/// No offset
			DIR_CENTER = 4,
			/// Offset along positive x axis
			DIR_EAST = 5,
			/// Offset along negative x and positive y axes
			DIR_NORTHWEST = 6,
			/// Offset along the positive y axis
			DIR_NORTH = 7,
			/// Offset along positive x and y axes
			DIR_NORTHEAST = 8,
			/// Placeholder for uninitialized and maximum values
			DIR_INVALID = 9
		};
		/** get any of the 9 adjacent tile of pTile.
		* @remark: Please note that because tiles are a dynamic quad tree, the same tile may be returned for two different directions.
		* @param nDirection: number 0-8, @see DIRECTION.
		* @return: pointer to the adjacent tiles. If there is no tile at that position, its parent tile is returned.
		*	hence, the same tile may be returned when querying different directions. */
		CTerrainTile* GetAdjacentTile(CTerrainTile* pTile, DIRECTION nDirection);

		/**
		* get any of the 9 adjacent tiles of a given point.
		* @remark: Please note that because tiles are a dynamic quad tree, the same tile may be returned for two different directions.
		* @param vPos the position centered at which the 9 adjacent tiles are queried.Please note that only x,z component is used. y is ignored.
		* @param nDirection number 0-8, @see DIRECTION.
		* @param fRadius radius of the adjacent region. If this is 0 or negative, the smallest quad tree size will be used.
		* @return pointer to the adjacent tiles. If there is no tile at that position, its parent tile is returned.
		*	hence, the same tile may be returned when querying different directions. */
		CTerrainTile* GetAdjacentTile(const Vector3& vPos, DIRECTION nDirection, float fRadius = 0.f);

		/**
		* attach object to the proper tile that tells most exactly where it is
		* if the object is already in the tile, it will be attached twice.So it is the caller's responsibility to check
		* for duplicate calls to this function for the same object.
		* @return: the terrain tile which the object is attached is returned
		*/
		CTerrainTile* AttachLocalObject(CBaseObject* obj);

		/**
		* the object will be attached to the root tile as solid object. global objects can be retrieved very fast by its name.
		* @return: the terrain tile which the object is attached is returned
		*/
		CTerrainTile* AttachGlobalObject(CBaseObject* obj);

		/**
		* detach the object from the tile or its sub tiles.
		* it will not be found. the function will return once the first matching object is found and detached
		* @param pObject: the object to be detached.
		* @return: true if the object has been found and detached.
		*/
		bool DetachObject(CBaseObject * pObject);

		/**
		* get existing smallest tile that contains the given point
		* @param fAbsoluteX
		* @param fAbsoluteY
		* @return
		*/
		CTerrainTile* GetTileByPoint(float fAbsoluteX, float fAbsoluteY);

		/**
		* get existing smallest tile that contains the given rect
		* @param fAbsoluteX
		* @param fAbsoluteY
		* @param fPtWidth
		* @param fPtHeight
		* @return
		*/
		CTerrainTile* GetTileByRect(float fAbsoluteX, float fAbsoluteY, float fPtWidth, float fPtHeight);

		/** get the global object by its name. If there have been several objects with the same name,
		* the most recently attached object is returned.
		* @remark: if local mesh's name begins with "g_", it can also be retrieved by calling this function.
		* however, if a global object has the same name, the global object is always returned instead of the local mesh.
		* @param sName: exact name of the object
		*/
		CBaseObject* GetGlobalObject(const string& sName);

		/**
		* Get an object. Call this function to see if object with a given identifier is already attached.
		* @param sName name of the object, if this is "", the function always returns NULL
		* @param vPos: the location to search for the object.
		* @param dwFlag : [reserved] a value specifying the search range:
		- 1 for global object only,
		- 2 for local object only,
		- 3 for both global and local object.
		* @return return the object if found, otherwise NULL.
		*/
		CBaseObject* GetLocalObject(const string& sName, const Vector3& vPos, DWORD dwFlag = 3);

		/**
		* get the first local object,whose position is very close to vPos. This function will search for the first (local mesh) object throughout the hierachy of the scene.
		* this function is kind of slow, please do not call on a per frame basis. Use GetObjectByViewBox() to get an object faster.
		* @param vPos: world position of the local mesh object
		* @param fEpsilon: if a mesh is close enough to vPos within this value.
		* @return: NULL if not found
		*/
		CBaseObject* GetLocalObject(const Vector3& vPos, float fEpsilon = 0.01f);
		CBaseObject* GetLocalObject(const Vector3& vPos, const std::string& sName, float fEpsilon = 0.01f);

		/**
		* get an object(usually a static mesh object) by a given view box.
		* @param viewbox: One can get the view box by getting the view culling object. see example in the ParaObject:GetViewBox().
		* @return: return the object with the closest match with the bounding box. NULL is returned if there is no match.
		*/
		CBaseObject* GetObjectByViewBox(const CShapeAABB& viewbox);

	private:
		/// Get and create tile
		CTerrainTile* CreateTileByRect(float fAbsoluteX, float fAbsoluteY, float fPtWidth, float fPtHeight);
		/// faster function 
		CTerrainTile* CreateTileByPoint(float fAbsoluteX, float fAbsoluteY);

		/** local meshes which have global names: global mesh name mapping.
		* all mesh name should begin with "g_" */
		map<string, CBaseObject*> m_globalMeshNameMapping;
		/// the tile depth.
		int		m_nDepth;
		float	m_fSmallestTileRadius;
	};
}