//-----------------------------------------------------------------------------
// Class:	CNPLScriptingState and ParaScripting
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised 2: Andy Wang 2007.8
//		Note: move in class LoadHAPI_NPL, LoadHAPI_ResourceManager and LoadHAPI_SceneManager from ParaScripting.cpp
// Desc: partially cross platformed
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaScriptingNPL.h"
#include "ParaScriptingCommon.h"
#include "ParaScriptingTerrain.h"
#include "ParaScriptingScene.h"

#ifdef EMSCRIPTEN_SINGLE_THREAD
#define auto_ptr unique_ptr
#endif
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

#include "NPLRuntime.h"
#include "ParaScriptingIPC.h"
#include "ParaScripting.h"

namespace ParaScripting
{

void CNPLScriptingState::LoadHAPI_SceneManager()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	LoadParaScene();

	LoadParaWorld();

	module(L)
		[
			namespace_("ParaTerrain")
			[
				// function declarations
				def("GetElevation", &ParaTerrain::GetElevation),
				def("GetTerrainElevFile", &ParaTerrain::GetTerrainElevFile),
				def("GetTerrainOnloadScript", &ParaTerrain::GetTerrainOnloadScript),
				def("GetRegionValue", &ParaTerrain::GetRegionValue),
				def("GetRegionValue", &ParaTerrain::GetRegionValue4),
				
				def("GetTextureCount", &ParaTerrain::GetTextureCount),
				def("GetTexture", &ParaTerrain::GetTexture),
				def("ReplaceTexture", &ParaTerrain::ReplaceTexture),
				def("ReplaceTexture", &ParaTerrain::ReplaceTexture_),
				def("GetTexturesInCell", &ParaTerrain::GetTexturesInCell),
				def("RemoveTextureInCell", &ParaTerrain::RemoveTextureInCell),
				
				def("SnapPointToVertexGrid", &ParaTerrain::SnapPointToVertexGrid,pure_out_value(_3) + pure_out_value(_4)),
				def("GetVertexSpacing", &ParaTerrain::GetVertexSpacing),

				def("Flatten", &ParaTerrain::Flatten),
				def("DigCircleFlat", &ParaTerrain::DigCircleFlat),
				def("GaussianHill", &ParaTerrain::GaussianHill),
				def("RadialScale", &ParaTerrain::RadialScale),
				def("Roughen_Smooth", &ParaTerrain::Roughen_Smooth),
				def("Spherical", &ParaTerrain::Spherical),
				def("AddHeightField", &ParaTerrain::AddHeightField),
				def("MergeHeightField", &ParaTerrain::MergeHeightField),
				def("Ramp", &ParaTerrain::Ramp),
				
				def("PaintTerrainData",&ParaTerrain::PaintTerrainData),
				def("GetTerrainData",&ParaTerrain::GetTerrainData),
				def("SetVisibleDataMask",&ParaTerrain::SetVisibleDataMask),

				def("IsHole", &ParaTerrain::IsHole),
				def("SetHole", &ParaTerrain::SetHole),
				def("UpdateHoles", &ParaTerrain::UpdateHoles),
				
				def("Paint", &ParaTerrain::Paint),
				def("Paint", &ParaTerrain::Paint_),

				def("UpdateTerrain", &ParaTerrain::UpdateTerrain),
				def("UpdateTerrain", &ParaTerrain::UpdateTerrain1),
				def("ReloadTerrain", &ParaTerrain::ReloadTerrain),
				def("IsModified", &ParaTerrain::IsModified),
				def("SetContentModified", &ParaTerrain::SetContentModified),
				def("SetContentModified", &ParaTerrain::SetContentModified4),
				def("SetAllLoadedModified", &ParaTerrain::SetAllLoadedModified),
				def("SaveTerrain", &ParaTerrain::SaveTerrain),

				def("EnableLighting", &ParaTerrain::EnableLighting),

				def("GetAttributeObject", &ParaTerrain::GetAttributeObject),
				def("GetAttributeObjectAt", &ParaTerrain::GetAttributeObjectAt),
				def("GetBlockAttributeObject", &ParaTerrain::GetBlockAttributeObject),
								
				def("EnterBlockWorld",&ParaTerrain::EnterBlockWorld),
				def("LeaveBlockWorld",&ParaTerrain::LeaveBlockWorld),
				def("IsInBlockWorld",&ParaTerrain::IsInBlockWorld),
				def("SaveBlockWorld",&ParaTerrain::SaveBlockWorld),
				def("DeleteAllBlocks",&ParaTerrain::DeleteAllBlocks),

				def("SetBlockTemplate",&ParaTerrain::SetBlockTemplateId),
				def("SetBlockTemplateByIdx",&ParaTerrain::SetBlockTemplateIdByIdx),

				def("GetBlockTemplate",&ParaTerrain::GetBlockTemplateId),
				def("GetBlockTemplateByIdx",&ParaTerrain::GetBlockTemplateIdByIdx),

				def("SetBlockUserData",&ParaTerrain::SetBlockUserData),
				def("SetBlockUserDataByIdx",&ParaTerrain::SetBlockUserDataByIdx),
		
				def("GetBlockUserData",&ParaTerrain::GetBlockUserData),
				def("GetBlockUserDataByIdx",&ParaTerrain::GetBlockUserDataByIdx),

				def("SetBlockMaterial", &ParaTerrain::SetBlockMaterial),
				def("GetBlockMaterial", &ParaTerrain::GetBlockMaterial),

				def("GetBlocksInRegion",&ParaTerrain::GetBlocksInRegion),
				def("GetActiveRegion",&ParaTerrain::GetVisibleChunkRegion),
				def("LoadBlockAsync", &ParaTerrain::LoadBlockAsync),

				def("Pick",&ParaTerrain::Pick),
				def("MousePick",&ParaTerrain::MousePick),
				def("FindFirstBlock",&ParaTerrain::FindFirstBlock),
				def("GetFirstBlock",&ParaTerrain::GetFirstBlock),
				def("RegisterBlockTemplate",&ParaTerrain::RegisterBlockTemplate),
				def("SetTemplateTexture",&ParaTerrain::SetTemplateTexture),
				def("SetBlockWorldYOffset",&ParaTerrain::SetBlockWorldYOffset),

				def("SelectBlock",&ParaTerrain::SelectBlock),
				def("SelectBlock",&ParaTerrain::SelectBlock1),
				def("DeselectAllBlock",&ParaTerrain::DeselectAllBlock),
				def("DeselectAllBlock",&ParaTerrain::DeselectAllBlock1),
				def("SetDamagedBlock",&ParaTerrain::SetDamagedBlock),
				def("SetDamagedDegree",&ParaTerrain::SetDamageDegree),
				def("GetChunkColumnTimeStamp", &ParaTerrain::GetChunkColumnTimeStamp),
				def("SetChunkColumnTimeStamp", &ParaTerrain::SetChunkColumnTimeStamp),
				def("GetMapChunkData", &ParaTerrain::GetMapChunkData),
				def("ApplyMapChunkData", &ParaTerrain::ApplyMapChunkData),
				def("GetBlockFullData", &ParaTerrain::GetBlockFullData, pure_out_value(_4) + pure_out_value(_5)),
				def("SetBlockWorldSunIntensity",&ParaTerrain::SetBlockWorldSunIntensity)
			]
		];

