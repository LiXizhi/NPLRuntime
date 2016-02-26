#pragma once
#include "GDIGraphicsPath.h"
#include "GDIMisc.h"

namespace ParaEngine
{
	struct TextureEntity;	
	class CGDIPen;
	class CGDIDC;
	struct GUITextureElement;
	struct TextureEntity;
	class CGDIGraphics
	{
	public:
		CGDIGraphics();
		~CGDIGraphics();

		/** this will internally create the CGDIDC object from the render target texture. */
		void SetRenderTarget(TextureEntity * pEntity);
		TextureEntity* GetRenderTarget();

		/** render to the current DC */
		HRESULT Render();
		/** Invalidate the graphics, causing the entire render target to be redrawn at the next render call */
		void Invalidate(bool bInvalidate = true);
		/** whether it is invalidated*/
		bool IsInvalidated();

		/**
		* get the current pen object
		* @return 
		*/
		CGDIPen* GetCurrentPen(){ return &m_currentPen;}
		/**
		* set the current pen object
		* @param pen 
		*/
		void SetCurrentPen(const CGDIPen& pen){ m_currentPen = pen;}

		/**
		* create a new pen object in the pen pools
		* @param name 
		* @return NULL may be returned if pen is not valid
		*/
		CGDIPen* CreatePen(string name);
		/** get a new pen object in the pen pools*/
		CGDIPen* GetPen(string name);

		/** save the current texture to disk file. When we saved the current graphics to disk, 
		* we will also reset the graphics object to disk. */
		bool SaveAs(const char* filename);
		/** load the texture from a disk image.The current graphics will be reset. */
		bool LoadFromTexture(TextureEntity* pTexture);
		/** get the current disk texture. NULL may be returned. */
		TextureEntity* GetDiskTexture();


		//////////////////////////////////////////////////////////////////////////
		//
		// graphic manipulating functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** reset to be reused. Nothing except the disk texture will be drawn.*/
		void Reset();

		/**
		* Clears the current render target of the CGDIGraphics
		*/
		HRESULT Clear(const Color &color);
		/**
		* Draw a line with a specific pen, the operation is pending
		* If point1=point2, the line will not draw
		*/
		HRESULT DrawLine(const CGDIPen *pen,const Vector3 &point1,const Vector3 &point2);
		HRESULT DrawLines(const CGDIPen *pen, const Vector3 *points,int nNumPoints);
		HRESULT DrawImage(GUITextureElement *pElement,RECT *prcDest,float depth );
		/**
		* Draw a point with a specific pen, the operation is pending
		*/
		HRESULT DrawPoint(const CGDIPen *pen,const Vector3 &point);
		
		/** undo a certain amount of steps*/
		void Undo(int nStep=1);
		/** redo a certain amount of steps*/
		void Redo(int nStep=1);

		/**
		* Set the current clipping rectangle for the path
		*/
		void SetClip(const CGDIRegion &region);
		const CGDIRegion* GetClip()const{return &m_region;}

		// lock the current GDC
		CGDIDC * LockDC(){m_bLocked=true;return m_pDC;}
		void UnlockDC(){m_bLocked=false;}

		/**
		* This callback function will be called immediately after the Direct3D device has been 
		* reset, which will happen after a lost device scenario. This is the best location to 
		* create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
		* the device is lost. Resources created here should be released in the OnLostDevice 
		* callback. 
		*/
		HRESULT RestoreDeviceObjects();
		/**
		* This callback function will be called immediately after the Direct3D device has 
		* entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
		* in the OnResetDevice callback should be released here, which generally includes all 
		* D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
		* information about lost devices.
		*/
		HRESULT InvalidateDeviceObjects();
		
	protected:
		HRESULT CalculatePath(CGDIGraphicsPath &pPath);
		/**
		* Calculate the two vertices for one end of a given line
		* @param point1, point2: [in] the first and second point of a line. 
		*	If the two points are the same, it will be treated as a point. In this case, nState=0 will return zero vertices;
		* @param vex1,vex2: [out] the return vertices;
		* @param pPen: [in] pointer to the pen used to draw this line.
		* @param nState: Specify which cap we like
		* nState=0: no cap, the return vertices are based on the first point, this is for the middle points of a polyline.
		* nState=-1 start cap, the return vertices are the start cap based on the first point;
		* nState=1 end cap, the return vertices are the end cap based on the second point; 
		*/
		CGDIDC* m_pDC;
		bool m_bLocked;
		/** if this is true, the graphics will be redrawn at the next render frame*/
		bool m_bInvalidated;
		CGDIGraphicsPath m_path;
		CGDIRegion m_region;
		/** the texture entity that is saved and persistent to disk file.*/
		asset_ptr<TextureEntity> m_pDiskTexture;

		/** the current pen object */
		CGDIPen m_currentPen;
		/** available pens*/
		map <string, CGDIPen> m_pens;
	private:
	};
}