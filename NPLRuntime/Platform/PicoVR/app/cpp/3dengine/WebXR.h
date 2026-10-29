//-----------------------------------------------------------------------------
// Class: WebXR.h
// Authors: big
// CreateDate: 2026.10.28
// Desc: IParaWebXR for PICO native OpenXR. Fills the same attribute fields as
// the WebXR build, so the script side works unchanged. Also exposes per eye fov.
//-----------------------------------------------------------------------------

#pragma once
#include "ParaEngine.h"
#include "IParaWebXR.h"

namespace ParaEngine
{
    struct PicoXRFrameData;

    class CParaWebXR : public IParaWebXR
    {
    public:
        CParaWebXR();

        virtual void SetIsXR(bool isXR) override;
        virtual bool GetIsXR() override;
        virtual bool IsPressLeftHandKey(int keyCode) override;
        virtual bool IsPressRightHandKey(int keyCode) override;
        virtual void UpdateWebXRView(
            int time,
            Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
            Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation,
            int viewCount,
            Vector3 leftHandPosition, Vector3 rightHandPosition,
            Vector4 leftHandOrientation, Vector4 rightHandOrientation,
            int leftHandKey[7], int rightHandKey[7], Vector4 leftAxes, Vector4 rightAxes) override;

        /** called by the OpenXR frame loop before the engine renders. Keeps the stereo layout and
        * activates script side WebXRCamera once GameLogic is loaded. */
        void UpdatePicoXRFrame(const PicoXRFrameData& data);

        ATTRIBUTE_METHOD1(CParaWebXR, GetWebXRLeftFov_s, Vector4*) { *p1 = cls->m_webXRLeftFov; return S_OK; }
        ATTRIBUTE_METHOD1(CParaWebXR, GetWebXRRightFov_s, Vector4*) { *p1 = cls->m_webXRRightFov; return S_OK; }

        virtual int InstallFields(CAttributeClass* pClass, bool bOverride) override;

        /** angleLeft, angleRight, angleUp, angleDown in radians */
        Vector4 m_webXRLeftFov;
        Vector4 m_webXRRightFov;
    };
}
