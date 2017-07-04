#pragma once
#include "math/ParaRect.h"
#include "AssetEntity.h"
#include "IObjectScriptingInterface.h"

namespace ParaEngine
{
	class CRenderTarget;

	/** picking from frame buffer (back buffer)
	* When there is picking query, it will render scene again (if out dated) with a special shader and read pixels from the back buffer. 
	* We can query a single point or we can query a rectangle region in the current viewport and see if have hit anything. 
	* Please note: in order for buffer picking to work, each pickable object/component should assign a different picking id in its draw method. 
	* In other words, picking and drawing are done using the same draw function. 
	*/
	struct CBufferPicking : public AssetEntity
	{
	public:
		explicit CBufferPicking(const AssetKey& key);
		virtual ~CBufferPicking();

		ATTRIBUTE_DEFINE_CLASS(CBufferPicking);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CBufferPicking, GetPickingCount_s, int*)	{ *p1 = cls->GetPickingCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CBufferPicking, GetPickingID_s, DWORD*)	{ *p1 = cls->GetPickingID(-1); return S_OK; }
		ATTRIBUTE_METHOD(CBufferPicking, ClearPickingResult_s)	{ cls->ClearPickingResult(); return S_OK; }
		ATTRIBUTE_METHOD(CBufferPicking, CheckDoPick_s)	{ cls->CheckDoPick(); return S_OK; }
		
		ATTRIBUTE_METHOD1(CBufferPicking, GetPickLeftTop_s, Vector2*)	{ *p1 = cls->GetPickLeftTop(); return S_OK; }
		ATTRIBUTE_METHOD1(CBufferPicking, SetPickLeftTop_s, Vector2)	{ cls->SetPickLeftTop(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBufferPicking, GetPickWidthHeight_s, Vector2*)	{ *p1 = cls->GetPickWidthHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CBufferPicking, SetPickWidthHeight_s, Vector2)	{ cls->SetPickWidthHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBufferPicking, IsResultDirty_s, bool*)	{ *p1 = cls->IsResultDirty(); return S_OK; }
		ATTRIBUTE_METHOD1(CBufferPicking, SetResultDirty_s, bool)	{ cls->SetResultDirty(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBufferPicking, GetPickIndex_s, int*)	{ *p1 = cls->GetPickIndex(); return S_OK; }
		ATTRIBUTE_METHOD1(CBufferPicking, SetPickIndex_s, int)	{ cls->SetPickIndex(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBufferPicking, GetViewport_s, int*)	{ *p1 = cls->GetViewport(); return S_OK; }
		ATTRIBUTE_METHOD1(CBufferPicking, SetViewport_s, int)	{ cls->SetViewport(p1); return S_OK; }

	public:
		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		/** pick by a point in the viewport. 
		* Tip: to pick a thin line, one may consider picking by a small rect region. 
		* @param nViewportId: viewport index, if -1 it means the current viewport.
		* @return the picking data directly. if 0 means nothing is picked.
		*/
		DWORD Pick(int nX, int nY, int nViewportId = -1);
		
		/** pick in a rectangular region in the viewport
		* to retrieve the actually result, use GetPickingResult(), which returns an array of unique picking id.  
		* @param nViewportId: viewport index, if -1 it means the current viewport. 
		* @return the number of pixels in region that has hit something pickable. 
		*/
		int Pick(const QRect& region, int nViewportId = -1);

		/** return an array of unique picking id in the last pick call. */
		const vector<DWORD>& GetPickingResult() { return m_pickingResult; };

		/** return the number of objects picked. */
		int GetPickingCount();

		/** get the picked item id of the given picking item. if no data at the index return 0. 
		* @param nIndex: if -1, it will use m_currentPickIndex;
		*/
		DWORD GetPickingID(int nIndex = 0);

		/** clear last picking result */
		void ClearPickingResult();

		void SetPickLeftTop(const Vector2& vPos);
		const Vector2& GetPickLeftTop();
		void SetPickWidthHeight(const Vector2& vPos);
		const Vector2& GetPickWidthHeight();

		int GetPickIndex() const;
		void SetPickIndex(int val);

		bool IsResultDirty() const;
		void SetResultDirty(bool bDirty=true);

		/* in which viewport to pick. default to -1, which is the default one. */
		int GetViewport() const;
		/* in which viewport to pick. default to -1, which is the default one. */
		void SetViewport(int val);

		/** do picking using current picking rectangle. One do not need to call this manually, this function
		* is called automatically when querying the picking result with GetPickingID and GetPickingCount.
		* @param bForceUpdate : if true, we will force update. if false, we will only do picking when result is dirty. 
		*/
		void CheckDoPick(bool bForceUpdate = false);
	protected:
		bool BeginBuffer();

		void DrawObjects();

		void EndBuffer();
		/** a render target will be created with the same name as this object. */
		virtual CRenderTarget* CreateGetRenderTarget(bool bCreateIfNotExist = true);

	private:
		vector <DWORD> m_pickingResult;
		QRect m_pickRect;
		bool m_bResultDirty;
		
		int m_currentPickIndex;
	
		Vector2 m_cache_left_top;
		Vector2 m_cache_width_height;
		int m_nViewport;
		
		/** only used when buffer name is not "backbuffer" */
		WeakPtr m_renderTarget;
	};

	/** all picking buffers
	* There is a default one called `BufferPickingManager::GetInstance().GetEntity("backbuffer");` which is the current backbuffer 
	* Some predefined picking object can be retrieved via NPL script using
	*
	* ParaEngine.GetAttributeObject():GetChild("BufferPicking")
	* ParaEngine.GetAttributeObject():GetChild("OverlayPicking")
	*/
	class BufferPickingManager : public AssetManager <CBufferPicking>
	{
	public:
		BufferPickingManager(){};
		static BufferPickingManager& GetInstance();
	public:
		/** set all picking result dirty */
		void SetResultDirty();
	};
}


