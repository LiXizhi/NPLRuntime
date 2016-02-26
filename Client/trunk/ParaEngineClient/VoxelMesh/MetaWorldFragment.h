#pragma once
#include <vector>
#include "MetaObject.h"
#include "DataGrid.h"
#include "ShapeAABB.h"

namespace ParaEngine
{
	class IsoSurfaceRenderable;
	class IsoSurfaceBuilder;
	class MetaObject;

	/**
	* A MetaWorldFragment is the basic building block of the world. it has its own IsoSurface which is used for rendering.
	* When created, an IsoSurface should be created..The MetaWorldFragment will also contain portals to other MetaWorldFragments for visibility culling (todo).
	*/
	class MetaWorldFragment : public CRefCountedOne
	{
	public:
		typedef std::vector<MetaWorldFragment*> WfList;
		typedef std::vector<MetaWorldFragment*>::iterator WfIter;
		typedef std::vector<MetaWorldFragment*>::const_iterator WfConstIter;


		///Creates new MetaWorldFragment, it will only create IsoSuface and grid on demand.
		MetaWorldFragment(const Vector3 &position, int ylevel = 0);
		virtual ~MetaWorldFragment();

		///Adds MetaObject to mObjs, and to mMoDataGrid
		void addMetaObject(MetaObjectPtr mo);
		///Updates IsoSurface
		void update(IsoSurfaceBuilder *builder);
		int getNumMetaObjects() {return (int)mObjs.size();} const
		CShapeAABB getAABB() {return mAabb;} const
		Vector3 getPosition() {return mPosition;} const
		bool empty() {return mObjs.empty();}
		static float getScale() {return mGridScale;}
		static float getSize() {return mSize;}
		static void setScale(float s) {mGridScale = s;}
		static void setSize(float s) {mSize = s;}

		IsoSurfaceRenderable * getIsoSurface() {return mSurf;}
		int getYLevel() { return mYLevel; }

		static void setMaterialName(const std::string &name) {mMaterialName = name;}
		static const std::string &getMaterialName() {return mMaterialName;}

		/**
		* it will only update when needed by draw function call. 
		*/
		bool IsNeedUpdate(){ return m_bNeedUpdate; }

		void SetNeedUpdate(bool bNeedUpdate = true){ m_bNeedUpdate = bNeedUpdate; }
	protected:
		void addToWfList(MetaWorldFragment* wf);

	protected:
		IsoSurfaceRenderable *mSurf;

		Vector3 mPosition;
		CShapeAABB mAabb;
		static float mGridScale;
		static float mSize;
		// Child meta objects in this world fragment. Vector of potentially overlapping MetaObjects children. 
		std::vector<MetaObjectPtr> mObjs;
		/** TODO: currently no merging is supported for adjacent fragments */
		std::vector<MetaWorldFragment*> mAdjacentFragments;

		/**	position in y, counted in tile-sizes.
		* this value is only useful when we are trying to manage a pool of metaworldfragments nearby according to their Y level. 
		* so that we can group by height.  This is usefully when doing voxel based terrain. 
		*/ 
		int mYLevel;

		/** whether the grid is modified and needs update in the next draw call. */
		bool m_bNeedUpdate;

		// TODO: use TextureEntity share ptr here. 
		static std::string mMaterialName;
	};

	typedef asset_ptr<MetaWorldFragment>  MetaWorldFragmentPtr;
}
