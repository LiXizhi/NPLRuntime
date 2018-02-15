#pragma once
#include "MetaObject.h"
#include "ShapeAABB.h"

namespace ParaEngine
{
	
	/// a meta height map is associated with a region in the global height map based terrain. 
	class MetaHeightmap : public MetaObject
	{
	public:
		/** a region in global height map based terrain. */
		class TerrainRegion
		{
		public:
			/** bounding box in world space */
			TerrainRegion(const CShapeAABB& aabb) : m_aabb(aabb) {};

			/** in world coordinate. */
			float getHeightAt(float x, float z);
			Vector3 getCenter() const;

			const CShapeAABB& getBoundingBox() const {return m_aabb;};
		private:
			CShapeAABB m_aabb;
		};
		
		/** 
		* @param wf: world fragment to which this meta object belongs. 
		*/
		MetaHeightmap(MetaWorldFragment *wf, const TerrainRegion& t, float groundThreshold);

		/// Adds this meta height map to the data grid.
		virtual void updateDataGrid(DataGrid* dataGrid);
		/// Returns the fallof range of the MetaHeightmap.
		float getFallofRange() const {return mFallofRange; }
		/// Sets the fallof range. A fallof range less than the dataGrids grid size 
		/// will make the algorithm fail.
		void setFallofRange(float fallof) {mFallofRange = fallof; }
		virtual CShapeAABB getAABB() const;

	protected:
		// the parent terrain tile to which this meta height map is bound. 
		TerrainRegion mTerrainTile;

		float mFallofRange, mGroundThreshold, mGradient;
	};
}