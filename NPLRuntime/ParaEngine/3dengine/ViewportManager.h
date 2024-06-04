#pragma once
#include "IAttributeFields.h"
#include "ViewportManager.h"
#include "Viewport.h"

#include "webxr.h"

namespace ParaEngine
{
    enum VIEWPORT_LAYOUT {
        VIEW_LAYOUT_DEFAULT,
        VIEW_LAYOUT_STEREO_LEFT_RIGHT,
        VIEW_LAYOUT_STEREO_UP_DOWN,
        VIEW_LAYOUT_STEREO_RED_BLUE,
        VIEW_LAYOUT_STEREO_OMNI = 4,//ODS,360°VR
        VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE=5,//ODS,360°VR
        //前后左右上下6个viewport组成一个立方体，然后通过shader转成球面，以实现180x360全景渲染
        VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE_1 = 6,

        VIEW_LAYOUT_INVALID,
        VIEW_LAYOUT_STEREO_WEBXR,
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
        ATTRIBUTE_METHOD1(CViewportManager, GetViewportCount_s, int*) { *p1 = cls->GetViewportCount(); return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, GetLayout_s, int*) { *p1 = cls->GetLayout(); return S_OK; }

        ATTRIBUTE_METHOD1(CViewportManager, GetODSFov_s, float*) { *p1 = cls->ods_fov; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, SetODSFov_s, float) { cls->ods_fov = (p1); return S_OK; }

        ATTRIBUTE_METHOD1(CViewportManager, GetWidthPerDegree_s, int*) { *p1 = cls->widthPerDegree; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, SetWidthPerDegree_s, int) { cls->widthPerDegree = (p1); return S_OK; }

        ATTRIBUTE_METHOD1(CViewportManager, GetOmniAlwaysUseUpFrontCamera_s, bool*) { *p1 = cls->m_bOmniAlwaysUseUpFrontCamera; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, SetOmniAlwaysUseUpFrontCamera_s, bool) { cls->m_bOmniAlwaysUseUpFrontCamera = (p1); return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, GetOmniForceLookatDistance_s, int*) { *p1 = cls->m_nOmniForceLookatDistance; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, SetOmniForceLookatDistance_s, int) { cls->m_nOmniForceLookatDistance = (p1); return S_OK; }

        ATTRIBUTE_METHOD1(CViewportManager, DeleteViewportByName_s, const char*) { cls->DeleteViewPort(p1); return S_OK; }

#ifdef EMSCRIPTEN
        ATTRIBUTE_METHOD1(CViewportManager, GetIsXR_s, bool*) { *p1 = cls->m_isXR; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, SetIsXR_s, bool) { cls->SetIsXR((p1)); return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, GetWebXRTime_s, int*) { *p1 = cls->m_webXRTime; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, GetWebXRHeadPose_s, const char**) { *p1 = ""; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, GetWebXRViews_s, const char**) { *p1 = ""; return S_OK; }
        ATTRIBUTE_METHOD1(CViewportManager, GetWebXRViewsCount_s, int*) { *p1 = cls->m_webXRViewCount; return S_OK; }
#endif
        /** get attribute by child object. used to iterate across the attribute field hierarchy. */
        virtual IAttributeFields* GetChildAttributeObject(const char * sName);

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

        void SetOmniAlwaysUseUpFrontCamera(bool val) { m_bOmniAlwaysUseUpFrontCamera = val; }
        bool IsOmniAlwaysUseUpFrontCamera() { return m_bOmniAlwaysUseUpFrontCamera; }
        void SetOmniForceLookatDistance(int val) { m_nOmniForceLookatDistance = val; }
        int GetOmniForceLookatDistance() { return m_nOmniForceLookatDistance; }

        int CheckInViewPortGroup(CViewport* pViewport);
    public:
        /** add view port 
        * @param nIndex: usually 0 is the GUI root's viewport, 1 is the main 3d scene's viewport.
        */
        CViewport* CreateGetViewPort(int nIndex = 1);
        
        CViewport* GetActiveViewPort();
        void SetActiveViewPortIndex(int val);
        void SetActiveViewPort(CViewport* pViewport);

        void DeleteViewPort(int nIndex=0);
        void DeleteViewPort(const std::string& name);

#ifdef EMSCRIPTEN
        void SetIsXR(bool isXR);
        bool GetIsXR();
        void SaveWebXRView(int time, ParaWebXRRigidTransform *headPose, ParaWebXRView views[2], int viewCount);
#endif
        void Cleanup();

        int GetWidth() const;
        int GetHeight() const;

        void ApplyViewport();

        int getCurrentFrameNumber()const
        {
            return m_nCurrentFrameNumber;
        }
    private:
        void SortViewport();
    private:
        /** all view ports */
        std::vector<CViewport*> m_viewportList;
        std::vector<CViewport*> m_viewportSorted;

        CViewport* m_normalScenePortInOdsSingleEye;

        ParaViewport m_viewport;
        int m_nWidth;
        int m_nHeight;
        int m_nActiveViewPortIndex;
        VIEWPORT_LAYOUT m_nLayout;
        int m_nCurrentFrameNumber;
        float ods_fov;
        int widthPerDegree;
        bool m_bOmniAlwaysUseUpFrontCamera;//Whether the camera is forced to face straight ahead
        int m_nOmniForceLookatDistance;
#ifdef EMSCRIPTEN
        bool m_isXR;
        int m_webXRTime;
        ParaWebXRRigidTransform *m_webXRHeadPose;
        ParaWebXRView m_webXRViews[2];
        int m_webXRViewCount;
#endif
    };

}

