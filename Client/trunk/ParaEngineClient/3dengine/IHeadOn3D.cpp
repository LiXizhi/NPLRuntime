//-----------------------------------------------------------------------------
// Class:	CParaEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Corporation
// Date:	2007.9.10
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIResource.h"
#include "2dengine/GUIText.h"
#include "PaintEngine/Painter.h"
#include "ParaScriptingGUI.h"
#include "ParaXAnimInstance.h"
#include "BipedObject.h"
#include "SceneObject.h"
#include "EffectManager.h"
#include "MiscEntity.h"

#include "IHeadOn3D.h"

using namespace ParaEngine;

/** default offset from the head attachment point in world unit*/
#define DEFAULT_OFFSET_Y	0.1f

HeadOn3DData::HeadOn3DData():m_vOffset(0,DEFAULT_OFFSET_Y,0),m_dwTextColor(0xff008f00),
	m_bShow(true), m_bZEnable(true), m_bEnable3DScaling(true), m_bUseGlobal3DScaling(true),m_f3DTextFacing(0.f),
	m_fNearZoomDist(15.f), m_fFarZoomDist(50.f), m_fMinUIScaling(0.5f), m_fMaxUIScaling(1.f),m_fAlphaFadePercentage(0.25f), m_bRender3DText(false)
{
#ifdef HEADON_UI_OBJ
	m_nUIObjID = 0;
#endif
}

IHeadOn3D::IHeadOn3D(void)
{
}

IHeadOn3D::~IHeadOn3D(void)
{
	
}

HeadOn3DData* ParaEngine::IHeadOn3D::GetHeadOn3DData(int nIndex)
{
	if((int)(m_data.size())>nIndex) 
		return &(m_data[nIndex]);
	else
	{
		m_data.resize(nIndex+1);
		return &(m_data[nIndex]);
	}
}

void ParaEngine::IHeadOn3D::SetHeadOnText( const char* sText, int nIndex )
{
	if(sText)
	{
		HeadOn3DData* pData = GetHeadOn3DData(nIndex);
		if(pData)
			pData->m_sText = sText;
	}
}

const char* ParaEngine::IHeadOn3D::GetHeadOnText(int nIndex)
{
	return 	(HasDataAt(nIndex)) ? GetHeadOn3DData(nIndex)->m_sText.c_str() : NULL;
}


void ParaEngine::IHeadOn3D::SetHeadOnUITemplateName( const char* sUIName, int nIndex)
{
	if(sUIName)
	{
		HeadOn3DData* pData = GetHeadOn3DData(nIndex);
		if(pData)
			pData->m_sUITemplateName = sUIName;
	}
}

const char* ParaEngine::IHeadOn3D::GetHeadOnUITemplateName(int nIndex)
{
	return 	(HasDataAt(nIndex)) ? GetHeadOn3DData(nIndex)->m_sUITemplateName.c_str() : NULL;
}

const std::string& ParaEngine::IHeadOn3D::GetHeadOnUITemplateNameS(int nIndex /*= 0*/)
{
	return 	(HasDataAt(nIndex)) ? GetHeadOn3DData(nIndex)->m_sUITemplateName : CGlobals::GetString();
}

void ParaEngine::IHeadOn3D::SetHeadOnTextColor( DWORD color, int nIndex )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_dwTextColor = color;
}

DWORD ParaEngine::IHeadOn3D::GetHeadOnTextColor(int nIndex)
{
	return 	(HasDataAt(nIndex)) ? GetHeadOn3DData(nIndex)->m_dwTextColor : 0xffffffff;
}

void ParaEngine::IHeadOn3D::SetHeadOnOffest( const Vector3& vOffset, int nIndex )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_vOffset = vOffset;
}

void ParaEngine::IHeadOn3D::GetHeadOnOffset( Vector3* pOut, int nIndex)
{
	if(HasDataAt(nIndex) && pOut!=0) 
	{
		*pOut = GetHeadOn3DData(nIndex)->m_vOffset;
	}
}

void ParaEngine::IHeadOn3D::ShowHeadOnDisplay( bool bShow, int nIndex )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_bShow = bShow;
}

bool ParaEngine::IHeadOn3D::IsHeadOnDisplayShown(int nIndex)
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_bShow : false;
}

#ifdef HEADON_UI_OBJ
int ParaEngine::IHeadOn3D::GetHeadOnUIObjID(int nIndex)
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_nUIObjID : 0;
}

