#pragma once
#include "ShapeAABB.h"

namespace ParaEngine
{
	class MetaWorldFragment;
	struct DataGrid;

	/**
	* Abstract class defining the interface for meta objects to be used with MetaObjectDataGrid.
	* it is a reference counted object
	*/
	class MetaObject : public CRefCountedOne
	{
	public:
		MetaObject(MetaWorldFragment *wf, const Vector3& position)
			: mPosition(position), mMetaWorldFragment(wf) {}
		MetaObject(MetaWorldFragment *wf)
			: mPosition(0,0,0), mMetaWorldFragment(wf) {}
		
		virtual ~MetaObject() {}

		/** Tells the meta object to update the data grid.*/
		virtual void updateDataGrid(DataGrid* dataGrid) = 0;
		/// Returns the position of the meta object.
		const Vector3& getPosition() const {return mPosition; }
		/// Sets the position of the meta object.
		void setPosition(const Vector3& position) {mPosition = position; }
		/// Returns the MetaWorldFragment* of the meta object.
		MetaWorldFragment* getMetaWorldFragment() const {return mMetaWorldFragment; }
		/// Checks for overlap with an AABB
		//	virtual bool intersects(const CShapeAABB& aabb) const = 0;
		virtual CShapeAABB getAABB() const  = 0;

	protected:
		Vector3 mPosition;
		// the parent mMetaWorldFragment who owns this meta object
		MetaWorldFragment* mMetaWorldFragment;
	};

	typedef asset_ptr<MetaObject> MetaObjectPtr;
}