	module(L)
		[
			namespace_("ParaCamera")
			[
				// function declarations
				def("GetPosition", &ParaCamera::GetPosition, pure_out_value(_1) + pure_out_value(_2) + pure_out_value(_3)),
				def("GetLookAtPosition", &ParaCamera::GetLookAtPosition, pure_out_value(_1) + pure_out_value(_2) + pure_out_value(_3)),
				def("FollowObject", (void(*)(ParaObject))&ParaCamera::FollowObject),
				def("FollowObject", (void(*)(const char*))&ParaCamera::FollowObject),
				def("FirstPerson", &ParaCamera::FirstPerson),
				def("ThirdPerson", &ParaCamera::ThirdPerson),
				def("Default", &ParaCamera::Default),
				def("Fixed", &ParaCamera::Fixed),
				def("GetKeyMap", &ParaCamera::GetKeyMap),
				def("SetKeyMap", &ParaCamera::SetKeyMap),
				def("GetAttributeObject", &ParaCamera::GetAttributeObject)
			]
		];

}


void CNPLScriptingState::LoadHAPI_ResourceManager()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	module(L)
		[
			namespace_("ParaAsset")
			[
				// ParaObject class declarations
				class_<ParaAssetObject>("ParaAssetObject")
				.def(constructor<>())
				.def("Reload", &ParaAssetObject::Reload)
				.def("IsLoaded", &ParaAssetObject::IsLoaded)
				.def("GetKeyName", &ParaAssetObject::GetKeyName)
				.def("GetFileName", &ParaAssetObject::GetFileName)
				.def("GetAttributeObject", &ParaAssetObject::GetAttributeObject)
				.def("IsValid", &ParaAssetObject::IsValid)
				.def("equals", &ParaAssetObject::equals)
				.def("GetHeight", &ParaAssetObject::GetHeight)
				.def("GetWidth", &ParaAssetObject::GetWidth)
				.def("SetSize", &ParaAssetObject::SetSize)
				.def("GetBoundingBox", &ParaAssetObject::GetBoundingBox)
				.def("SetTextureFPS", &ParaAssetObject::SetTextureFPS)
				.def("EnableTextureAutoAnimation", &ParaAssetObject::EnableTextureAutoAnimation)
				.def("SetCurrentFrameNumber", &ParaAssetObject::SetCurrentFrameNumber)
				.def("GetCurrentFrameNumber", &ParaAssetObject::GetCurrentFrameNumber)
				.def("GetFrameCount", &ParaAssetObject::GetFrameCount)
				.def("UnloadAsset", &ParaAssetObject::UnloadAsset)
				.def("LoadAsset", &ParaAssetObject::LoadAsset)
				.def("Refresh", &ParaAssetObject::Refresh)
				.def("Refresh", &ParaAssetObject::Refresh1)
				.def("Release", &ParaAssetObject::Release)
				.def("GetRefCount", &ParaAssetObject::GetRefCount)
				.def("GarbageCollectMe", &ParaAssetObject::GarbageCollectMe)
				.def("SetHandle", &ParaAssetObject::SetHandle)
				.def("GetHandle", &ParaAssetObject::GetHandle)
				.def("GetParamBlock", &ParaAssetObject::GetParamBlock)
				.def("GetNumReplaceableTextures", &ParaAssetObject::GetNumReplaceableTextures)
				.def("GetDefaultReplaceableTexture", &ParaAssetObject::GetDefaultReplaceableTexture)
				.def("Begin", &ParaAssetObject::Begin)
				.def("BeginPass", &ParaAssetObject::BeginPass)
				.def("SetTexture", &ParaAssetObject::SetTexture)
				.def("EndPass", &ParaAssetObject::EndPass)
				.def("End", &ParaAssetObject::End)
				.def("CommitChanges", &ParaAssetObject::CommitChanges)
				.def("SetCallback", &ParaAssetObject::SetCallback)
				.def("GetType", &ParaAssetObject::GetType),
				// parameter block object declarations
				class_<ParaParamBlock>("ParaParamBlock")
				.def(constructor<>())
				.def("IsValid", &ParaParamBlock::IsValid)
				.def("SetInt", &ParaParamBlock::SetInt)
				.def("SetBoolean", &ParaParamBlock::SetBoolean)
				.def("SetFloat", &ParaParamBlock::SetFloat)
				.def("SetVector2", &ParaParamBlock::SetVector2)
				.def("SetVector3", &ParaParamBlock::SetVector3)
				.def("SetVector4", &ParaParamBlock::SetVector4)
				.def("SetMatrix43", &ParaParamBlock::SetMatrix43)
				.def("SetParam", &ParaParamBlock::SetParam)
				.def("SetTexture", &ParaParamBlock::SetTexture)
				.def("SetTexture", &ParaParamBlock::SetTexture_)
				.def("SetTextureObj", &ParaParamBlock::SetTextureObj)
				.def("Clear", &ParaParamBlock::Clear),

				// function declarations
				def("OpenArchive", &ParaAsset::OpenArchive),
				def("OpenArchive", &ParaAsset::OpenArchive2),
				def("OpenArchive", &ParaAsset::OpenArchiveEx),
				def("CloseArchive", &ParaAsset::CloseArchive),
				def("GeneratePkgFile", &ParaAsset::GeneratePkgFile),
				def("GeneratePkgFile", &ParaAsset::GeneratePkgFile_),
				def("Init", &ParaAsset::Init),
				def("Unload", &ParaAsset::Unload),
				def("UnloadAll", &ParaAsset::UnloadAll),
				def("UnloadDatabase", &ParaAsset::UnloadDatabase),
				def("GarbageCollect", &ParaAsset::GarbageCollect),
				def("LoadEffectFile", &ParaAsset::LoadEffectFile),
				def("GetEffectFile", &ParaAsset::GetEffectFile),
				def("GetBlockMaterial", &ParaAsset::GetBlockMaterial),
				def("CreateBlockMaterial", &ParaAsset::CreateBlockMaterial),
				def("CreateGetBlockMaterial", &ParaAsset::CreateGetBlockMaterial),
				def("DeleteBlockMaterial", &ParaAsset::DeleteBlockMaterial),
				def("LoadParaX", &ParaAsset::LoadParaX),
				def("LoadDatabase", &ParaAsset::LoadDatabase),
				def("LoadStaticMesh", &ParaAsset::LoadStaticMesh),
				def("LoadTexture", &ParaAsset::LoadTexture),
				def("LoadRenderTarget", &ParaAsset::LoadRenderTarget),
				def("LoadPickingBuffer", &ParaAsset::LoadPickingBuffer),
				def("LoadSpriteFrame", &ParaAsset::LoadSpriteFrame),
				def("LoadFont", &ParaAsset::LoadFont),
				def("LoadImageFont", &ParaAsset::LoadImageFont),
				def("GetBoneAnimProviderIDByName", &ParaAsset::GetBoneAnimProviderIDByName),
				def("GetBoneAnimProviderFileNameByID", &ParaAsset::GetBoneAnimProviderFileNameByID),
				def("CreateBoneAnimProvider", &ParaAsset::CreateBoneAnimProvider),
				def("DeleteBoneAnimProvider", &ParaAsset::DeleteBoneAnimProvider),
				def("PrintToFile", &ParaAsset::PrintToFile),
				def("AddFontName", &ParaAsset::AddFontName),
				def("SetAssetServerUrl", & ParaAsset::SetAssetServerUrl),
				def("GetAssetServerUrl", & ParaAsset::GetAssetServerUrl),
				def("Refresh", & ParaAsset::Refresh),
				def("LoadSound", & ParaAsset::LoadSound)
			]
		];

}