void ParaEngine::IHeadOn3D::SetHeadOnUIObjID( int nID , int nIndex)
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_nUIObjID = nID;
}

bool ParaEngine::IHeadOn3D::HasHeadOnUIObj(int nIndex)
{
	return 	(HasDataAt(nIndex) && GetHeadOn3DData(nIndex)m_nUIObjID>0);
}
#endif


bool ParaEngine::IHeadOn3D::IsHeadOnZEnabled(int nIndex)
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_bZEnable : true;
}

void ParaEngine::IHeadOn3D::SetHeadOnZEnabled( bool bZnabled, int nIndex)
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_bZEnable = bZnabled;
}

bool ParaEngine::IHeadOn3D::IsHeadOn3DScalingEnabled( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_bEnable3DScaling : false;
}

void ParaEngine::IHeadOn3D::SetHeadOn3DScalingEnabled( bool bZnabled, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_bEnable3DScaling = bZnabled;
}

bool ParaEngine::IHeadOn3D::IsHeadOnUseGlobal3DScaling( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_bUseGlobal3DScaling : false;
}

void ParaEngine::IHeadOn3D::SetHeadOnUseGlobal3DScaling( bool bZnabled, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_bUseGlobal3DScaling = bZnabled;
}

void ParaEngine::IHeadOn3D::SetHeadOnNearZoomDist( float fValue, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_fNearZoomDist = fValue;
}

float ParaEngine::IHeadOn3D::GetHeadOnNearZoomDist( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_fNearZoomDist : 15.f;
}

void ParaEngine::IHeadOn3D::SetHeadOnFarZoomDist( float fValue, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_fFarZoomDist = fValue;
}

float ParaEngine::IHeadOn3D::GetHeadOnFarZoomDist( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_fFarZoomDist : 50.f;
}

void ParaEngine::IHeadOn3D::SetHeadOnMinUIScaling( float fValue, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_fMinUIScaling = fValue;
}

float ParaEngine::IHeadOn3D::GetHeadOnMinUIScaling( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_fMinUIScaling : 0.5f;
}

void ParaEngine::IHeadOn3D::SetHeadOnMaxUIScaling( float fValue, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_fMaxUIScaling = fValue;
}

float ParaEngine::IHeadOn3D::GetHeadOnMaxUIScaling( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_fMaxUIScaling : 1.f;
}

void ParaEngine::IHeadOn3D::SetHeadOnAlphaFadePercentage( float fValue, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
		pData->m_fAlphaFadePercentage = fValue;
}

float ParaEngine::IHeadOn3D::GetHeadOnAlphaFadePercentage( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_fAlphaFadePercentage : 0.25f;
}

void ParaEngine::IHeadOn3D::SetHeadOn3DFacing( float fValue, int nIndex /*= 0*/ )
{
	HeadOn3DData* pData = GetHeadOn3DData(nIndex);
	if(pData)
	{
		pData->m_f3DTextFacing = fValue;
		pData->m_bRender3DText = true;
	}
}

float ParaEngine::IHeadOn3D::GetHeadOn3DFacing( int nIndex /*= 0*/ )
{
	return 	HasDataAt(nIndex) ? GetHeadOn3DData(nIndex)->m_f3DTextFacing : 0;
}

