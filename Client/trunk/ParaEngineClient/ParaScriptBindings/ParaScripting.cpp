//-----------------------------------------------------------------------------
// Class:	CNPLScriptingState and ParaScripting
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised 2: Andy Wang 2007.8
//		Note: move class LoadHAPI_NPL, LoadHAPI_ResourceManager and LoadHAPI_SceneManager to ParaScripting3.cpp
//			  and remove unnecessary header files
// Desc: partially cross platformed
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLRuntime.h"

#include "ParaScriptingScene.h"
#include "ParaScriptingCharacter.h"
#include "ParaXModel/ParaXModelExporter.h"
using namespace ParaEngine;

/** 
for luabind, The main drawback of this approach is that the compilation time will increase for the file 
that does the registration, it is therefore recommended that you register everything in the same cpp-file.
*/
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp>
using namespace luabind;

namespace ParaScripting
{
/** split registration to save compiling time. 
void register_part1(class_<X>& x){    
	x.def(...);
}
void register_part2(class_<X>& x){    
	x.def(...);
}
void register_(lua_State* L){    
	class_<X> x("x");    
	register_part1(x);    
	register_part2(x);    
	module(L) [ x ];
}
*/

/** this function does nothing but count as one instruction in preemptive function. */
int NPL_dummy(lua_State *L){
	return 0;
}

void CNPLScriptingState::LoadParaScene()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	lua_register(L, "dummy", NPL_dummy);

