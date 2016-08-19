#pragma once
#include "IAttributeFields.h"
#include "ViewportManager.h"
#include "Viewport.h"

namespace ParaEngine
{
	enum VIEWPORT_LAYOUT {
		VIEW_LAYOUT_DEFAULT,
		VIEW_LAYOUT_STEREO_LEFT_RIGHT,
		VIEW_LAYOUT_STEREO_UP_DOWN,
		VIEW_LAYOUT_STEREO_RED_BLUE,
		VIEW_LAYOUT_INVALID,
	};

	/** manager multiple viewport
	*/
	class CViewportManager : public IAttributeFields
	{
	public:
		CViewportManager();
		virtual ~CViewportManager(void);

		ATTRIBUTE_DEFINE_CLASS(CViewportManager);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);

		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

	public:
		/** call this function when the window size and screen back buffer size is changed */
		void UpdateViewport(int nBackbufferWidth, int nBackbufferHeight);

		/** render all view ports */
		HRESULT Render(double dTimeDelta, int nPipelineOrder);

		/** @param x, y: [in|out] a position on back buffer. If it is screen position, it should be multiplied by UI scaling. 
		* @param pWidth, pHeight: the view port's size is returned. 
		* @return true: if there is a point inside one of the viewports. otherwise false.
		*/
		void GetPointOnViewport(int& x, int& y, int* pWidth, int* pHeight);

		CViewport* GetViewportByPoint(int x, int y);

		/** set default view layout */
		void SetLayout(VIEWPORT_LAYOUT nLayout, CSceneObject* pMainScene = NULL, CGUIRoot* pGUIRoot = NULL);
		VIEWPORT_LAYOUT GetLayout();

		/** just in case the size changed. */
		void UpdateLayout();

		int GetViewportCount();
		void SetViewportCount(int nCount);

		/** get current viewport*/
		void GetCurrentViewport(ParaViewport& out);
	public:
		/** add view port 
		* @param nIndex: usually 0 is the GUI root's viewport, 1 is the main 3d scene's viewport.
		*/
		CViewport* CreateGetViewPort(int nIndex = 1);
		
		CViewport* GetActiveViewPort();
		void SetActiveViewPortIndex(int val);
		void SetActiveViewPort(CViewport* pViewport);

		void DeleteViewPort(int nIndex=0);

		void Cleanup();

		int GetWidth() const;
		int GetHeight() const;

		void ApplyViewport();

	private:
		void SortViewport();
	private:
		/** all view ports */
		std::vector<CViewport*> m_viewportList;
		std::vector<CViewport*> m_viewportSorted;

		ParaViewport m_viewport;
		int m_nWidth;
		int m_nHeight;
		int m_nActiveViewPortIndex;
		VIEWPORT_LAYOUT m_nLayout;
	};

}

