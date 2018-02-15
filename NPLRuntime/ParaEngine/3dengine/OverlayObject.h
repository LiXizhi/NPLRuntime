#pragma once
#include "TileObject.h"

namespace ParaEngine
{
	class CGUIButton;

	/** Overlay provides custom 3D drawings to the scene after main 3d scene is rendered.
	Overlay is usually used to render helper objects such as translation/rotation/scaling manipulators.
	Internally it uses 3D object's headon display interface and GUI painter API.
	In addition to lines and triangles, one can render any kind of GUI objects and even provides interactions in the 3d scene.
	*/
	class COverlayObject : public CTileObject
	{
	public:
		COverlayObject();
		virtual ~COverlayObject();

		ATTRIBUTE_DEFINE_CLASS(COverlayObject);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(COverlayObject);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

	public:
		/** if true, we will attach this object to quad-tree terrain tile according to its attributes when adding to the scene
		* if false, we will automatically attach it to a container object named after its class name.
		*/
		virtual bool IsTileObject();
		virtual void SetTileObject(bool bIsTileObject);

		/** derived class can override this function to place the object in to the render pipeline.
		* if this function return -1 by default, the SceneObject will automatically place the object into the render pipeline.
		* if return 0, it means the object has already placed the object and the scene object should skip this object.
		*/
		virtual int PrepareRender(CBaseCamera* pCamera, SceneState * sceneState);

		virtual CGUIBase* GetHeadOnUIObject(int nIndex = 0);
	protected:
		ref_ptr<CGUIButton> m_pGUIObject;
		bool m_bIsTileObject;
	};
}

