#pragma once

#include "IAttributeFields.h"

namespace ParaEngine {
    class IParaWebXR : public IAttributeFields
    {
    public:
        virtual void SetIsXR(bool isXR) {};
        virtual bool GetIsXR() { return false; };
        virtual void UpdateWebXRView(
            int time,
            Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
            Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation, 
            int viewCount,
            Vector3 leftHandPosition, Vector3 rightHandPosition,
            Vector4 leftHandOrientation, Vector4 rightHandOrientation,
            int leftHandKey[7], int rightHandKey[7], Vector4 leftAxes, Vector4 rightAxes) {};
        virtual bool IsPressLeftHandKey(int keyCode) { return false; };
        virtual bool IsPressRightHandKey(int keyCode) { return false; };

        ATTRIBUTE_METHOD1(IParaWebXR, GetIsXR_s, bool*) { *p1 = cls->m_isXR; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, SetIsXR_s, bool) { cls->SetIsXR((p1)); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRTime_s, int*) { *p1 = cls->m_webXRTime; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRLeftView_s, Vector4*) { *p1 = cls->m_webXRLeftView; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRLeftPosition_s, Vector3*) { *p1 = cls->m_webXRLeftPosition; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRLeftOrientation_s, Vector4*) { *p1 = cls->m_webXRLeftOrientation; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRRightView_s, Vector4*) { *p1 = cls->m_webXRRightView; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRRightPosition_s, Vector3*) { *p1 = cls->m_webXRRightPosition; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRRightOrientation_s, Vector4*) { *p1 = cls->m_webXRRightOrientation; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRViewsCount_s, int*) { *p1 = cls->m_webXRViewCount; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRLeftHandPosition_s, Vector3*) { *p1 = cls->m_webXRLeftHandPosition; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRRightHandPosition_s, Vector3*) { *p1 = cls->m_webXRRightHandPosition; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRLeftHandOrientation_s, Vector4*) { *p1 = cls->m_webXRLeftHandOrientation; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRRightHandOrientation_s, Vector4*) { *p1 = cls->m_webXRRightHandOrientation; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey0_s, bool*) { *p1 = cls->IsPressLeftHandKey(0); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey1_s, bool*) { *p1 = cls->IsPressLeftHandKey(1); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey2_s, bool*) { *p1 = cls->IsPressLeftHandKey(2); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey3_s, bool*) { *p1 = cls->IsPressLeftHandKey(3); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey4_s, bool*) { *p1 = cls->IsPressLeftHandKey(4); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey5_s, bool*) { *p1 = cls->IsPressLeftHandKey(5); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressLeftHandKey6_s, bool*) { *p1 = cls->IsPressLeftHandKey(6); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey0_s, bool*) { *p1 = cls->IsPressRightHandKey(0); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey1_s, bool*) { *p1 = cls->IsPressRightHandKey(1); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey2_s, bool*) { *p1 = cls->IsPressRightHandKey(2); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey3_s, bool*) { *p1 = cls->IsPressRightHandKey(3); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey4_s, bool*) { *p1 = cls->IsPressRightHandKey(4); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey5_s, bool*) { *p1 = cls->IsPressRightHandKey(5); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, isPressRightHandkey6_s, bool*) { *p1 = cls->IsPressRightHandKey(6); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRLeftAxes_s, Vector4*) { *p1 = cls->m_webXRLeftAxes; return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebXR, GetWebXRRightAxes_s, Vector4*) { *p1 = cls->m_webXRRightAxes; return S_OK; }

        virtual int InstallFields(CAttributeClass *pClass, bool bOverride) {
            IAttributeFields::InstallFields(pClass, bOverride);
            pClass->AddField("isXR", FieldType_Bool, (void*)SetIsXR_s, (void*)GetIsXR_s, NULL, NULL, bOverride);
            pClass->AddField("webXRTime", FieldType_Int, NULL, (void*)GetWebXRTime_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftView", FieldType_Vector4, NULL, (void*)GetWebXRLeftView_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftPosition", FieldType_Vector3, NULL, (void*)GetWebXRLeftPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightPosition", FieldType_Vector3, NULL, (void*)GetWebXRRightPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightView", FieldType_Vector4, NULL, (void*)GetWebXRRightView_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftOrientation", FieldType_Vector4, NULL, (void*)GetWebXRLeftOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightOrientation", FieldType_Vector4, NULL, (void*)GetWebXRRightOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("webXRViewsCount", FieldType_Int, NULL, (void*)GetWebXRViewsCount_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftHandPosition", FieldType_Vector3, NULL, (void*)GetWebXRLeftHandPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightHandPosition", FieldType_Vector3, NULL, (void*)GetWebXRRightHandPosition_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftHandOrientation", FieldType_Vector4, NULL, (void*)GetWebXRLeftHandOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightHandOrientation", FieldType_Vector4, NULL, (void*)GetWebXRRightHandOrientation_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey0", FieldType_Bool, NULL, (void*)isPressLeftHandKey0_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey1", FieldType_Bool, NULL, (void*)isPressLeftHandKey1_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey2", FieldType_Bool, NULL, (void*)isPressLeftHandKey2_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey3", FieldType_Bool, NULL, (void*)isPressLeftHandKey3_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey4", FieldType_Bool, NULL, (void*)isPressLeftHandKey4_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey5", FieldType_Bool, NULL, (void*)isPressLeftHandKey5_s, NULL, NULL, bOverride);
            pClass->AddField("leftHandKey6", FieldType_Bool, NULL, (void*)isPressLeftHandKey6_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey0", FieldType_Bool, NULL, (void*)isPressRightHandkey0_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey1", FieldType_Bool, NULL, (void*)isPressRightHandkey1_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey2", FieldType_Bool, NULL, (void*)isPressRightHandkey2_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey3", FieldType_Bool, NULL, (void*)isPressRightHandkey3_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey4", FieldType_Bool, NULL, (void*)isPressRightHandkey4_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey5", FieldType_Bool, NULL, (void*)isPressRightHandkey5_s, NULL, NULL, bOverride);
            pClass->AddField("rightHandkey6", FieldType_Bool, NULL, (void*)isPressRightHandkey6_s, NULL, NULL, bOverride);
            pClass->AddField("webXRLeftAxes", FieldType_Vector4, NULL, (void*)GetWebXRLeftAxes_s, NULL, NULL, bOverride);
            pClass->AddField("webXRRightAxes", FieldType_Vector4, NULL, (void*)GetWebXRRightAxes_s, NULL, NULL, bOverride);
            return S_OK;
        };

        bool m_isXR;
        int m_webXRTime;

        Vector4 m_webXRLeftView;
        Vector3 m_webXRLeftPosition;
        Vector4 m_webXRLeftOrientation;
        Vector4 m_webXRRightView;
        Vector3 m_webXRRightPosition;
        Vector4 m_webXRRightOrientation;
        int m_webXRViewCount;

        Vector3 m_webXRLeftHandPosition;
        Vector3 m_webXRRightHandPosition;
        Vector4 m_webXRLeftHandOrientation;
        Vector4 m_webXRRightHandOrientation;

        int m_webXRLeftHandKey[7];
        int m_webXRRightHandKey[7];
        Vector4 m_webXRLeftAxes;
        Vector4 m_webXRRightAxes;
    };

    class CParaWebXRFactory
    {
        public:
        static IParaWebXR *GetInstance();
    };
}