int NPL_export_capi(lua_State *L){
	return ParaScripting::CNPL::export_(L);
}

int NPL_filename_capi(lua_State *L){
	const char* filename = ParaScripting::CNPL::GetFileName(L);
	if (filename != 0)
	{
		lua_pushlstring(L, filename, strlen(filename));
		return 1;
	}
	return 0;
}

void CNPLScriptingState::LoadHAPI_NPL()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	module(L)
		[
			//		def("_ALERT", (void (*) (const object& ))&NPL_Alert),

			namespace_("NPL")
			[
				class_<ParaNPLRuntimeState>("ParaNPLRuntimeState")
				.def(constructor<>())
				.def("IsValid", &ParaNPLRuntimeState::IsValid)
				.def("GetName", &ParaNPLRuntimeState::GetName)
				.def("Start", &ParaNPLRuntimeState::Start)
				.def("Stop", &ParaNPLRuntimeState::Stop)
				.def("Reset", &ParaNPLRuntimeState::Reset)
				.def("Reset", &ParaNPLRuntimeState::Reset1)
				.def("GetCurrentQueueSize", &ParaNPLRuntimeState::GetCurrentQueueSize)
				.def("GetProcessedMsgCount", &ParaNPLRuntimeState::GetProcessedMsgCount)
				.def("SetMsgQueueSize", &ParaNPLRuntimeState::SetMsgQueueSize)
				.def("GetMsgQueueSize", &ParaNPLRuntimeState::GetMsgQueueSize)
				.def("WaitForMessage", &ParaNPLRuntimeState::WaitForMessage)
				.def("WaitForMessage", &ParaNPLRuntimeState::WaitForMessage2)
				.def("GetField", &ParaNPLRuntimeState::GetField)
				.def("SetField", &ParaNPLRuntimeState::SetField)
				.def("PeekMessage", &ParaNPLRuntimeState::PeekMessage)
				.def("PopMessageAt", &ParaNPLRuntimeState::PopMessageAt)
				.def("GetStats", &ParaNPLRuntimeState::GetStats),

				// function declarations
				def("activate", &CNPL::activate),
				def("activate", &CNPL::activate3),
				def("activate", &CNPL::activate5),
				def("activate", &CNPL::activate1),
				def("call",&CNPL::call),
				def("ShowWindow", &CNPL::ShowWindow),
				def("load", &CNPL::load1),
				def("load", &CNPL::load),
				def("StartNetServer", &CNPL::StartNetServer),
				def("StopNetServer", &CNPL::StopNetServer),
				def("Ping", &CNPL::Ping),
				def("AddPublicFile", &CNPL::AddPublicFile),
				def("ClearPublicFiles", &CNPL::ClearPublicFiles),
				def("AddNPLRuntimeAddress", &CNPL::AddNPLRuntimeAddress),
				def("GetIP", &CNPL::GetIP),
				def("accept", &CNPL::accept),
				def("SetProtocol", &CNPL::SetProtocol),
				def("reject", &CNPL::reject),
				def("SetUseCompression", &CNPL::SetUseCompression),
				def("SetCompressionKey", &CNPL::SetCompressionKey),
				def("GetAttributeObject", &CNPL::GetAttributeObject),
				
				def("EnableNetWork", &CNPL::EnableNetwork),
				def("AddDNSRecord", &CNPL::AddDNSRecord),
				def("GetExternalIP", &CNPL::GetExternalIP),
				def("SetTimer", &CNPL::SetTimer),
				def("KillTimer",&CNPL::KillTimer),
				def("ChangeTimer",&CNPL::ChangeTimer),
				def("Compile",&CNPL::Compile),
				def("DoString",&CNPL::DoString),
				def("DoString",&CNPL::DoString2),
				def("test",&CNPL::test),
				def("SerializeToSCode",&CNPL::SerializeToSCode),
				def("SerializeToSCode", &CNPL::SerializeToSCode2),
				def("IsSCodePureData",&CNPL::IsSCodePureData),
				def("IsPureData",&CNPL::IsPureData),
				def("IsPureTable",&CNPL::IsPureTable),
				def("LoadTableFromString",&CNPL::LoadTableFromString),
				def("LoadObjectFromString", &CNPL::LoadObjectFromString),
				def("GetSourceName",&CNPL::GetSourceName),
				def("SetSourceName",&CNPL::SetSourceName),
				def("SetDefaultChannel",&CNPL::SetDefaultChannel),
				def("GetDefaultChannel",&CNPL::GetDefaultChannel),
				def("SetChannelProperty",&CNPL::SetChannelProperty),
				def("ResetChannelProperties",&CNPL::ResetChannelProperties),
				def("GetChannelProperty",&CNPL::GetChannelProperty, pure_out_value(_2) + pure_out_value(_3)),
				def("RegisterWSCallBack",&CNPL::RegisterWSCallBack),
				def("UnregisterWSCallBack",&CNPL::UnregisterWSCallBack),
				def("AsyncDownload",&CNPL::AsyncDownload),
				def("CancelDownload",&CNPL::CancelDownload),
				def("Download",&CNPL::Download),
				def("CreateRuntimeState",&CNPL::CreateRuntimeState),
				def("GetRuntimeState",&CNPL::GetRuntimeState),
				def("DeleteRuntimeState",&CNPL::DeleteRuntimeState),
				def("CreateGetRuntimeState",&CNPL::CreateGetRuntimeState),
				def("AppendURLRequest",&CNPL::AppendURLRequest1),
				def("EncodeURLQuery",&CNPL::EncodeURLQuery),
				def("ChangeRequestPoolSize",&CNPL::ChangeRequestPoolSize),
				def("RegisterEvent",&CNPL::RegisterEvent),
				def("UnregisterEvent",&CNPL::UnregisterEvent),
				def("GetStats", &CNPL::GetStats),
				def("GetLuaState", &CNPL::GetLuaState),

				def("FromJson",&CNPL::FromJson),
				def("ToJson", &CNPL::ToJson),
				def("ToJson", &CNPL::ToJson2),
				def("Compress", &CNPL::Compress),
				def("Decompress", &CNPL::Decompress),
				def("this", &CNPL::this2_),
				def("this", &CNPL::this_)
			]
		];

	{
		// register a number of NPL C API. 
		lua_pushlstring(L, "NPL", 3);
		lua_rawget(L, LUA_GLOBALSINDEX);
		if (lua_istable(L, -1))
		{
			// NPL.export function
			lua_pushlstring(L, "export", 6);
			lua_pushcfunction(L, NPL_export_capi);
			lua_rawset(L, -3);
			// NPL.filename function
			lua_pushlstring(L, "filename", 8);
			lua_pushcfunction(L, NPL_filename_capi);
			lua_rawset(L, -3);

			lua_pop(L, 1);
		}
	}

#ifdef PARAENGINE_CLIENT
	module(L)
		[
			namespace_("ParaIPC")
			[
				class_<ParaIPCQueue>("ParaIPCQueue")
				.def(constructor<>())
				.def(constructor<const char*, int>())
				.def("GetName", &ParaIPCQueue::GetName)
				.def("Cleanup", &ParaIPCQueue::Cleanup)
				.def("Remove", &ParaIPCQueue::Remove)
				.def("Clear", &ParaIPCQueue::Clear)
				.def("send", &ParaIPCQueue::send)
				.def("try_send", &ParaIPCQueue::try_send)
				.def("receive", &ParaIPCQueue::receive)
				.def("try_receive", &ParaIPCQueue::try_receive)
				.def("IsValid", &ParaIPCQueue::IsValid),

				// function declarations
				def("CreateGetQueue", &ParaIPC::CreateGetQueue),
				def("RemoveQueue", &ParaIPC::RemoveQueue),
				def("Clear", &ParaIPC::Clear)
			]
		];
#endif
	LoadHAPI_Jabber();
}

}//namespace ParaScripting
