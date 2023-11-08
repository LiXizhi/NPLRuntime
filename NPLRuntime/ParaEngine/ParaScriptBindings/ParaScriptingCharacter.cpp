//-----------------------------------------------------------------------------
// Class:	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//
// Revised 2: Andy Wang 2007.8
//		Note: add necessary Get*** functions to ParaCharacter class
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <time.h>
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "BipedObject.h"
#include "AutoCamera.h"
#include "CustomCharModelInstance.h"
#include "ParaXAnimInstance.h"
#include "AIModuleNPC.h"
#include "BipedController.h"
#include "BipedStateManager.h"
#include "MovieCtrlers.h"
#include "MissileObject.h"
#include "util/StringHelper.h"
#include "ParaScriptingScene.h"
#include "ParaScriptingCharacter.h"
#include "SequenceCtler.h"
#include "FaceTrackingCtrler.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif
extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace luabind;

#include "memdebug.h"

namespace ParaScripting
{
	/************************************************************************/
	/* ParaMovieCtrler                                                      */
	/************************************************************************/

	void ParaMovieCtrler::SaveMovie(const char* filename)
	{
		if(IsValid())
			m_pMovieCtrler->SaveToFile(filename);
	}

	void ParaMovieCtrler::LoadMovie(const char* filename)
	{
		if(IsValid())
			m_pMovieCtrler->LoadFromFile(filename);
	}

	void ParaMovieCtrler::Suspend()
	{
		if(IsValid())
			m_pMovieCtrler->SetMode(CMovieCtrler::MOVIE_SUSPENDED);
	}
	void ParaMovieCtrler::Resume()
	{
		if(IsValid())
			m_pMovieCtrler->Resume();
	}
	void ParaMovieCtrler::Play()
	{
		if(IsValid())
			m_pMovieCtrler->SetMode(CMovieCtrler::MOVIE_PLAYING);
	}
	void ParaMovieCtrler::Record()
	{
		if(IsValid())
			m_pMovieCtrler->SetMode(CMovieCtrler::MOVIE_RECORDING);
	}

	void ParaMovieCtrler::RecordNewDialog(const char* sDialog)
	{
		if(IsValid())
			m_pMovieCtrler->RecordDialogKey(sDialog);
	}
	bool ParaMovieCtrler::RecordNewAction(const char* sActionName)
	{
		if(IsValid())
			return m_pMovieCtrler->RecordActionKey(sActionName);
		return false;
	}
	bool ParaMovieCtrler::RecordNewAction_(const char* sActionName, float fTime)
	{
		if(IsValid())
			return m_pMovieCtrler->RecordActionKey(sActionName, fTime);
		return false;
	}

	void ParaMovieCtrler::RecordNewEffect(int effectID, const char* sTarget)
	{
		if(IsValid())
			m_pMovieCtrler->RecordEffectKey(effectID, sTarget);
	}
	void ParaMovieCtrler::GetOffsetPosition(float *x, float *y, float *z)
	{
		if(!IsValid())
		{
			*x=0;*y=0;*z=0;
			return;
		}
		const Vector3& v = m_pMovieCtrler->GetPosOffset();
		*x = v.x;*y = v.y;*z = v.z;
	}

	void ParaMovieCtrler::SetOffsetPosition(float x, float y, float z)
	{
		if(IsValid())
			m_pMovieCtrler->SetPosOffset(Vector3(x,y,z));
	}

	void ParaMovieCtrler::SetTime(float fTime)
	{
		if(IsValid())
			m_pMovieCtrler->SetTime(fTime);
	}

	string ParaMovieCtrler::GetTime(const char* sFormat)
	{
		string sTime="";
		int nTime = 0;

		// get the movie time
		CAIBase* pAIObj = NULL;
		if(IsValid())
			nTime = (int)m_pMovieCtrler->GetTime();

		if (sFormat && sFormat[0]== '\0')
		{
			char tmp[50];
			itoa(nTime, tmp, 10);
			sTime=tmp;
		}
		else
		{
			tm time;
			memset(&time, 0, sizeof(tm));
			time.tm_sec = nTime % 60;
			time.tm_min = (int(nTime/60)) % 60;
			time.tm_hour = (int(nTime/3600)) % 60;
			char tmp[100];
			strftime(tmp, 100, sFormat, &time);
			sTime = tmp;
		}
		return sTime;
	}

	/************************************************************************/
	/* ParaCharacter                                                        */
	/************************************************************************/
	ParaCharacter::ParaCharacter()
	{
	}
	
	ParaCharacter::ParaCharacter(CBaseObject* pObj)
	{
		if(pObj && (pObj->IsBiped()))
			m_pCharacter = (CBipedObject*)pObj;
	}
	ParaCharacter::~ParaCharacter()
	{
	}

	void ParaCharacter::FallDown()
	{
		if (m_pCharacter) {
			m_pCharacter->FallDown();
		}
	}

	void ParaCharacter::SetSizeScale(float fScale)
	{
		if (m_pCharacter) {
			m_pCharacter->SetSizeScale(fScale);
		}
	}