bool ParaEngine::IHeadOn3D::DrawHeadOnUI(CBaseObject* pObj, int& nObjCount, SceneState* pSceneState, CGUIText** ppObjUITextDefault, bool bZEnablePass, bool b3DTextPass)
{
	using namespace ParaScripting;
	if(pObj == NULL)
	{
		if(nObjCount > 0)
		{
			EndPaint(pSceneState, b3DTextPass);
		}
		return false;
	}
	
	if( pObj->m_data.size()>0 && 
		((b3DTextPass && pObj->m_data[0].m_bRender3DText) || (!b3DTextPass && !(pObj->m_data[0].m_bRender3DText))) )
	{
	}
	else
		return true;

	//
	// for text head on display
	//
	for(int i=0;i < (int)(pObj->m_data.size()); ++i)
	{
		// get text, only draw if there is an non-empty text
		if(pObj->IsHeadOnDisplayShown(i))
		{
			CGUIBase* pObjUIText = NULL;
			int nUIType = Type_GUIText;
			const char* sText = pObj->GetHeadOnText(i);
			if(sText[0] =='\0')
			{
				CGUIBase* pObjUIText_ = pObj->GetHeadOnUIObject(i);
				if (pObjUIText_)
				{
					int nThisUIType = pObjUIText_->GetType()->GetTypeValue();
					// if ui is text but text attribute is empty, we will skip rendering.
					if(nThisUIType == Type_GUIText)
					{
						continue;
					}
					else
					{
						pObjUIText = pObjUIText_;
						nUIType = nThisUIType;
					}
				}
				else 
					continue;
			}

			// get the text 3d position in render coordinate system.
			Vector3 vPos = pObj->GetRenderOffset();
			Vector3 vOffset(0,0,0);
			pObj->GetHeadOnOffset(&vOffset, i);

			if(!b3DTextPass)
			{
				if(pObj->IsBiped())
				{
					// for biped object, we will try to find its head attachment, if there is no head attachment, the GetPhysicsHeight is used. 

					bool bHasHeadPoint = false;
					CBipedObject* pBiped = ((CBipedObject*)pObj);
#ifdef ALIGNWITH_WITH_HEAD_BONE
					CParaXAnimInstance* pParaXAI = pBiped->GetParaXAnimInstance();
					if(pParaXAI)
					{
						/// get world position from the mount point
						Matrix4 mxWorld;
						pBiped->GetRenderMatrix(mxWorld);

						Matrix4 mat;
						if(pParaXAI->GetAttachmentMatrix(&mat, ATT_ID_HEAD) != NULL)
						{
							mxWorld = mat*mxWorld;
							ParaVec3TransformCoord(&vPos, &vOffset,& mxWorld);
							bHasHeadPoint = true;
						}
					}
#endif
					if(!bHasHeadPoint)
					{
						vPos.y +=pBiped->GetHeight(); 
						vPos += vOffset;
					}
				}
				else
				{
					vPos.y += pObj->GetHeight(); 
					vPos += vOffset;
				}
			}
			
			// get the text color
			LinearColor textColor(pObj->GetHeadOnTextColor(i));
			

			if(nObjCount == 0)
			{
				// if this is the first time call, we will set up render states
				if (!BeginPaint(pSceneState, b3DTextPass, bZEnablePass))
					return false;
			}
			nObjCount++;

			// retrieve UI template to pObjUIText, if not template is specified, the default one is used. 
			if(pObjUIText == NULL)
			{
				CGUIBase* ObjUIText = pObj->GetHeadOnUIObject(i);
				if (ObjUIText)
				{
					int nThisUIType = ObjUIText->GetType()->GetTypeValue();
					// only support button and text at the moment.
					if(nThisUIType==Type_GUIText || nThisUIType ==Type_GUIButton)
					{
						pObjUIText = ObjUIText;
						nUIType = nThisUIType;
					}
				}
			}
			
			if(pObjUIText == NULL)
			{
				if((*ppObjUITextDefault) == NULL)
				{
					// get or create the UI text object for displaying the text
					using namespace ParaScripting;
					CGUIBase* pObjUIText_ = CGlobals::GetGUI()->GetUIObject("_HeadOnDisplayText_");
					if (!pObjUIText_)
					{
						ParaUIObject ObjUIText = ParaUI::CreateUIObject("text", "_HeadOnDisplayText_", "_lt", -100, -20, 200, 12);
						ObjUIText.SetFontString("System;12");
						ObjUIText.AttachToRoot();
						ObjUIText.SetVisible(false);
						ObjUIText.SetAutoSize(false);
						(*ppObjUITextDefault) = (CGUIText*)(ObjUIText.m_pObj);
						if((*ppObjUITextDefault))
						{
							GUIFontElement* pFont = (*ppObjUITextDefault)->GetFontElement("text");
							// set text alignment
							if(pFont)
								pFont->dwTextFormat=DT_CENTER|DT_NOCLIP;
						}
						pObjUIText_ = (CGUIText*)(ObjUIText.m_pObj);
					}
					(*ppObjUITextDefault) = (CGUIText*)pObjUIText_;

					if((*ppObjUITextDefault) == NULL)
						return false;
				}
				pObjUIText = (*ppObjUITextDefault);
			}

			// set text
			pObjUIText->SetTextA(sText);
			GUIFontElement* pFont = pObjUIText->GetFontElement("text");
			// set color
			if(pFont)
				pFont->FontColor=textColor;

			// set position
			if(b3DTextPass)
			{
				// for 3d text we will set the current world transform
				Matrix4 mat(Matrix4::IDENTITY);
				// m_fMaxUIScaling is used for 3d text scaling. 
				mat.setScale(Vector3(1 / 100.f*pObj->GetHeadOnMaxUIScaling(), 1 / 100.f*pObj->GetHeadOnMaxUIScaling(), 1));
				mat._41 = vOffset.x;
				mat._42 = vOffset.y;
				mat._43 = vOffset.z;

				// set model facing
				float fFacing = pObj->GetFacing()+pObj->GetHeadOn3DFacing();
				if(fFacing!=0.f)
				{
					Matrix4 mxRot;
					ParaMatrixRotationY( &mxRot, fFacing);
					mat *= mxRot;
				}
				Matrix4 localMat(Matrix4::IDENTITY);
				pObj->GetLocalTransform(&localMat);
				if (localMat != Matrix4::IDENTITY)
				{
					mat *= localMat;
				}
				mat._41 += vPos.x;
				mat._42 += vPos.y;
				mat._43 += vPos.z;

				auto painter = pObjUIText->GetPainter();
				painter->SetSpriteUseWorldMatrix(true);
				painter->PushMatrix();
				{
					painter->LoadMatrix(mat);
					pObjUIText->Draw();
					painter->PopMatrix();
				}
				
				painter->SetSpriteUseWorldMatrix(false);
			}
			else
			{
				CGUIPosition * pUIPos =  pObjUIText->GetPosition(); 
				pUIPos->SetPositionType(CGUIPosition::relative_to_3Dobject);
				pUIPos->Relative.To3D.fOffSet = 0.f;
				memcpy(pUIPos->Relative.To3D.m_v3DPosition, &vPos, sizeof(Vector3));

				Vector2 vOldScaling;
				pObjUIText->GetScaling(&vOldScaling);
				DWORD dwOldColorMask = pObjUIText->GetColorMask();
				float fAlphaFade = 1.f;

				// do UI scaling according to distance to camera
				if(pObj->IsHeadOn3DScalingEnabled())
				{
					float g_fNearZoomDist = 15.f;
					float g_fFarZoomDist = 50.f;
					float g_fMinUIScaling = 0.5f;
					float g_fMaxUIScaling = 1.f;
					// within which the alpha will change from 1 to 0 at the far end. 
					float g_fAlphaFadePercentage = 0.2f;
					bool bEnableUIScaling = true;
					if(pObj->IsHeadOnUseGlobal3DScaling())
					{
						if(CGlobals::GetScene()->IsHeadOn3DScalingEnabled())
						{
							g_fNearZoomDist = CGlobals::GetScene()->GetHeadOnNearZoomDist();
							g_fFarZoomDist = CGlobals::GetScene()->GetHeadOnFarZoomDist();
							g_fMinUIScaling = CGlobals::GetScene()->GetHeadOnMinUIScaling();
							g_fMaxUIScaling = CGlobals::GetScene()->GetHeadOnMaxUIScaling();
							g_fAlphaFadePercentage =  CGlobals::GetScene()->GetHeadOnAlphaFadePercentage();
						}
						else
						{
							bEnableUIScaling = false;
						}
					}
					else
					{
						g_fNearZoomDist = pObj->GetHeadOnNearZoomDist();
						g_fFarZoomDist = pObj->GetHeadOnFarZoomDist();
						g_fMinUIScaling = pObj->GetHeadOnMinUIScaling();
						g_fMaxUIScaling = pObj->GetHeadOnMaxUIScaling();
						g_fAlphaFadePercentage =  pObj->GetHeadOnAlphaFadePercentage();
					}
					if(bEnableUIScaling)
					{
						Vector3 vDistToCamera = (pSceneState->vEye - (vPos + pSceneState->GetScene()->GetRenderOrigin()));
						float fDist = vDistToCamera.squaredLength();

						float fUIScaling = g_fMaxUIScaling;

						if(fDist > g_fNearZoomDist*g_fNearZoomDist && g_fFarZoomDist != g_fNearZoomDist)
						{
							fDist = sqrt(fDist);
							if(fDist > g_fFarZoomDist)
							{
								fUIScaling = g_fMinUIScaling;
								if(g_fAlphaFadePercentage<1)
								{
									fAlphaFade = 0.f;
								}
							}
							else
							{
								fUIScaling = g_fMaxUIScaling - (fDist - g_fNearZoomDist) / (g_fFarZoomDist - g_fNearZoomDist) * (g_fMaxUIScaling-g_fMinUIScaling);
								if(g_fAlphaFadePercentage<1)
								{
									float fAlphaNearZoomDist = g_fNearZoomDist + (g_fFarZoomDist-g_fNearZoomDist)*(1-g_fAlphaFadePercentage);
									fAlphaFade = 1 - (fDist - fAlphaNearZoomDist) / (g_fFarZoomDist - fAlphaNearZoomDist) * (1-0);
								}
							}
						}
						if(fAlphaFade<1.f)
						{
							DWORD dwAlphaMask = ((DWORD)((int)((dwOldColorMask>>6)*fAlphaFade)));
							pObjUIText->SetColorMask((dwOldColorMask&0x00ffffff)| (dwAlphaMask<<6));
						}
						pObjUIText->SetScaling(vOldScaling*fUIScaling);
					}
				}
				// render the object
				if(fAlphaFade>0.f)
				{
					if(nUIType == Type_GUIText)
						((CGUIText*)pObjUIText)->RenderInBatch(NULL);
					else
					{
						pObjUIText->Draw();
					}
				}
				pObjUIText->SetScaling(vOldScaling);
				pObjUIText->SetColorMask(dwOldColorMask);
			}
			
		}
	}
#ifdef HEADON_UI_OBJ
	//
	// for UI object head on display: this is not used. 
	//
	if(pObj->HasHeadOnUIObj())
	{
		using namespace ParaScripting;
		ParaUIObject ObjUI = ParaUI::GetUIObject1(pObj->GetHeadOnUIObjID());
		if(ObjUI.IsValid() && ObjUI.m_pObj->GetVisible())
		{
			CGUIBase* pObjUI = ObjUI.m_pObj;
			// get the text 3d position in render coordinate system.
			Vector3 vPos = pObj->GetRenderOffset();
			Vector3 vOffset(0,0,0);
			pObj->GetHeadOnOffset(&vOffset);

			vPos.y += pObj->GetHeight(); 
			vPos += vOffset;

			// set position
			CGUIPosition * pUIPos =  pObjUI->GetPosition(); 
			pUIPos->SetPositionType(CGUIPosition::relative_to_3Dobject);
			pUIPos->Relative.To3D.fOffSet = 0.f;
			memcpy(pUIPos->Relative.To3D.m_v3DPosition, &vPos, sizeof(Vector3));

			
			// render the object, can this work with container
			pObjUI->Draw();
		}
		else
		{
			pObj->SetHeadOnUIObjID(0);
		}
	}
#endif

	return true;
}

