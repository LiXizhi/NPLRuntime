//-----------------------------------------------------------------------------
// Class:	WebXR
// Authors:	big
// Emails:	onedou@126.com
// CreateDate:	2024.6.19
// ModifyDate: 2024.8.12
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

        // Emulate WebXR.
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRTimeEmulate_s, string) { cls->m_webXRTime = atoi(p1.c_str()); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRLeftHandPositionEmulate_s, Vector3) { cls->m_webXRLeftHandPosition = p1; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRRightHandPositionEmulate_s, Vector3) { cls->m_webXRRightHandPosition = p1; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRLeftHandOrientationEmulate_s, Vector4) { cls->m_webXRLeftHandOrientation = p1; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRRightHandOrientationEmulate_s, Vector4) { cls->m_webXRRightHandOrientation = p1; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey0Emulate_s, bool) { cls->m_webXRLeftHandKey[0] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey1Emulate_s, bool) { cls->m_webXRLeftHandKey[1] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey2Emulate_s, bool) { cls->m_webXRLeftHandKey[2] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey3Emulate_s, bool) { cls->m_webXRLeftHandKey[3] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey4Emulate_s, bool) { cls->m_webXRLeftHandKey[4] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey5Emulate_s, bool) { cls->m_webXRLeftHandKey[5] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressLeftHandKey6Emulate_s, bool) { cls->m_webXRLeftHandKey[6] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey0Emulate_s, bool) { cls->m_webXRRightHandKey[0] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey1Emulate_s, bool) { cls->m_webXRRightHandKey[1] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey2Emulate_s, bool) { cls->m_webXRRightHandKey[2] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey3Emulate_s, bool) { cls->m_webXRRightHandKey[3] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey4Emulate_s, bool) { cls->m_webXRRightHandKey[4] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey5Emulate_s, bool) { cls->m_webXRRightHandKey[5] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetPressRightHandKey6Emulate_s, bool) { cls->m_webXRRightHandKey[6] = p1 ? 1 : 0; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRLeftAxesEmulate_s, Vector4) { cls->m_webXRLeftAxes = p1; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetWebXRRightAxesEmulate_s, Vector4) { cls->m_webXRRightAxes = p1; return S_OK; }

        virtual int InstallFields(CAttributeClass *pClass, bool bOverride) {
            IAttributeFields::InstallFields(pClass, bOverride);
            pClass->AddField("isXR", FieldType_Bool, (void*)SetIsXR_s, (void*)GetIsXR_s, NULL, NULL, bOverride);
            pClass->AddField("webXRTime", FieldType_Int, NULL, (void*)GetWebXRTime_s, NULL, NULL, bOverride);
            pClass->AddField("webXRTimeEmulate", FieldType_Int, (void*)SetWebXRTimeEmulate_s, NULL, NULL, NULL, bOverride);

            pClass->AddField("webXRLeftView", FieldType_Vector4, NULL, (void*)GetWebXRLeftView_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftPosition", FieldType_Vector3, NULL, (void*)GetWebXRLeftPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightView", FieldType_Vector4, NULL, (void*)GetWebXRRightView_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightPosition", FieldType_Vector3, NULL, (void*)GetWebXRRightPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRViewsCount", FieldType_Int, NULL, (void*)GetWebXRViewsCount_s, NULL, NULL, bOverride);

            pClass->AddField("webXRLeftOrientation", FieldType_Vector4, NULL, (void*)GetWebXRLeftOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightOrientation", FieldType_Vector4, NULL, (void*)GetWebXRRightOrientation_s, NULL, NULL, bOverride);

            pClass->AddField("webXRLeftHandPosition", FieldType_Vector3, NULL, (void*)GetWebXRLeftHandPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftHandPositionEmulate", FieldType_Vector3, (void*)SetWebXRLeftHandPositionEmulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("webXRRightHandPosition", FieldType_Vector3, NULL, (void*)GetWebXRRightHandPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightHandPositionEmulate", FieldType_Vector3, (void*)SetWebXRRightHandPositionEmulate_s, NULL, NULL, NULL, bOverride);

            pClass->AddField("webXRLeftHandOrientation", FieldType_Vector4, NULL, (void*)GetWebXRLeftHandOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftHandOrientationEmulate", FieldType_Vector4, (void*)SetWebXRLeftHandOrientationEmulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("webXRRightHandOrientation", FieldType_Vector4, NULL, (void*)GetWebXRRightHandOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightHandOrientationEmulate", FieldType_Vector4, (void*)SetWebXRRightHandOrientationEmulate_s, NULL, NULL, NULL, bOverride);

            pClass->AddField("leftHandKey0", FieldType_Bool, NULL, (void*)isPressLeftHandKey0_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey0Emulate", FieldType_Bool, (void*)SetPressLeftHandKey0Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey1", FieldType_Bool, NULL, (void*)isPressLeftHandKey1_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey1Emulate", FieldType_Bool, (void*)SetPressLeftHandKey1Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey2", FieldType_Bool, NULL, (void*)isPressLeftHandKey2_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey2Emulate", FieldType_Bool, (void*)SetPressLeftHandKey2Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey3", FieldType_Bool, NULL, (void*)isPressLeftHandKey3_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey3Emulate", FieldType_Bool, (void*)SetPressLeftHandKey3Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey4", FieldType_Bool, NULL, (void*)isPressLeftHandKey4_s, NULL, NULL, bOverride);
            pClass->AddField("lefthandKey4Emualte", FieldType_Bool, (void*)SetPressLeftHandKey4Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey5", FieldType_Bool, NULL, (void*)isPressLeftHandKey5_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey5Emulate", FieldType_Bool, (void*)SetPressLeftHandKey5Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey6", FieldType_Bool, NULL, (void*)isPressLeftHandKey6_s, NULL, NULL, bOverride);
            pClass->AddField("lefthandKey6Emulate", FieldType_Bool, (void*)SetPressLeftHandKey6Emulate_s, NULL, NULL, NULL, bOverride);
            
            pClass->AddField("rightHandKey0", FieldType_Bool, NULL, (void*)isPressRightHandKey0_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey1Emulate", FieldType_Bool, (void*)SetPressRightHandKey0Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey1", FieldType_Bool, NULL, (void*)isPressRightHandKey1_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey1Emulate", FieldType_Bool, (void*)SetPressRightHandKey1Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey2", FieldType_Bool, NULL, (void*)isPressRightHandKey2_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey2Emulate", FieldType_Bool, (void*)SetPressRightHandKey2Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey3", FieldType_Bool, NULL, (void*)isPressRightHandKey3_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey3Emulate", FieldType_Bool, (void*)SetPressRightHandKey3Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey4", FieldType_Bool, NULL, (void*)isPressRightHandKey4_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey4Emulate", FieldType_Bool, (void*)SetPressRightHandKey4Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey5", FieldType_Bool, NULL, (void*)isPressRightHandKey5_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey5Emulate", FieldType_Bool, (void*)SetPressRightHandKey5Emulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey6", FieldType_Bool, NULL, (void*)isPressRightHandKey6_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandKey6Emulate", FieldType_Bool, (void*)SetPressRightHandKey6Emulate_s, NULL, NULL, NULL, bOverride);

            pClass->AddField("webXRLeftAxes", FieldType_Vector4, NULL, (void*)GetWebXRLeftAxes_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftAxesEmulate", FieldType_Vector4, (void*)SetWebXRLeftAxesEmulate_s, NULL, NULL, NULL, bOverride);
            pClass->AddField("webXRRightAxes", FieldType_Vector4, NULL, (void*)GetWebXRRightAxes_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightAxesEmulate", FieldType_Vector4, (void*)SetWebXRRightAxesEmulate_s, NULL, NULL, NULL, bOverride);
            return S_OK;
        };
    };
}
