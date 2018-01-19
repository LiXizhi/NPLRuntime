#pragma once
#include "ShapeAABB.h"
#include "MetaObject.h"

namespace ParaEngine
{
	/**
	* a meta ball is a building block of meta world fragment.
	* it is a ball object with a given center and a radius. It can either be excavating or not. 
	* excavating is means a solid ball, non-excavating means hollow ball(empty space)
	*/
	class MetaBall : public MetaObject
	{
	public:
		MetaBall(MetaWorldFragment *wf, const Vector3& position, float radius = 3.0f, bool excavating = true);

		/// Adds this meta ball to the data grid.
		virtual void updateDataGrid(DataGrid* dataGrid);
		/// Returns the radius of the meta ball.
		float getRadius() const {return mRadius; }
		/// Sets the radius of the meta ball.
		void setRadius(float radius) {mRadius = radius; }
		void setExcavating(bool e) {mExcavating = true;}
		virtual CShapeAABB getAABB() const;

	protected:
		// radius of the ball
		float mRadius;

		// excavating is means a solid ball, non-excavating means hollow ball(empty space)
		bool mExcavating;
	};
	typedef asset_ptr<MetaBall> MetaBallPtr;
}