	module(L)
	[
		namespace_("ParaScene")
		[
			// ParaObject class declarations
			class_<ParaObject>("ParaObject")
			//class_<ParaObject>("ParaObject")
				.def(constructor<>())
				.property("name",&ParaObject::GetName,&ParaObject::SetName)
				.property("onclick",&ParaObject::GetOnClick,&ParaObject::SetOnClick)
				.property("onentersentientarea",&ParaObject::GetOnEnterSentientArea,&ParaObject::SetOnEnterSentientArea)
				.property("onleavesentientarea",&ParaObject::GetOnLeaveSentientArea,&ParaObject::SetOnLeaveSentientArea)
				.property("onperceived",&ParaObject::GetOnPerceived,&ParaObject::SetOnPerceived)
				.property("onframemove",&ParaObject::GetOnFrameMove,&ParaObject::SetOnFrameMove)
				.property("id", &ParaObject::GetID)
				.def("IsValid", &ParaObject::IsValid)
				.def("GetID", &ParaObject::GetID)
				.def("IsAttached", &ParaObject::IsAttached)
				.def("GetPrimaryAsset", &ParaObject::GetPrimaryAsset)
				.def("GetEffectParamBlock", &ParaObject::GetEffectParamBlock)
				.def("SetAttribute", &ParaObject::SetAttribute)
				.def("CheckAttribute", &ParaObject::CheckAttribute)
				.def("DistanceTo", &ParaObject::DistanceTo)
				.def("DistanceToSq", &ParaObject::DistanceToSq)
				.def("DistanceToPlayerSq", &ParaObject::DistanceToPlayerSq)
				.def("DistanceToCameraSq", &ParaObject::DistanceToCameraSq)
				.def("On_Click", &ParaObject::On_Click)
				.def("GetViewBox", &ParaObject::GetViewBox)
				.def("GetType", &ParaObject::GetType)
				.def("GetMyType", &ParaObject::GetMyType)
				.def("equals", &ParaObject::equals)
				.def("GetName", &ParaObject::GetName)
				.def("SetName", &ParaObject::SetName)
				.def("IsStanding", &ParaObject::IsStanding)
				.def("SetVisible", &ParaObject::SetVisible)
				.def("IsVisible", &ParaObject::IsVisible)
				.def("SetPosition", &ParaObject::SetPosition)
				.def("GetPosition", &ParaObject::GetPosition, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
				.def("GetViewCenter", &ParaObject::GetViewCenter, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
				.def("OffsetPosition", &ParaObject::OffsetPosition)
				.def("Rotate", &ParaObject::Rotate)
				.def("SetScaling", &ParaObject::SetScaling)
				.def("SetScale", &ParaObject::SetScale)
				.def("GetScale", &ParaObject::GetScale)
				.def("SetRotation", &ParaObject::SetRotation)
				.def("GetRotation", &ParaObject::GetRotation)
				.def("SetFacing", &ParaObject::SetFacing)
				.def("GetFacing", &ParaObject::GetFacing)
				.def("SetPhysicsGroup", &ParaObject::SetPhysicsGroup)
				.def("GetPhysicsGroup", &ParaObject::GetPhysicsGroup)
				.def("SetSelectGroupIndex", &ParaObject::SetSelectGroupIndex)
				.def("GetSelectGroupIndex", &ParaObject::GetSelectGroupIndex)
				.def("SetDensity", &ParaObject::SetDensity)
				.def("GetDensity", &ParaObject::GetDensity)
				.def("SetPhysicsRadius", &ParaObject::SetPhysicsRadius)
				.def("GetPhysicsRadius", &ParaObject::GetPhysicsRadius)
				.def("SetPhysicsHeight", &ParaObject::SetPhysicsHeight)
				.def("GetPhysicsHeight", &ParaObject::GetPhysicsHeight)
				.def("SnapToTerrainSurface", &ParaObject::SnapToTerrainSurface)
				.def("IsOPC", &ParaObject::IsOPC)
				.def("IsCharacter", &ParaObject::IsCharacter)
				.def("ToCharacter", &ParaObject::ToCharacter)
				.def("ToString", &ParaObject::ToString)
				.def("ToString", &ParaObject::ToString1)
				.def("EnablePhysics", &ParaObject::EnablePhysics)
				.def("IsPhysicsEnabled", &ParaObject::IsPhysicsEnabled)
				.def("AddEvent", &ParaObject::AddEvent)
				.def("IsSentient", &ParaObject::IsSentient)
				.def("GetSentientRadius", &ParaObject::GetSentientRadius)
				.def("GetPerceptiveRadius", &ParaObject::GetPerceptiveRadius)
				.def("SetPerceptiveRadius", &ParaObject::SetPerceptiveRadius)
				.def("GetNumOfPerceivedObject", &ParaObject::GetNumOfPerceivedObject)
				.def("IsAlwaysSentient", &ParaObject::IsAlwaysSentient)
				.def("GetPerceivedObject", &ParaObject::GetPerceivedObject)
				.def("SetAlwaysSentient", &ParaObject::SetAlwaysSentient)
				.def("MakeSentient", &ParaObject::MakeSentient)
				.def("UpdateTileContainer", &ParaObject::UpdateTileContainer)
				.def("MakeGlobal", &ParaObject::MakeGlobal)
				.def("IsGlobal", &ParaObject::IsGlobal)
				.def("SetGroupID", &ParaObject::SetGroupID)
				.def("SetSentientField", &ParaObject::SetSentientField)
				.def("IsSentientWith", &ParaObject::IsSentientWith)
				.def("IsPersistent", &ParaObject::IsPersistent)
				.def("SetPersistent", &ParaObject::SetPersistent)
				.def("GetMovableRegion", &ParaObject::GetMovableRegion, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4)+ pure_out_value(_5) + pure_out_value(_6) + pure_out_value(_7))
				.def("SetMovableRegion", &ParaObject::SetMovableRegion)
				.def("SaveToDB", &ParaObject::SaveToDB)
				.def("GetEffectHandle", &ParaObject::GetEffectHandle)
				.def("SetEffectHandle", &ParaObject::SetEffectHandle)
				.def("Reset", &ParaObject::Reset)
				.def("GetAttributeObject", &ParaObject::GetAttributeObject)
				.def("CheckLoadPhysics", &ParaObject::CheckLoadPhysics)
				.def("LoadPhysics", &ParaObject::LoadPhysics)
				.def("GetNumReplaceableTextures", &ParaObject::GetNumReplaceableTextures)
				.def("GetDefaultReplaceableTexture", &ParaObject::GetDefaultReplaceableTexture)
				.def("GetReplaceableTexture", &ParaObject::GetReplaceableTexture)
				.def("SetReplaceableTexture", &ParaObject::SetReplaceableTexture)
				.def("SetHomeZone", &ParaObject::SetHomeZone)
				.def("GetHomeZone", &ParaObject::GetHomeZone)
				.def("AddReference", &ParaObject::AddReference)
				.def("DeleteReference", &ParaObject::DeleteReference)
				.def("DeleteAllRefs", &ParaObject::DeleteAllRefs)
				.def("GetRefObjNum", &ParaObject::GetRefObjNum)
				.def("GetRefObject", &ParaObject::GetRefObject)
				.def("GetXRefScriptCount", &ParaObject::GetXRefScriptCount)
				.def("GetXRefScript", &ParaObject::GetXRefScript)
				.def("GetXRefScriptPosition", &ParaObject::GetXRefScriptPosition, pure_out_value(_3) + pure_out_value(_4) + pure_out_value(_5))
				.def("GetXRefScriptScaling", &ParaObject::GetXRefScriptScaling, pure_out_value(_3) + pure_out_value(_4) + pure_out_value(_5))
				.def("GetXRefScriptFacing", &ParaObject::GetXRefScriptFacing)
				.def("GetXRefScriptLocalMatrix", &ParaObject::GetXRefScriptLocalMatrix)
				.def("SetHeadOnText", &ParaObject::SetHeadOnText)
				.def("GetHeadOnText", &ParaObject::GetHeadOnText)
				.def("SetHeadOnUITemplateName", &ParaObject::SetHeadOnUITemplateName)
				.def("GetHeadOnUITemplateName", &ParaObject::GetHeadOnUITemplateName)
				.def("SetHeadOnTextColor", &ParaObject::SetHeadOnTextColor)
				.def("SetHeadOnOffset", &ParaObject::SetHeadOnOffest)
				.def("SetHeadOnOffest", &ParaObject::SetHeadOnOffest)
				.def("GetHeadOnOffset", &ParaObject::GetHeadOnOffset, pure_out_value(_3) + pure_out_value(_4) + pure_out_value(_5))
				.def("ShowHeadOnDisplay", &ParaObject::ShowHeadOnDisplay)
				.def("IsHeadOnDisplayShown", &ParaObject::IsHeadOnDisplayShown)
				.def("HasHeadOnDisplay", &ParaObject::HasHeadOnDisplay)
				.def("GetAttachmentPosition", &ParaObject::GetAttachmentPosition, pure_out_value(_3) + pure_out_value(_4) + pure_out_value(_5))
				.def("HasAttachmentPoint", &ParaObject::HasAttachmentPoint)
				.def("SetAnimation", &ParaObject::SetAnimation)
				.def("GetAnimation", &ParaObject::GetAnimation)
				.def("GetField", &ParaObject::GetField)
				.def("SetField", &ParaObject::SetField)
				.def("CallField", &ParaObject::CallField)
				.def("GetDynamicField", &ParaObject::GetDynamicField)
				.def("SetDynamicField", &ParaObject::SetDynamicField)
				.def("GetObject", &ParaObject::GetObject)
				.def("GetTexture", &ParaObject::GetTexture)
				.def("AddChild", &ParaObject::AddChild),

				// ParaCharacter class declarations
				class_<ParaCharacter>("ParaCharacter")
				.def(constructor<>())
				.def("FallDown", &ParaCharacter::FallDown)
				.def("SetSpeedScale", &ParaCharacter::SetSpeedScale)
				.def("GetSpeedScale", &ParaCharacter::GetSpeedScale)
				.def("SetSizeScale", &ParaCharacter::SetSizeScale)
				.def("GetSizeScale", &ParaCharacter::GetSizeScale)
				.def("SetFocus", &ParaCharacter::SetFocus)
				.def("ResetBaseModel", &ParaCharacter::ResetBaseModel)
				.def("LoadStoredModel", &ParaCharacter::LoadStoredModel)
				.def("SetSkin", &ParaCharacter::SetSkin)
				.def("GetSkin", &ParaCharacter::GetSkin)
				.def("PlayAnimation", &ParaCharacter::PlayAnimation)
				.def("Stop", &ParaCharacter::Stop)
				.def("MoveTo", &ParaCharacter::MoveTo)
				.def("MoveAndTurn", &ParaCharacter::MoveAndTurn)
				.def("SetBodyParams", &ParaCharacter::SetBodyParams)
				.def("GetBodyParams", &ParaCharacter::GetBodyParams)
				.def("SetDisplayOptions", &ParaCharacter::SetDisplayOptions)
				.def("GetDisplayOptions", &ParaCharacter::GetDisplayOptions)
				.def("SetCharacterSlot", &ParaCharacter::SetCharacterSlot)
				.def("GetCharacterSlotItemID", &ParaCharacter::GetCharacterSlotItemID)
				.def("RefreshModel", &ParaCharacter::RefreshModel)
				.def("LoadFromFile", &ParaCharacter::LoadFromFile)
				.def("SaveToFile", &ParaCharacter::SaveToFile)
				.def("UseAIModule", &ParaCharacter::UseAIModule)
				.def("AssignAIController", &ParaCharacter::AssignAIController)
				.def("IsAIControllerEnabled", &ParaCharacter::IsAIControllerEnabled)
				.def("GetMovieController", &ParaCharacter::GetMovieController)
				.def("GetSeqController", &ParaCharacter::GetSeqController)
				.def("GetFaceTrackingController", &ParaCharacter::GetFaceTrackingController)
				.def("CastEffect", &ParaCharacter::CastEffect)
				.def("CastEffect", &ParaCharacter::CastEffect2)
				.def("AddAttachment", &ParaCharacter::AddAttachment)
				.def("AddAttachment", &ParaCharacter::AddAttachment3)
				.def("AddAttachment", &ParaCharacter::AddAttachment4)
				.def("AddAttachment", &ParaCharacter::AddAttachment5)
				.def("GetAttachmentAttObj", &ParaCharacter::GetAttachmentAttObj)
				.def("RemoveAttachment", &ParaCharacter::RemoveAttachment)
				.def("RemoveAttachment", &ParaCharacter::RemoveAttachment2)
				.def("MountOn", &ParaCharacter::MountOn)
				.def("MountOn", &ParaCharacter::MountOn_)
				.def("MountOn", &ParaCharacter::MountOn2)
				.def("HasMountPoint", &ParaCharacter::HasMountPoint)
				.def("UnMount", &ParaCharacter::UnMount)
				.def("IsMounted", &ParaCharacter::IsMounted)
				.def("AddAction", &ParaCharacter::AddAction)
				.def("AddAction", &ParaCharacter::AddAction2)
				.def("WalkingOrRunning", &ParaCharacter::WalkingOrRunning)
				.def("GetCartoonFaceComponent", &ParaCharacter::GetCartoonFaceComponent)
				.def("SetCartoonFaceComponent", &ParaCharacter::SetCartoonFaceComponent)
				.def("GetSkinColorMask", &ParaCharacter::GetSkinColorMask)
				.def("SetSkinColorMask", &ParaCharacter::SetSkinColorMask)
				.def("IsSupportCartoonFace", &ParaCharacter::IsSupportCartoonFace)
				.def("IsCustomModel", &ParaCharacter::IsCustomModel)
				.def("GetGender", &ParaCharacter::GetGender)
				.def("GetRaceID", &ParaCharacter::GetRaceID)
				.def("EnableAnimIDMap", &ParaCharacter::EnableAnimIDMap)
				.def("IsAnimIDMapEnabled", &ParaCharacter::IsAnimIDMapEnabled)
				.def("AddAnimIDMap", &ParaCharacter::AddAnimIDMap)
				.def("ClearAllAnimIDMap", &ParaCharacter::ClearAllAnimIDMap)
				.def("GetAnimID", &ParaCharacter::GetAnimID)
				.def("GetAnimFileName", &ParaCharacter::GetAnimFileName)
				.def("HasAnimation", &ParaCharacter::HasAnimation)
				.def("IsValid", &ParaCharacter::IsValid),

			// ParaMovieCtrler class declarations
			class_<ParaMovieCtrler>("ParaMovieCtrler")
				.def(constructor<>())
				.def("GetTime", &ParaMovieCtrler::GetTime)
				.def("SetTime", &ParaMovieCtrler::SetTime)
				.def("Suspend", &ParaMovieCtrler::Suspend)
				.def("Resume", &ParaMovieCtrler::Resume)
				.def("Play", &ParaMovieCtrler::Play)
				.def("Record", &ParaMovieCtrler::Record)
				.def("RecordNewDialog", &ParaMovieCtrler::RecordNewDialog)
				.def("RecordNewAction", &ParaMovieCtrler::RecordNewAction)
				.def("RecordNewAction", &ParaMovieCtrler::RecordNewAction_)
				.def("GetOffsetPosition", &ParaMovieCtrler::GetOffsetPosition, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
				.def("SetOffsetPosition", &ParaMovieCtrler::SetOffsetPosition)
				.def("SaveMovie", &ParaMovieCtrler::SaveMovie)
				.def("LoadMovie", &ParaMovieCtrler::LoadMovie)
				.def("IsValid", &ParaMovieCtrler::IsValid),

			// ParaSeqCtrler class declarations
			class_<ParaSeqCtrler>("ParaSeqCtrler")
				.def(constructor<>())
				.def("Save", &ParaSeqCtrler::Save)
				.def("LoadByID", &ParaSeqCtrler::LoadByID)
				.def("LoadFromFile", &ParaSeqCtrler::LoadFromFile)
				.def("Create", &ParaSeqCtrler::Create)
				.def("ToString", &ParaSeqCtrler::ToString)
				.def("GetKeyPos", &ParaSeqCtrler::GetKeyPos)
				.def("SetKeyPos", &ParaSeqCtrler::SetKeyPos)
				.def("GetTotalKeys", &ParaSeqCtrler::GetTotalKeys)
				.def("AdvanceKey", &ParaSeqCtrler::AdvanceKey)
				.def("BeginAddKeys", &ParaSeqCtrler::BeginAddKeys)
				.def("EndAddKeys", &ParaSeqCtrler::EndAddKeys)
				.def("GetSequenceID", &ParaSeqCtrler::GetSequenceID)
				.def("DeleteKeysRange", &ParaSeqCtrler::DeleteKeysRange)
				.def("GetPlayDirection", &ParaSeqCtrler::GetPlayDirection)
				.def("SetPlayDirection", &ParaSeqCtrler::SetPlayDirection)
				.def("GetInterval", &ParaSeqCtrler::GetInterval)
				.def("SetInterval", &ParaSeqCtrler::SetInterval)
				.def("GetStartPos", &ParaSeqCtrler::GetStartPos, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
				.def("SetStartPos", &ParaSeqCtrler::SetStartPos)
				.def("GetStartFacing", &ParaSeqCtrler::GetStartFacing)
				.def("SetStartFacing", &ParaSeqCtrler::SetStartFacing)
				.def("GetPlayMode", &ParaSeqCtrler::GetPlayMode)
				.def("SetPlayMode", &ParaSeqCtrler::SetPlayMode)
				.def("GetMovingTimeout", &ParaSeqCtrler::GetMovingTimeout)
				.def("SetMovingTimeout", &ParaSeqCtrler::SetMovingTimeout)
				.def("Suspend", &ParaSeqCtrler::Suspend)
				.def("Resume", &ParaSeqCtrler::Resume)

				.def("RunTo", &ParaSeqCtrler::RunTo)
				.def("WalkTo", &ParaSeqCtrler::WalkTo)
				.def("MoveTo", &ParaSeqCtrler::MoveTo)
				.def("MoveAndTurn", &ParaSeqCtrler::MoveAndTurn)
				.def("PlayAnim", &ParaSeqCtrler::PlayAnim)
				.def("PlayAnim", &ParaSeqCtrler::PlayAnim1)
				.def("Wait", &ParaSeqCtrler::Wait)
				.def("Exec", &ParaSeqCtrler::Exec)
				.def("Pause", &ParaSeqCtrler::Pause)
				.def("Turn", &ParaSeqCtrler::Turn)
				.def("MoveForward", &ParaSeqCtrler::MoveForward)
				.def("MoveBack", &ParaSeqCtrler::MoveBack)
				.def("MoveLeft", &ParaSeqCtrler::MoveLeft)
				.def("MoveRight", &ParaSeqCtrler::MoveRight)
				.def("Jump", &ParaSeqCtrler::Jump)
				.def("Goto", &ParaSeqCtrler::Goto)
				.def("Goto", &ParaSeqCtrler::Goto1)
				.def("Lable", &ParaSeqCtrler::Lable)

				.def("IsValid", &ParaSeqCtrler::IsValid),
		
			// ParaFaceTrackingCtrler class declarations
			class_<ParaFaceTrackingCtrler>("ParaFaceTrackingCtrler")
				.def(constructor<>())
				.def("FaceTarget", &ParaFaceTrackingCtrler::FaceTarget)
				.def("IsValid", &ParaFaceTrackingCtrler::IsValid),

			// ParaCharacter class declarations
			class_<ParaMiniSceneGraph>("ParaMiniSceneGraph")
				.def(constructor<>())
				.def("GetName", &ParaMiniSceneGraph::GetName)
				.def("SetName", &ParaMiniSceneGraph::SetName)
				.def("SetVisible", &ParaMiniSceneGraph::SetVisible)
				.def("IsVisible", &ParaMiniSceneGraph::IsVisible)
				.def("EnableCamera", &ParaMiniSceneGraph::EnableCamera)
				.def("IsCameraEnabled", &ParaMiniSceneGraph::IsCameraEnabled)
				.def("GetAttributeObjectCamera", &ParaMiniSceneGraph::GetAttributeObjectCamera)
				//.def("UpdateCameraParam", &ParaMiniSceneGraph::UpdateCameraParam)
				//.def("GetCameraParam", &ParaMiniSceneGraph::GetCameraParam, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4)+ pure_out_value(_5)+ pure_out_value(_6))
				.def("GetObject", &ParaMiniSceneGraph::GetObject)
				.def("GetObject", &ParaMiniSceneGraph::GetObject3)
				.def("GetObject", &ParaMiniSceneGraph::GetObject4)
				.def("DestroyObject", &ParaMiniSceneGraph::DestroyObject)
				.def("DestroyObject", &ParaMiniSceneGraph::DestroyObject_)
				.def("RemoveObject", &ParaMiniSceneGraph::RemoveObject)
				.def("RemoveObject", &ParaMiniSceneGraph::RemoveObject_)
				.def("AddChild", &ParaMiniSceneGraph::AddChild)
				.def("Reset", &ParaMiniSceneGraph::Reset)
				.def("SetActor", &ParaMiniSceneGraph::SetActor)
				.def("GetActor", &ParaMiniSceneGraph::GetActor)
				.def("GetTexture", &ParaMiniSceneGraph::GetTexture)
				.def("CameraZoom", &ParaMiniSceneGraph::CameraZoom)
				.def("CameraZoomSphere", &ParaMiniSceneGraph::CameraZoomSphere)
				.def("CameraRotate", &ParaMiniSceneGraph::CameraRotate)
				.def("CameraPan", &ParaMiniSceneGraph::CameraPan)
				.def("CameraSetLookAtPos", &ParaMiniSceneGraph::CameraSetLookAtPos)
				.def("CameraGetLookAtPos", &ParaMiniSceneGraph::CameraGetLookAtPos, pure_out_value(_2) + pure_out_value(_3)+ pure_out_value(_4))
				.def("CameraSetLootAtPos", &ParaMiniSceneGraph::CameraSetLookAtPos) // backward compatible
				.def("CameraGetLootAtPos", &ParaMiniSceneGraph::CameraGetLookAtPos, pure_out_value(_2) + pure_out_value(_3)+ pure_out_value(_4)) // backward compatible
				.def("CameraSetEyePosByAngle", &ParaMiniSceneGraph::CameraSetEyePosByAngle)
				.def("CameraGetEyePosByAngle", &ParaMiniSceneGraph::CameraGetEyePosByAngle, pure_out_value(_2) + pure_out_value(_3)+ pure_out_value(_4))
				.def("CameraSetEyePos", &ParaMiniSceneGraph::CameraSetEyePos)
				.def("CameraGetEyePos", &ParaMiniSceneGraph::CameraGetEyePos, pure_out_value(_2) + pure_out_value(_3)+ pure_out_value(_4))
				.def("Draw", &ParaMiniSceneGraph::Draw)
				.def("SaveToFile", &ParaMiniSceneGraph::SaveToFile)
				.def("SaveToFileEx", &ParaMiniSceneGraph::SaveToFileEx)
				.def("SetRenderTargetSize", &ParaMiniSceneGraph::SetRenderTargetSize)
				.def("SetMaskTexture", &ParaMiniSceneGraph::SetMaskTexture)
				.def("EnableActiveRendering", &ParaMiniSceneGraph::EnableActiveRendering)
				.def("IsActiveRenderingEnabled", &ParaMiniSceneGraph::IsActiveRenderingEnabled)
				.def("MousePick", &ParaMiniSceneGraph::MousePick)
				.def("ShowHeadOnDisplay", &ParaMiniSceneGraph::ShowHeadOnDisplay)
				.def("IsHeadOnDisplayShown", &ParaMiniSceneGraph::IsHeadOnDisplayShown)
				.def("GetAttributeObject", &ParaMiniSceneGraph::GetAttributeObject)
				.def("GetAttributeObject", &ParaMiniSceneGraph::GetAttributeObject1)
				.def("CreateSkyBox", &ParaMiniSceneGraph::CreateSkyBox)
				.def("CreateSkyBox", &ParaMiniSceneGraph::CreateSkyBox_)
				.def("DeleteSkyBox", &ParaMiniSceneGraph::DeleteSkyBox)
				.def("GetTimeOfDaySTD", &ParaMiniSceneGraph::GetTimeOfDaySTD)
				.def("SetTimeOfDaySTD", &ParaMiniSceneGraph::SetTimeOfDaySTD)
				.def("SetBackGroundColor", &ParaMiniSceneGraph::SetBackGroundColor)
				.def("DestroyChildren", &ParaMiniSceneGraph::DestroyChildren)
				.def("IsValid", &ParaMiniSceneGraph::IsValid),

			// function declarations
			def("GetActor",&ParaScene::GetCurrentActor),
			def("SetActor",&ParaScene::SetCurrentActor),
			def("GetObject", &ParaScene::GetObject),
			def("GetObject", &ParaScene::GetObject3),
			def("GetObject", &ParaScene::GetObject4),
			def("GetObject", &ParaScene::GetObject5),
			def("GetObjectByViewBox", &ParaScene::GetObjectByViewBox),
			def("GetActionMeshesBySphere", &ParaScene::GetActionMeshesBySphere),
			def("GetPlayer", &ParaScene::GetPlayer),
			def("GetNextObject", &ParaScene::GetNextObject),
			def("CreateWorld", & ParaScene::CreateWorld),
			def("Reset", & ParaScene::Reset),
			def("CreateGlobalTerrain", & ParaScene::CreateGlobalTerrain),
			def("CreateManagedLoader", &ParaScene::CreateManagedLoader),
			def("Attach", & ParaScene::Attach),
			def("Delete", & ParaScene::Delete),
			def("Detach", & ParaScene::Detach),
			def("FireMissile", & ParaScene::FireMissile),
			def("FireMissile", & ParaScene::FireMissile1),
			def("FireMissile", & ParaScene::FireMissile2),
			def("CreateMeshObject", (ParaObject(*)(const char*, const char* , float, float , float ,float , bool ,const char*)) & ParaScene::CreateMeshObject),
			def("CreateMeshObject", (ParaObject(*)(const char*, ParaAssetObject& , float, float , float ,float , bool ,const char*)) & ParaScene::CreateMeshObject),
			def("CreateObject", ParaScene::CreateObject),
			def("CreateSkyBox", & ParaScene::CreateSkyBox),
			def("CreateSkyBox", & ParaScene::CreateSkyBox_),
			def("DeleteSkyBox", & ParaScene::DeleteSkyBox),
			def("CreateVoxelMesh", & ParaScene::CreateVoxelMesh),
			def("CreateZone", & ParaScene::CreateZone),
			def("CreatePortal", & ParaScene::CreatePortal),
			def("CreateMeshPhysicsObject", (ParaObject(*)(const char*, const char* , float , float , float , bool ,const char* localMatrix)) & ParaScene::CreateMeshPhysicsObject),
			def("CreateMeshPhysicsObject", (ParaObject(*)(const char*, ParaAssetObject& , float , float , float , bool ,const char* localMatrix)) & ParaScene::CreateMeshPhysicsObject),
			def("CreateDynamicPhysicsObject", & ParaScene::CreateDynamicPhysicsObject),
			def("CreateCharacter", (ParaObject(*)(const char*, ParaAssetObject&, const char*, bool, float, float, float))& ParaScene::CreateCharacter),
			def("CreateCharacter", (ParaObject(*)(const char*, const char*, const char*, bool, float, float, float))& ParaScene::CreateCharacter),
			def("CreateLightObject", & ParaScene::CreateLightObject),
			def("Play3DSound", & ParaScene::Play3DSound),
			def("OnTerrainChanged", & ParaScene::OnTerrainChanged),
			def("SetFog", & ParaScene::SetFog),
			def("SetGlobalWater", & ParaScene::SetGlobalWater),
			def("GetGlobalWaterLevel", & ParaScene::GetGlobalWaterLevel),
			def("IsGlobalWaterEnabled", & ParaScene::IsGlobalWaterEnabled),
			def("UpdateOcean", & ParaScene::UpdateOcean),
			def("AddWaterRipple", & ParaScene::AddWaterRipple),
			def("SetShadowMethod", & ParaScene::SetShadowMethod),
			def("GetShadowMethod", & ParaScene::GetShadowMethod),
			def("MousePick", & ParaScene::MousePick),
			def("GetObjectsBySphere", & ParaScene::GetObjectsBySphere),
			def("GetObjectsByScreenRect", & ParaScene::GetObjectsByScreenRect),
			def("SelectObject", & ParaScene::SelectObject),
			def("SelectObject", & ParaScene::SelectObject1),
			def("EnableMouseClick", & ParaScene::EnableMouseClick),
			def("RegisterEvent", & ParaScene::RegisterEvent),
			def("RegisterEvent", & ParaScene::RegisterEvent1),
			def("UnregisterEvent", & ParaScene::UnregisterEvent),
			def("UnregisterAllEvent", & ParaScene::UnregisterAllEvent),
			def("EnableLighting", & ParaScene::EnableLighting),
			def("IsLightingEnabled", & ParaScene::IsLightingEnabled),
			def("SetTimeOfDay", & ParaScene::SetTimeOfDay),
			def("GetTimeOfDay", & ParaScene::GetTimeOfDay),
			def("SetTimeOfDaySTD", & ParaScene::SetTimeOfDaySTD),
			def("GetTimeOfDaySTD", & ParaScene::GetTimeOfDaySTD),
			def("SetMaximumAngle", & ParaScene::SetMaximumAngle),
			def("AdvanceTimeOfDay", & ParaScene::AdvanceTimeOfDay),
			def("SetDayLength", & ParaScene::SetDayLength),
			def("GetDayLength", & ParaScene::GetDayLength),
			def("SetModified", & ParaScene::SetModified),
			def("IsModified", & ParaScene::IsModified),
			def("IsScenePaused", & ParaScene::IsScenePaused),
			def("PauseScene", & ParaScene::PauseScene),
			def("IsSceneEnabled", & ParaScene::IsSceneEnabled),
			def("EnableScene", & ParaScene::EnableScene),
			def("TogglePlayer", & ParaScene::TogglePlayer),
			def("GetAttributeObject", & ParaScene::GetAttributeObject),
			def("GetAttributeObject", &ParaScene::GetAttributeObject1),
			def("GetAttributeObjectSky", & ParaScene::GetAttributeObjectSky),
			def("GetAttributeObjectSunLight", &ParaScene::GetAttributeObjectSunLight),
			def("GetAttributeObjectPlayer", & ParaScene::GetAttributeObjectPlayer),
			def("GetAttributeObjectOcean", & ParaScene::GetAttributeObjectOcean),
			def("SaveAllCharacters", & ParaScene::SaveAllCharacters),
			def("GetMiniSceneGraph", & ParaScene::GetMiniSceneGraph),
			def("DeleteMiniSceneGraph", & ParaScene::DeleteMiniSceneGraph),
			def("EnableMiniSceneGraph", & ParaScene::EnableMiniSceneGraph),
			def("IsMiniSceneGraphEnabled", & ParaScene::IsMiniSceneGraphEnabled),
			def("GetPostProcessingScene", & ParaScene::GetPostProcessingScene),
			def("EnablePostProcessing", & ParaScene::EnablePostProcessing),
			def("IsPostProcessingEnabled", & ParaScene::IsPostProcessingEnabled),
			def("ShowHeadOnDisplay", & ParaScene::ShowHeadOnDisplay),
			def("IsHeadOnDisplayShown", & ParaScene::IsHeadOnDisplayShown),
			def("GetScreenPosFrom3DPoint", & ParaScene::GetScreenPosFrom3DPoint),
			def("LoadNPCsByRegion", & ParaScene::LoadNPCsByRegion),
			def("SetCharacterRegionPath", & ParaScene::SetCharacterRegionPath),
			def("SetCharRegionCoordinates", & ParaScene::SetCharRegionCoordinates),
			def("SetCharTextureSize", & ParaScene::SetCharTextureSize),
			def("SetMaxRenderCount", & ParaScene::SetMaxRenderCount),
			def("GetMaxRenderCount", & ParaScene::GetMaxRenderCount),
			def("CheckExist", &ParaScene::CheckExist),
			def("Execute", &ParaScene::Execute),
			def("BmaxExportToSTL", &ParaXModelExporter::BmaxExportToSTL)
		]
	];
}

void CNPLScriptingState::LoadHAPI_AI()
{
	
}
}//namespace ParaScripting