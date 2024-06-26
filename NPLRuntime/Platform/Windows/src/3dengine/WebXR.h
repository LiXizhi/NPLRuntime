//-----------------------------------------------------------------------------
// Class:	WebXR
// Authors:	big
// Emails:	onedou@126.com
// CreateDate:	2024.6.19
//-----------------------------------------------------------------------------

#pragma once
#include "ParaEngine.h"
#include "IParaWebXR.h"

#pragma once
#include "ParaEngine.h"
#include "IParaWebXR.h"

namespace ParaEngine
{
    class CParaWebXR : public IParaWebXR
    {
    public:
        virtual void SetIsXR(bool isXR) override;
        virtual bool GetIsXR() override;
        virtual void UpdateWebXRView(
            int time,
            Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
            Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation, 
            int viewCount,
            Vector3 leftHandPosition, Vector3 rightHandPosition,
            Vector4 leftHandOrientation, Vector4 rightHandOrientation) override;
    };
}
