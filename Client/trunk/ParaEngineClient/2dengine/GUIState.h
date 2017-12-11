#pragma once

namespace ParaEngine
{
	class CGUIBase;
	class CPaintDevice;
	class CPainter;

	/**
	* It's used as parameter to Render method of each GUI object. Its only
	* instance is in the GUIRoot object.
	* It keeps track of all the intermediate results  generated during game rendering.
	* It might be used by game interface, environment simulator or AI, since most of
	* the time we are only interested of computing object that is in the view.
	*/
	struct GUIState
	{
	public:
		GUIState();

	public:
		void AddToPostRenderingList(IObject* obj);
		void CleanupGUIState();

		/** draw calls between BeginDepthTest() and EndDepthTest() are rendered with depth enabled,
		* such as for relative to 3d objects. Nested called are allowed and detected. 
		*/
		void BeginDepthTest(CGUIBase* pObject = NULL);
		void EndDepthTest(CGUIBase* pObject = NULL);
	public:
		/// rendering device pointer
		RenderDevicePtr pd3dDevice;
#ifdef USE_DIRECTX_RENDERER
		/** back buffer */
		LPDIRECT3DSURFACE9 pBackSurface;
		/// back buffer format
		D3DFORMAT		d3dBackbufferFormat;
#endif
		/// back buffer width
		UINT			nBkbufWidth;
		/// back buffer height
		UINT			nBkbufHeight;

		/// true if the it's currently performing batch rendering
		bool			bIsBatchRender;
		double			dTimeDelta;
		/** current/root painter object. */
		CPainter*		painter;

		int m_nDepthEnableCount;
	public:
		/**
		listPostRenderingObjects: this is the general post rendering list. Objects in this list are
		sorted only by primary asset entity name. Texture entity is usually considered as the primary
		asset entity, since texture switch is most slow on GPU. The process goes like this: scan the
		list. For each items with the same texture as the first element in the list, render and remove
		it from the list. This goes on until the list is blank. When inserting items into the list,
		you can also sort while inserting.
		Note: post rendering are performed in the order listed above.
		*/
		std::list<WeakPtr>		listPostRenderingObjects;
	};

}