bool ParaEngine::IHeadOn3D::BeginPaint(SceneState* pSceneState, bool b3DTextPass, bool bZEnablePass)
{
	RenderDevicePtr pD3dDevice = CGlobals::GetRenderDevice();

	auto painter = CGlobals::GetGUI()->GetPainter();
	painter->SetUse3DTransform(b3DTextPass);
	if (!painter->isActive())
	{
		// begin paint. 
		if (!painter->begin(CGlobals::GetGUI()))
			return false;
	}

	if (pSceneState->GetRenderState() == RenderState_Overlay_ZPass)
	{
		pD3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		pD3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATER);
		pD3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
	else
	{
		pD3dDevice->SetRenderState(D3DRS_ZENABLE, bZEnablePass ? TRUE : FALSE);
		pD3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
	
	if (b3DTextPass)
	{
		CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT, true);
		CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT, true);
	}
	return true;
}

void ParaEngine::IHeadOn3D::EndPaint(SceneState* pSceneState, bool b3DTextPass)
{
	auto painter = CGlobals::GetGUI()->GetPainter();
	if (painter->isActive())
	{
		painter->end();
	}

	RenderDevicePtr pD3dDevice = CGlobals::GetRenderDevice();
	if (pSceneState->GetRenderState() == RenderState_Overlay_ZPass)
	{
		pD3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
	else
	{
		pD3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		pD3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}

	if (b3DTextPass)
	{
		CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR, true);
		CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR, true);
	}
}

CGUIBase* ParaEngine::IHeadOn3D::GetHeadOnUIObject(int nIndex /*= 0*/)
{
	const std::string& sUITemplateName = GetHeadOnUITemplateNameS(nIndex);
	return  (!sUITemplateName.empty()) ? CGlobals::GetGUI()->GetUIObject(sUITemplateName) : NULL;
}

bool ParaEngine::IHeadOn3D::HasHeadOnDisplay(int nIndex/*=0*/)
{
	return ((int)(m_data.size()) > nIndex);
}