	float ParaCharacter::GetSizeScale()
	{
		if (m_pCharacter)
		{
			return m_pCharacter->GetSizeScale();
		}
		return 0.0f;
	}

	void ParaCharacter::SetSpeedScale(float fSpeedScale)
	{
		if (m_pCharacter) {
			m_pCharacter->SetSpeedScale(fSpeedScale);
		}
	}

	float ParaCharacter::GetSpeedScale()
	{
		if (m_pCharacter)
		{
			return m_pCharacter->GetSpeedScale();
		}
		return 0.0f;
	}

	void ParaCharacter::SetFocus()
	{
		if (m_pCharacter && m_pCharacter->IsGlobal()) {
			CGlobals::GetScene()->SetCurrentPlayer(m_pCharacter);
			((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->FollowBiped(m_pCharacter, CameraFollowThirdPerson, 0);
		}
	}

	void ParaCharacter::ResetBaseModel(ParaAssetObject assetCharBaseModel)
	{
		if (m_pCharacter && assetCharBaseModel.m_pAsset) {
			m_pCharacter->ResetBaseModel(assetCharBaseModel.m_pAsset);
			RefreshModel();
		}
	}

	void ParaCharacter::SetSkin(int nIndex)
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				pChar->SetSkin(nIndex);
			}
		}
	}

	int ParaCharacter::GetSkin()
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->GetSkin();
			}
		}
		return 0;
	}

	void ParaCharacter::LoadStoredModel(int nModelSetID)
	{
		if (m_pCharacter) {
			m_pCharacter->LoadStoredModel(nModelSetID);
			RefreshModel();
		}
	}
	
	void ParaCharacter::LoadFromFile(const char* filename)
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				//pChar->load(filename);
				//RefreshModel();
			}
		}
	}
	
	void ParaCharacter::SaveToFile(const char* filename)
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				//pChar->save(filename);
			}
		}
	}
	
	void ParaCharacter::SetBodyParams(int skinColor, int faceType, int hairColor, int hairStyle, int facialHair)
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				pChar->SetBodyParams(skinColor, faceType, hairColor, hairStyle, facialHair);
			}
		}
	}

	int ParaCharacter::GetBodyParams(int type)
	{
		if (m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->GetBodyParams(type);
			}
		}
		return 0;
	}

	void ParaCharacter::SetDisplayOptions(int bShowUnderwear, int bShowEars, int bShowHair)
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				pChar->SetDisplayOptions(bShowUnderwear, bShowEars, bShowHair);
			}
		}
	}

	bool ParaCharacter::GetDisplayOptions(int type)
	{
		if (m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->GetDisplayOptions(type);
			}
		}
		return true;
	}

	void ParaCharacter::SetCharacterSlot(int nSlotID, int nItemID)
	{
		if (m_pCharacter) 
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				pChar->SetCharacterSlot(nSlotID, nItemID);
			}
		}
	}

	int ParaCharacter::GetCharacterSlotItemID(int nSlotID)
	{
		if (m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->GetCharacterSlot(nSlotID);
			}
		}
		return -1;
	}

	void ParaCharacter::RefreshModel()
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				pChar->SetModified();
			}
		}
	}

	void ParaCharacter::PlayAnimation(const object& anims)
	{
		if (!m_pCharacter) 
			return;
		if (type(anims) == LUA_TSTRING)
		{
			const char * sAnim = object_cast<const char*>(anims);

			CBipedStateManager* pCharState = (m_pCharacter)->GetBipedStateManager();
			if(pCharState)
			{
				ActionKey act(sAnim);
				pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, &act);
			}
		}
		else if (type(anims) == LUA_TNUMBER)
		{
			int nAnimID = object_cast<int>(anims);
			CBipedStateManager* pCharState = (m_pCharacter)->GetBipedStateManager();
			if(pCharState)
			{
				ActionKey act(nAnimID);
				pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, &act);
			}
		}
		else if (type(anims) == LUA_TTABLE)
		{
			int Anims[2] = {-1,-1};
			for (luabind::iterator itCur(anims), itEnd;itCur!=itEnd;++itCur)
			{
				const object& key = itCur.key();
				const object& anim = *itCur;
				if(type(key) == LUA_TNUMBER)
				{
					int nKey = object_cast<int>(key);
					if(nKey>=1 && nKey<=2)
					{
						if(type(anim) == LUA_TNUMBER)
						{
							Anims[nKey-1] = object_cast<int>(anim);
						}
					}
				}
			}
			for (int i=0;i<2;++i)
			{
				if(Anims[i] != -1)
				{
					m_pCharacter->PlayAnimation(Anims[i], true, true);
				}
			}
		}
	}

	void ParaCharacter::PlayAnimation2(int animid, bool bUpdateSpeed, bool bAppend)
	{
		m_pCharacter->PlayAnimation(animid, bUpdateSpeed, bAppend);
	}

	void ParaCharacter::Stop()
	{
		if (m_pCharacter) {
			CSequenceCtler::Stop(m_pCharacter);
		}
	}

	void ParaCharacter::MoveTo(double x, double y, double z)
	{
		if (m_pCharacter) {
			DVector3 v(x, y, z);
			CSequenceCtler::MoveCommand(m_pCharacter, v);
		}
	}
	
	void ParaCharacter::MoveAndTurn(double x, double y, double z, float facing)
	{
		if (m_pCharacter) {
			DVector3 v(x, y, z);
			CSequenceCtler::MoveCommand(m_pCharacter, v, false, false, &facing);
		}
	}


	void ParaCharacter::UseAIModule(const char* sAIType )
	{
		if (m_pCharacter) {
			m_pCharacter->UseAIModule(sAIType);
		}
	}

	ParaMovieCtrler ParaCharacter::GetMovieController()
	{
		CMovieCtrler* pMovieCtrler= NULL;

		if (m_pCharacter) 
		{
			/// if there is no AI object, we will create a default one to use
			CAIBase* pAIObj = NULL;
			if((pAIObj=m_pCharacter->GetAIModule()) == NULL)
				pAIObj = m_pCharacter->UseAIModule(""); 
			if(pAIObj) 
			{
				if(pAIObj->GetType() == CAIBase::NPC)
				{
					pMovieCtrler = ((CAIModuleNPC*)pAIObj)->GetMovieController();
					if(pMovieCtrler == NULL)
					{
						AssignAIController("movie","true");
						pMovieCtrler = ((CAIModuleNPC*)pAIObj)->GetMovieController();
					}
				}
			}
		}
		return ParaMovieCtrler(pMovieCtrler);
	}
	ParaSeqCtrler ParaCharacter::GetSeqController()
	{
		CSequenceCtler* pCtrler= NULL;

		if (m_pCharacter) 
		{
			/// if there is no AI object, we will create a default one to use
			CAIBase* pAIObj = NULL;
			if((pAIObj=m_pCharacter->GetAIModule()) == NULL)
				pAIObj = m_pCharacter->UseAIModule(""); 
			if(pAIObj) 
			{
				if(pAIObj->GetType() == CAIBase::NPC)
				{
					pCtrler = ((CAIModuleNPC*)pAIObj)->GetSequenceController();
					if(pCtrler == NULL)
					{
						AssignAIController("sequence","true");
						pCtrler = ((CAIModuleNPC*)pAIObj)->GetSequenceController();
					}
				}
			}
		}
		return ParaSeqCtrler(pCtrler);
	}

	ParaFaceTrackingCtrler ParaCharacter::GetFaceTrackingController()
	{
		CFaceTrackingCtrler* pCtrler= NULL;
		if (m_pCharacter) 
		{
			/// if there is no AI object, we will create a default one to use
			CAIBase* pAIObj = NULL;
			if((pAIObj=m_pCharacter->GetAIModule()) == NULL)
				pAIObj = m_pCharacter->UseAIModule(""); 
			if(pAIObj) 
			{
				if(pAIObj->GetType() == CAIBase::NPC)
				{
					pCtrler = ((CAIModuleNPC*)pAIObj)->GetFaceController();
					if(pCtrler == NULL)
					{
						AssignAIController("face","true");
						pCtrler = ((CAIModuleNPC*)pAIObj)->GetFaceController();
					}
				}
			}
		}
		return ParaFaceTrackingCtrler(pCtrler);
	}
	

	bool ParaCharacter::IsAIControllerEnabled(const char* sAICtrlerName)
	{
		bool bEnabled = false;
		if (m_pCharacter && sAICtrlerName!=NULL) 
		{
			/// if there is no AI object, return false
			CAIBase* pAIObj = NULL;
			if((pAIObj=m_pCharacter->GetAIModule()) == NULL)
				return false;
			string sCtrlerName(sAICtrlerName);
			if (sCtrlerName == "face")
			{ 
				if(pAIObj->GetType() == CAIBase::NPC)
					bEnabled = ((CAIModuleNPC*)pAIObj)->IsFaceTrackingEnabled();
			}
			else if (sCtrlerName == "follow")
			{ 
				if(pAIObj->GetType() == CAIBase::NPC)
					bEnabled = ((CAIModuleNPC*)pAIObj)->IsFollowEnabled();
			}
			else if (sCtrlerName == "movie")
			{
				if(pAIObj->GetType() == CAIBase::NPC)
					bEnabled = ((CAIModuleNPC*)pAIObj)->IsMovieControlerEnabled();
			}
			else if (sCtrlerName == "sequence")
			{
				if(pAIObj->GetType() == CAIBase::NPC)
					bEnabled = ((CAIModuleNPC*)pAIObj)->IsSequenceControlerEnabled();
			}
		}
		return bEnabled;
	}

	void ParaCharacter::AssignAIController(const char* sAICtrlerName, const char* sParam1)
	{
		if (m_pCharacter && sAICtrlerName!=NULL) {
			/// if there is no AI object, we will create a default one to use
			CAIBase* pAIObj = NULL;
			if((pAIObj=m_pCharacter->GetAIModule()) == NULL)
				pAIObj = m_pCharacter->UseAIModule(""); 
			if(!pAIObj) 
				return;
			string sCtrlerName(sAICtrlerName);
			string sParam(sParam1);
			if (sCtrlerName == "face")
			{ 
				/*
				* sAICtrlerName = "face": Face tracking controller:
				* sParam1 = "true"|"false": "true" to enable face tracking.
				* e.g. Char:AssignAIController("face", true);
				*/
				if(pAIObj->GetType() == CAIBase::NPC)
					((CAIModuleNPC*)pAIObj)->EnableFaceTracking((sParam=="true"));
			}
			else if (sCtrlerName == "follow")
			{ 
				/*
				* sAICtrlerName = "follow": follow another named biped:
				* sParam1 = ""|"name": "" to disable following, or follow a biped named "name".
				* e.g. Char:AssignAIController("follow", "player1");
				*/
				if(pAIObj->GetType() == CAIBase::NPC)
				{
					if(sParam == "false")
					{
						sParam.clear();
					}
					((CAIModuleNPC*)pAIObj)->FollowBiped(sParam);
				}
			}
			else if (sCtrlerName == "movie")
			{
				/*
				* sAICtrlerName = "movie": enable a movie controller.
				* sParam1 = ""|"true"|"false": "" or "true" to enable a movie, or "false" to disable it.
				* e.g. Char:AssignAIController("movie", "true");Char:AssignAIController("movie", "false");
				*/
				if(pAIObj->GetType() == CAIBase::NPC)
					((CAIModuleNPC*)pAIObj)->EnableMovieControler(sParam!="false");
			}
			else if (sCtrlerName == "sequence")
			{
				/*
				* sAICtrlerName = "sequence": enable a sequence controller.
				* sParam1 = ""|"true"|"false": "" or "true" to enable a sequence, or "false" to disable it.
				* e.g. Char:AssignAIController("sequence", "true");Char:AssignAIController("sequence", "false");
				*/
				if(pAIObj->GetType() == CAIBase::NPC)
					((CAIModuleNPC*)pAIObj)->EnableSequenceControler(sParam!="false");
			}
		}
	}


	ParaScripting::ParaAttributeObject ParaCharacter::GetAttachmentAttObj( int nAttachmentID )
	{
		if (m_pCharacter) 
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar!=0)
			{
				return ParaAttributeObject(pChar->GetAttachmentAttObj(nAttachmentID));
			}
		}
		return ParaAttributeObject();
	}


	void ParaCharacter::AddAttachment5(ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID, float fScaling, ParaAssetObject ReplaceableTexture)
	{
		AddAttachment6(ModelAsset, nAttachmentID, nSlotID, fScaling, ReplaceableTexture, 2);
	}

	void ParaCharacter::AddAttachment6(ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID, float fScaling, ParaAssetObject ReplaceableTexture, int replaceableTextureID)
	{
		CharModelInstance* pChar = m_pCharacter->GetCharModelInstance();
		if (pChar != 0)
		{
			TextureEntity* pTex = NULL;
			if (ReplaceableTexture.m_pAsset && ReplaceableTexture.m_pAsset->GetType() == AssetEntity::texture)
			{
				pTex = (TextureEntity*)(ReplaceableTexture.m_pAsset);
			}
			if (ModelAsset.m_pAsset->GetType() == AssetEntity::parax)
			{
				pChar->AddAttachment((ParaXEntity*)ModelAsset.m_pAsset, nAttachmentID, nSlotID, fScaling, pTex, replaceableTextureID);
			}
			else if (ModelAsset.m_pAsset->GetType() == AssetEntity::mesh)
			{
				pChar->AddAttachment((MeshEntity*)ModelAsset.m_pAsset, nAttachmentID, nSlotID, fScaling, pTex);
			}
		}
	}

	void ParaCharacter::AddAttachment3(ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID)
	{
		AddAttachment5(ModelAsset, nAttachmentID, nSlotID, 1.0f, ParaAssetObject());
	}

	void ParaCharacter::AddAttachment4( ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID, float fScaling )
	{
		AddAttachment5(ModelAsset, nAttachmentID, nSlotID, fScaling, ParaAssetObject());
	}

	void ParaCharacter::AddAttachment(ParaAssetObject ModelAsset, int nAttachmentID)
	{
		AddAttachment5(ModelAsset, nAttachmentID, -1, 1.0f, ParaAssetObject());
	}

	void ParaCharacter::RemoveAttachment(int nAttachmentID)
	{
		RemoveAttachment2(nAttachmentID, -1);
	}

	void ParaCharacter::RemoveAttachment2(int nAttachmentID, int nSlotID)
	{
		CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
		if(pChar!=0)
		{
			pChar->AddAttachment((ParaXEntity*)NULL, nAttachmentID, nSlotID);
		}
	}

	ParaParamBlock ParaCharacter::GetAttachmentParamBlock(int attachmentID, int slotID)
	{
		if (m_pCharacter)
		{
			CharModelInstance* pChar = m_pCharacter->GetCharModelInstance();
			if (pChar != 0)
			{
				return ParaParamBlock(pChar->GetAttachmentParamBlock(attachmentID, slotID));
			}
		}
		return ParaParamBlock();
	}

	void ParaCharacter::CastEffect(int nEffectID)
	{
		CastEffect2(nEffectID, "");
	}

	void ParaCharacter::CastEffect2(int nEffectID, const char* sTarget_)
	{
		const string sTarget = sTarget_;
		if (m_pCharacter) {
			CBipedStateManager* pCharState =  m_pCharacter->GetBipedStateManager();
			if(pCharState && pCharState->IsRecording())
			{
				// if it is recording, add the effect key frame.
				// record the state
				ParaScripting::ParaMovieCtrler movieCtler = (ParaScripting::ParaCharacter(m_pCharacter)).GetMovieController();
				movieCtler.RecordNewEffect(nEffectID, sTarget.c_str());
			}
			if(sTarget=="")
			{
				// if there is no target, just attach it to the character model's hand attachment by default.
				CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
				if(pChar)
				{
					pChar->CastEffect(nEffectID);
				}
			}
			else if(sTarget[0]=='<')
			{
				string sAttachmentID = sTarget.substr(1, sTarget.size()-2);
				if( ! sAttachmentID.empty() )
				{
					int nAttachmentID=0;
					try{
						nAttachmentID=atoi(sAttachmentID.c_str());
					}
					catch(...)
					{
						OUTPUT_LOG("invalid cast effect attachment ID.\n");
						nAttachmentID=0; // default to 0
					}
					// if there is no target, just attach it to the character model
					CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
					if(pChar)
					{
						pChar->CastEffect(nEffectID, nAttachmentID);
					}
				}
			}
			else
			{
				ParaXEntity* EffectModel = CGlobals::GetAssetManager()->LoadParaXByID(nEffectID);
				if(EffectModel)
				{
					// otherwise, it should be treated as global missile object.
					CMissileObject* pMissile =  CGlobals::GetScene()->NewMissile();
					if(pMissile)
					{
						pMissile->InitObject(EffectModel, sTarget, 10.f);
						DVector3 vSrcPos = m_pCharacter->GetObjectCenter();
						pMissile->SetPosition(vSrcPos);

					}
					else
					{
						OUTPUT_LOG("failed creating missile object.\n");
					}
				}
			}
		}
	}

	void ParaCharacter::MountOn2(ParaObject& target, int nMountID)
	{
		if (m_pCharacter && target.IsValid())  
		{
			m_pCharacter->MountOn(target.m_pObj, nMountID);
		}

	}

	void ParaCharacter::MountOn(ParaObject& target)
	{
		MountOn2(target, -1);
	}

	void ParaCharacter::MountOn_(const char* sTarget)
	{
		if (m_pCharacter) 
		{
			CBaseObject* pMountTargetObj = CGlobals::GetScene()->GetGlobalObject(sTarget);
			if(pMountTargetObj)
			{
				m_pCharacter->MountOn(pMountTargetObj);
			}
		}
	}

	void ParaCharacter::UnMount()
	{
		if (m_pCharacter) 
		{
			m_pCharacter->MountOn(NULL);
		}
	}

	void ParaCharacter::AddAction(int nAction)
	{
		if(m_pCharacter)
		{
			CBipedStateManager* pCharState = m_pCharacter->GetBipedStateManager();
			if(pCharState)
			{
				pCharState->AddAction((CBipedStateManager::ActionSymbols)nAction);
			}
		}
	}
	void ParaCharacter::AddAction2( int nAction, const object& param)
	{
		if(m_pCharacter)
		{
			CBipedStateManager* pCharState = m_pCharacter->GetBipedStateManager();
			if(pCharState)
			{
				int nType = type(param);
				if(nAction == CBipedStateManager::S_ACTIONKEY)
				{
					if (nType == LUA_TSTRING)
					{
						string str = object_cast<const char*>(param);
						ActionKey act(str);
						pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, &act);
					}
					else if(nType == LUA_TNUMBER)
					{
						int nValue = (int)object_cast<double>(param);
						ActionKey act(static_cast<ActionKey::KeyID>(nValue));
						pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, (const void*)&act);
					}
					else
					{
						pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, "");
					}
				}
				else
				{
					if (nType == LUA_TSTRING)
					{
						string str = object_cast<const char*>(param);
						pCharState->AddAction((CBipedStateManager::ActionSymbols)nAction, str.c_str());
					}
					else if (nType == LUA_TNUMBER)
					{
						double dValue = (double)object_cast<double>(param);
						pCharState->AddAction((CBipedStateManager::ActionSymbols)nAction, &dValue);
						if (nAction == CBipedStateManager::S_WALK_FORWORD)
						{
							pCharState->SetAngleDelta((float)dValue);
						}
					}
					else
					{
						pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, "");
					}
				}
			}
		}
	}
	bool ParaCharacter::WalkingOrRunning()
	{
		if(m_pCharacter)
		{
			CBipedStateManager* pCharState = m_pCharacter->GetBipedStateManager();
			return (pCharState!=NULL) && pCharState->WalkingOrRunning();
		}
		return true;
	}

	bool ParaCharacter::HasMountPoint( int nMountPointID )
	{
		if(m_pCharacter)
		{
			CParaXAnimInstance* pParaXAI = m_pCharacter->GetParaXAnimInstance();
			return (pParaXAI && pParaXAI->HasMountPoint(nMountPointID));
		}
		return false;
	}

	bool ParaCharacter::IsMounted()
	{
		if(m_pCharacter)
		{
			CBipedStateManager* pCharState = m_pCharacter->GetBipedStateManager();
			if(pCharState)
				return pCharState->IsMounted();
		}
		return false;
	}

	void ParaCharacter::SetSkinColorMask(const char * strColor)
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				DWORD dwColor = 0;
				if(strColor[0] == 'F' || (strColor[1] == '\0'))
				{
					dwColor = 0xffffffff;
				}
				else
				{
					sscanf(strColor, "%x", &dwColor);
					dwColor |= 0xff000000;
				}
				pChar->SetSkinColorMask(dwColor);
			}
		}
	}
	
	string ParaCharacter::GetSkinColorMask()
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				DWORD dwColor = (pChar->GetSkinColorMask());
				if(dwColor == 0xffffffff || dwColor == 0)
				{
					return "F";
				}
				else
				{
					char tmp[32];
					snprintf(tmp, 32, "%x", (dwColor&0x00ffffff));
					return string(tmp);
				}
			}
		}
		return "F";
	}

	string ParaCharacter::GetCartoonFaceComponentCode(int nComponentID, int SubType)
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				CartoonFace* pCartoonFace =  pChar->GetCartoonFace();
				if(pCartoonFace)
				{
					if(nComponentID>=0 && nComponentID<=CFS_TOTAL_NUM)
					{
						FaceComponent& component = pCartoonFace->GetComponent(nComponentID);
						switch(SubType)
						{
						case 0:
							{
								int nStyle = component.GetStyle();
								if(nStyle != 0 )
								{
									char tmp[32];
									ParaEngine::StringHelper::fast_itoa(nStyle, tmp, 32);
									return string(tmp);
								}
								break;
							}
						case 1:
							{
								DWORD tcolor = component.GetColor();
								if(tcolor == 0xffffffff)
								{
									return "F";
								}
								else if(tcolor != 0)
								{
									char tmp[32];
									snprintf(tmp, 32, "%x", (DWORD)(tcolor&0x00ffffff));
									return string(tmp);
								}
								break;
							}
						case 2:
							{
								float fScale = component.GetScaling();
								if(fScale != 0.f)
								{
									char tmp[4];
									tmp[1] = '\0';
									if(fScale > 0.f)
									{
										tmp[0] = '0'+(int)((fScale-0.001f)*10);
									}
									else
									{
										tmp[0] = 'a'+(int)((-fScale-0.001f)*7);
									}
									return string(tmp);
								}
								break;
							}
						case 3:
							{
								float fRot = component.GetScaling();
								if(fRot != 0.f)
								{
									char tmp[4];
									tmp[1] = '\0';
									if(fRot > 0.f)
									{
										
									}
									else
									{
										
									}
									// TODO: 
								}
								break;
							}
						case 4:
							{
								int x = 0;
								component.GetPosition(&x, NULL);
								// TODO:
								break;
							}
						case 5:
							{
								int y = 0;
								component.GetPosition( NULL, &y );
								// TODO:
								break;
							}
						}
					}
				}
			}
		}
		return "0";	
	}

	void ParaCharacter::SetCartoonFaceComponentCode(int nComponentID, int SubType, const char * color)
	{
		// TODO:
	}

	double ParaCharacter::GetCartoonFaceComponent( int nComponentID, int SubType)
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				CartoonFace* pCartoonFace =  pChar->GetCartoonFace();
				if(pCartoonFace)
				{
					if(nComponentID>=0 && nComponentID<=CFS_TOTAL_NUM)
					{
						FaceComponent& component = pCartoonFace->GetComponent(nComponentID);
						switch(SubType)
						{
						case 0:
							return (double)component.GetStyle();
						case 1:
							// 2007.12.7: Minor bug: GetCartoonFaceComponent() sometimes returns unpredictable number for the color component. 
							// I think it is a minor bug that DWORD(0xffffffff) is sometimes converted to 4294967296, instead of 4294967295 randomly. 
							return (double) component.GetColor();
						case 2:
							return (double)component.GetScaling();
						case 3:
							return (double)component.GetRotation();
						case 4:
							{
								int x = 0;
								component.GetPosition(&x, NULL);
								return (double)x;
							}
						case 5:
							{
								int y = 0;
								component.GetPosition( NULL, &y );
								return (double)y;
							}
						}
					}
				}
			}
		}
		return 0.f;
	}

	void ParaCharacter::SetCartoonFaceComponent( int nComponentID, int SubType, double value )
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				CartoonFace* pCartoonFace =  pChar->GetCartoonFace();
				if(pCartoonFace)
				{
					if(nComponentID>=0 && nComponentID<=CFS_TOTAL_NUM)
					{
						FaceComponent& component = pCartoonFace->GetComponent(nComponentID);
						switch(SubType)
						{
						case 0:
							if(component.GetStyle() != ((int)value))
							{
								component.SetStyle((int)value);
								pCartoonFace->SetModified(true);
							}
							break;
						case 1:
							if(component.GetColor()!=((DWORD)((unsigned long long)value)))
							{
								component.SetColor((DWORD)((unsigned long long)value));
								pCartoonFace->SetModified(true);
							}
							break;
						case 2:
							if(component.GetScaling() != ((float)value))
							{
								component.SetScaling((float)value);
								pCartoonFace->SetModified(true);
							}
							break;
						case 3:
							if(component.GetRotation() != ((float)value))
							{
								component.SetRotation((float)value);
								pCartoonFace->SetModified(true);
							}
							break;
						case 4:
							{
								int x = (int)value;
								int oldX = 0;
								component.GetPosition(&oldX,NULL);
								if(oldX != x)
								{
									component.SetPosition(&x, NULL);
									pCartoonFace->SetModified(true);
								}
								break;
							}
						case 5:
							{
								int y = (int)value;
								int oldY = 0;
								component.GetPosition(NULL, &oldY);
								if(oldY != y)
								{
									component.SetPosition(NULL, &y);
									pCartoonFace->SetModified(true);
								}
								break;
							}
						}
					}
				}
			}
		}
	}

	bool ParaCharacter::IsSupportCartoonFace()
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->IsSupportCartoonFace();
			}
		}
		return false;
	}

	bool ParaCharacter::IsCustomModel()
	{
		if(m_pCharacter)
		{
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->IsCustomModel();
			}
		}
		return false;
	}


	int ParaCharacter::GetGender()
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->gender;
			}
		}
		return -1;
	}

	int ParaCharacter::GetRaceID()
	{
		if (m_pCharacter) {
			CharModelInstance* pChar =  m_pCharacter->GetCharModelInstance();
			if(pChar)
			{
				return pChar->race;
			}
		}
		return -1;
	}

	void ParaCharacter::EnableAnimIDMap( bool bEnable )
	{
		if (m_pCharacter) {
			m_pCharacter->GetAnimInstance()->EnableAnimIDMap(bEnable);
		}
	}

	bool ParaCharacter::IsAnimIDMapEnabled()
	{
		if (m_pCharacter) {
			return m_pCharacter->GetAnimInstance()->IsAnimIDMapEnabled();
		}
		return false;
	}

	// TODO: not tested
	bool ParaCharacter::AddAnimIDMap( int nFromID, int nToID )
	{
		if (m_pCharacter) 
		{
			map<int, int>* pMap = m_pCharacter->GetAnimInstance()->GetAnimIDMap();
			if (pMap)
			{
				map<int, int>::iterator it = pMap->find(nFromID);
				if(it!=pMap->end())
				{
					if(nToID<0)
					{
						pMap->erase(it);
					}
					else
					{
						(*it).second = nToID;
					}
				}
				else
				{
					if(nToID>=0)
					{
						(*pMap)[nFromID] = nToID;
					}
				}
			}
		}
		return true;
	}

	// TODO: not tested
	void ParaCharacter::ClearAllAnimIDMap()
	{
		if (m_pCharacter) 
		{
			map<int, int>* pMap = m_pCharacter->GetAnimInstance()->GetAnimIDMap();
			if (pMap)
			{
				pMap->clear();
			}
		}
	}

	int ParaCharacter::GetAnimID()
	{
		if (m_pCharacter) 
		{
			return m_pCharacter->GetCurrentAnimation();
		}
		return -1;
	}

	const char* ParaCharacter::GetAnimFileName()
	{
		int nAnimID = GetAnimID();
		if(nAnimID>=2000)
		{
			return ParaAsset::GetBoneAnimProviderFileNameByID(nAnimID);
		}
		return NULL;
	}

	bool ParaCharacter::HasAnimation( int nAnimID )
	{
		if (m_pCharacter) 
		{
			return m_pCharacter->HasAnimation(nAnimID);
		}
		return false;
	}

	bool ParaCharacter::IsValid()
	{
		return m_pCharacter;
	}

	void ParaCharacter::EnableAutoAnimation(bool enable)
	{
		CBipedStateManager* pCharState = (m_pCharacter)->GetBipedStateManager();
		if (pCharState)
		{
			pCharState->EnableAutoAnimation(enable);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// ParaSeqCtrler
	//
	//////////////////////////////////////////////////////////////////////////
	bool ParaSeqCtrler::Save(bool bOverride){return m_pSequenceCtrler->Save(bOverride);};
	bool ParaSeqCtrler::LoadByID(int nSequenceID){return m_pSequenceCtrler->Load(nSequenceID);};
	bool ParaSeqCtrler::LoadFromFile(const char* fileName){return m_pSequenceCtrler->Load(fileName);};
	int  ParaSeqCtrler::Create(const char* name, const char* description, const char* pData,bool bInMemory)
	{return m_pSequenceCtrler->Create(name, description, pData, bInMemory);};
	string ParaSeqCtrler::ToString(){return m_pSequenceCtrler->ToString();};

	int ParaSeqCtrler::GetKeyPos(){return m_pSequenceCtrler->GetKeyPos();};
	void ParaSeqCtrler::SetKeyPos(int nPos){m_pSequenceCtrler->SetKeyPos(nPos);};

	int ParaSeqCtrler::GetTotalKeys(){return m_pSequenceCtrler->GetTotalKeys();};
	int ParaSeqCtrler::AdvanceKey(int nOffset){return m_pSequenceCtrler->AdvanceKey(nOffset);};
	void ParaSeqCtrler::BeginAddKeys(){m_pSequenceCtrler->BeginAddKeys();};
	void ParaSeqCtrler::EndAddKeys(){m_pSequenceCtrler->EndAddKeys();};

	int ParaSeqCtrler::GetSequenceID(){return m_pSequenceCtrler->GetSequenceID();};
	bool ParaSeqCtrler::DeleteKeysRange(int nFrom, int nTo){return m_pSequenceCtrler->DeleteKeysRange(nFrom, nTo);};

	bool ParaSeqCtrler::GetPlayDirection(){return m_pSequenceCtrler->GetPlayDirection();};
	void ParaSeqCtrler::SetPlayDirection(bool bForward){m_pSequenceCtrler->SetPlayDirection(bForward);};

	float ParaSeqCtrler::GetInterval(){return m_pSequenceCtrler->GetInterval();}
	void ParaSeqCtrler::SetInterval(float fInterval){m_pSequenceCtrler->SetInterval(fInterval);}

	void ParaSeqCtrler::GetStartPos(float& x, float& y, float&z){
		Vector3 vPos = m_pSequenceCtrler->GetStartPos();
		x=vPos.x;
		y=vPos.y;
		z=vPos.z;
	}
	void ParaSeqCtrler::SetStartPos(float x, float y, float z){m_pSequenceCtrler->SetStartPos(Vector3(x,y,z));}

	float ParaSeqCtrler::GetStartFacing(){return m_pSequenceCtrler->GetStartFacing();}
	void ParaSeqCtrler::SetStartFacing(float facing){m_pSequenceCtrler->SetStartFacing(facing);}

	int ParaSeqCtrler::GetPlayMode(){return m_pSequenceCtrler->GetPlayMode();}
	void ParaSeqCtrler::SetPlayMode(int mode){m_pSequenceCtrler->SetPlayMode(mode);};

	float ParaSeqCtrler::GetMovingTimeout(){return m_pSequenceCtrler->GetMovingTimeout();}
	void ParaSeqCtrler::SetMovingTimeout(float fTimeout){m_pSequenceCtrler->SetMovingTimeout(fTimeout);}

	void ParaSeqCtrler::Suspend(){m_pSequenceCtrler->Suspend();};
	void ParaSeqCtrler::Resume(){m_pSequenceCtrler->Resume();};

	void ParaSeqCtrler::RunTo(float x,float y,float z){m_pSequenceCtrler->RunTo(x,y,z);};
	void ParaSeqCtrler::WalkTo(float x,float y,float z){m_pSequenceCtrler->WalkTo(x,y,z);};
	void ParaSeqCtrler::MoveTo(float x,float y,float z){m_pSequenceCtrler->MoveTo(x,y,z);};
	void ParaSeqCtrler::MoveAndTurn(float x,float y,float z,float facing){m_pSequenceCtrler->MoveAndTurn(x,y,z,facing);};
	void ParaSeqCtrler::PlayAnim(const char* sAnim){m_pSequenceCtrler->PlayAnim(sAnim);};
	void ParaSeqCtrler::PlayAnim1(int nAnimID){m_pSequenceCtrler->PlayAnim(nAnimID);};
	void ParaSeqCtrler::Wait(float fSeconds){m_pSequenceCtrler->Wait(fSeconds);};
	void ParaSeqCtrler::Exec(const char* sCmd){m_pSequenceCtrler->Exec(sCmd);};
	void ParaSeqCtrler::Pause(){m_pSequenceCtrler->Pause();};
	void ParaSeqCtrler::Turn(float fAngleAbsolute){m_pSequenceCtrler->Turn(fAngleAbsolute);};
	void ParaSeqCtrler::MoveForward(float fDistance){m_pSequenceCtrler->MoveForward(fDistance);};
	void ParaSeqCtrler::MoveBack(float fDistance){m_pSequenceCtrler->MoveBack(fDistance);};
	void ParaSeqCtrler::MoveLeft(float fDistance){m_pSequenceCtrler->MoveLeft(fDistance);};
	void ParaSeqCtrler::MoveRight(float fDistance){m_pSequenceCtrler->MoveRight(fDistance);};
	void ParaSeqCtrler::Jump(){m_pSequenceCtrler->Jump();};
	void ParaSeqCtrler::Goto(int nOffset){m_pSequenceCtrler->Goto(nOffset);};
	void ParaSeqCtrler::Goto1(const char* sLable){m_pSequenceCtrler->Goto(sLable);};
	void ParaSeqCtrler::Lable(const char* sLable){m_pSequenceCtrler->Lable(sLable);}

	//////////////////////////////////////////////////////////////////////////
	//
	// ParaFaceTrackingCtrler
	//
	//////////////////////////////////////////////////////////////////////////
	void ParaFaceTrackingCtrler::FaceTarget( float x, float y, float z, float fDuration )
	{
		if(m_pFaceTrackingCtrler)
		{
			Vector3 v(x, y, z);
			m_pFaceTrackingCtrler->FaceTarget(&v, fDuration);
		}
	}

}//namespace ParaScripting
