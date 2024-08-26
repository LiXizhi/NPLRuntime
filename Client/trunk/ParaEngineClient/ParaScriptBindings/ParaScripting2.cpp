//-----------------------------------------------------------------------------
// Class:	CNPLScriptingState and ParaScripting
// Authors:	
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2006.5, 2007.3
// The original parascripting.cpp is too large to be compiled by vc compiler (internal compiler error). 
// So it is split it into two files: one is ParaScripting.h, the other is ParaScripting2.cpp.
// ParaScripting2.cpp is mainly for GUI HAPI definition.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
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
#include "ParaScriptingMisc.h"
#include "ParaScriptingNPL.h"
#include "ParaScriptingIC.h"
#include "ParaScriptingNetwork.h"
#include "ParaScripting.h"
#include "ParaScriptingGUI.h"	
#include "ParaScriptingAudio.h"

#include "ParaScriptingScene.h"
#include "Framework/Common/PlatformBridge/PlatformBridge.h"

void CNPLScriptingState::LoadHAPI_Audio()
{
	using namespace luabind;
	lua_State* L = GetLuaState();
	module(L)
		[
			namespace_("ParaAudio")
				[
					class_<ParaAudioSource>("ParaAudioSource")
						.property("MinDistance", &ParaAudioSource::getMinDistance, &ParaAudioSource::setMinDistance)
						.property("MaxDistance", &ParaAudioSource::getMaxDistance, &ParaAudioSource::setMaxDistance)
						.property("RolloffFactor", &ParaAudioSource::getRolloffFactor, &ParaAudioSource::setRolloffFactor)
						.property("Strength", &ParaAudioSource::getStrength, &ParaAudioSource::setStrength)
						.property("Volume", &ParaAudioSource::getVolume, &ParaAudioSource::setVolume)
						.property("Pitch", &ParaAudioSource::getPitch, &ParaAudioSource::setPitch)
						.property("MinVolume", &ParaAudioSource::getMinVolume, &ParaAudioSource::setMinVolume)
						.property("MaxVolume", &ParaAudioSource::getMaxVolume, &ParaAudioSource::setMaxVolume)
						.property("InnerConeAngle", &ParaAudioSource::getInnerConeAngle, &ParaAudioSource::setInnerConeAngle)
						.property("OuterConeAngle", &ParaAudioSource::getOuterConeAngle, &ParaAudioSource::setOuterConeAngle)
						.property("OuterConeVolume", &ParaAudioSource::getOuterConeVolume, &ParaAudioSource::setOuterConeVolume)
						.def_readonly("TotalAudioTime", &ParaAudioSource::getTotalAudioTime)
						.def_readonly("TotalAudioSize", &ParaAudioSource::getTotalAudioSize)
						.def_readonly("CurrentAudioTime", &ParaAudioSource::getCurrentAudioTime)
						.def_readonly("CurrentAudioPosition", &ParaAudioSource::getCurrentAudioPosition)
						.def_readonly("name", &ParaAudioSource::GetName)
						.def(constructor<>())
						.def("play", &ParaAudioSource::play)
						.def("play2d", &ParaAudioSource::play2d)
						.def("play3d", &ParaAudioSource::play3d)
						.def("pause", &ParaAudioSource::pause)
						.def("stop", &ParaAudioSource::stop)
						.def("release", &ParaAudioSource::release)
						.def("loop", &ParaAudioSource::loop)
						.def("move", &ParaAudioSource::move)
						.def("seek", &ParaAudioSource::seek)
						.def("isPlaying", &ParaAudioSource::isPlaying)
						.def("isStopped", &ParaAudioSource::isStopped)
						.def("isLooping", &ParaAudioSource::isLooping)
						.def("isPaused", &ParaAudioSource::isPaused)
						.def("isValid", &ParaAudioSource::isValid)
						.def("setPosition", &ParaAudioSource::setPosition)
						.def("setVelocity", &ParaAudioSource::setVelocity)
						.def("setDirection", &ParaAudioSource::setDirection)
						.def("getPosition", &ParaAudioSource::getPosition, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
						.def("getVelocity", &ParaAudioSource::getVelocity, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
						.def("getDirection", &ParaAudioSource::getDirection, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
						.def("IsValid", &ParaAudioSource::IsValid),

						def("Create", &ParaAudio::Create),
						def("Get", &ParaAudio::Get),
						def("CreateGet", &ParaAudio::CreateGet),
						def("SetVolume", &ParaAudio::SetVolume),
						def("GetVolume", &ParaAudio::GetVolume),
						def("SetDistanceModel", &ParaAudio::SetDistanceModel),
						def("PlayMidiMsg", &ParaAudio::PlayMidiMsg),
						def("StopMidiMsg", &ParaAudio::StopMidiMsg),
						def("PlayWaveFile", &ParaAudio::PlayWaveFile),
						def("PlayWaveFile", &ParaAudio::PlayWaveFile1),
						def("StopWaveFile", &ParaAudio::StopWaveFile),
						def("StopWaveFile", &ParaAudio::StopWaveFile1),
						def("ReleaseWaveFile", &ParaAudio::ReleaseWaveFile),
						def("StartRecording", &ParaAudio::StartRecording),
						def("StopRecording", &ParaAudio::StopRecording),
						def("GetCapturedAudio", &ParaAudio::GetCapturedAudio),
						def("SaveRecording", &ParaAudio::SaveRecording),
						def("SaveRecording", &ParaAudio::SaveRecording2)
				]
		];
}

void CNPLScriptingState::LoadHAPI_Network()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	module(L)
		[
			namespace_("ParaNetwork")
				[
					def("IsNetworkLayerRunning", &ParaNetwork::IsNetworkLayerRunning),
						def("SetNerveCenterAddress", &ParaNetwork::SetNerveCenterAddress),
						def("SetNerveReceptorAddress", &ParaNetwork::SetNerveReceptorAddress),
						def("EnableNetwork", &ParaNetwork::EnableNetwork)
				]

		];
}

void CNPLScriptingState::LoadHAPI_UI()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	module(L)
		[
			namespace_("ParaMisc")
				[
					def("UniSubString", &ParaMisc::UniSubString),
						def("GetUnicodeCharNum", &ParaMisc::GetUnicodeCharNum),
						def("EncodingConvert", &ParaMisc::EncodingConvert),
						def("UTF16ToUTF8", &ParaMisc::UTF16ToUTF8),
						def("UTF8ToUTF16", &ParaMisc::UTF8ToUTF16),
						def("SimpleEncode", &ParaMisc::SimpleEncode),
						def("SimpleDecode", &ParaMisc::SimpleDecode),
						def("GetTextFromClipboard", &ParaMisc::GetTextFromClipboard),
						def("CopyTextToClipboard", &ParaMisc::CopyTextToClipboard),
						def("RandomDouble", &ParaMisc::RandomDouble),
						def("RandomLong", &ParaMisc::RandomLong),
						def("md5", &ParaMisc::md5_),
						def("md5", &ParaMisc::md5),
						def("sha1", &ParaMisc::sha1),
						def("sha1", &ParaMisc::sha1_),
						def("base64", &ParaMisc::base64),
						def("unbase64", &ParaMisc::unbase64),
						def("LuaCallNative", &ParaEngine::PlatformBridge::LuaCallNative)
				]
		];

	module(L)
		[
			namespace_("Config")
				[
					// declarations
					def("AppendIntValue", &ParaConfig::AppendIntValue),
						def("AppendDoubleValue", &ParaConfig::AppendDoubleValue),
						def("AppendTextValue", &ParaConfig::AppendTextValue),
						def("SetIntValue", &ParaConfig::SetIntValue),
						def("SetDoubleValue", &ParaConfig::SetDoubleValue),
						def("SetTextValue", &ParaConfig::SetTextValue),
						def("GetIntValue", &ParaConfig::GetIntValue, pure_out_value(_2)),
						def("GetDoubleValue", &ParaConfig::GetDoubleValue, pure_out_value(_2)),
						def("SetIntValueEx", &ParaConfig::SetIntValueEx),
						def("SetDoubleValueEx", &ParaConfig::SetDoubleValueEx),
						def("SetTextValueEx", &ParaConfig::SetTextValueEx),
						def("GetIntValueEx", &ParaConfig::GetIntValueEx, pure_out_value(_2)),
						def("GetDoubleValueEx", &ParaConfig::GetDoubleValueEx, pure_out_value(_2)),
						def("GetTextValue", &ParaConfig::GetTextValue),
						def("GetTextValueEx", &ParaConfig::GetTextValueEx)
				]
		];
	module(L)
		[
			namespace_("ParaPainter")
				[
					// declarations
					def("GetAttributeObject", &ParaPainter::GetAttributeObject),
						def("SetField", &ParaPainter::SetField),
						def("GetField", &ParaPainter::GetField),
						def("CallField", &ParaPainter::CallField),
						// states
						def("Begin", &ParaPainter::Begin),
						def("Flush", &ParaPainter::Flush),
						def("Save", &ParaPainter::Save),
						def("Restore", &ParaPainter::Restore),
						def("SetCompositionMode", &ParaPainter::SetCompositionMode),
						def("GetCompositionMode", &ParaPainter::GetCompositionMode),
						def("SetFont", &ParaPainter::SetFont),
						def("SetPen", &ParaPainter::SetPen),
						def("SetBrush", &ParaPainter::SetBrush),
						def("SetBrushOrigin", &ParaPainter::SetBrushOrigin),
						def("SetBackground", &ParaPainter::SetBackground),
						def("SetOpacity", &ParaPainter::SetOpacity),
						def("SetClipRegion", &ParaPainter::SetClipRegion),
						def("SetClipping", &ParaPainter::SetClipping),
						def("HasClipping", &ParaPainter::HasClipping),
						// transforms
						def("SetTransform", &ParaPainter::SetTransform),
						def("GetTransform", &ParaPainter::GetTransform),
						def("Scale", &ParaPainter::Scale),
						def("Shear", &ParaPainter::Shear),
						def("Rotate", &ParaPainter::Rotate),
						def("Translate", &ParaPainter::Translate),
						// draws
						def("DrawPoint", &ParaPainter::DrawPoint),
						def("DrawLine", &ParaPainter::DrawLine),
						def("DrawRect", &ParaPainter::DrawRect),
						def("DrawTriangleList", &ParaPainter::DrawTriangleList),
						def("DrawLineList", &ParaPainter::DrawLineList),
						def("DrawTexture", &ParaPainter::DrawTexture),
						def("DrawTexture", &ParaPainter::DrawTexture2),
						def("DrawText", &ParaPainter::DrawText),
						def("DrawText", &ParaPainter::DrawText2),
						def("DrawSceneObject", &ParaPainter::DrawSceneObject),
						def("End", &ParaPainter::End)
				]
		];
	module(L)
		[
			namespace_("ParaUI")
				[
#ifdef PARAENGINE_CLIENT
#ifdef USE_FLASH_MANAGER
					class_<ParaFlashPlayer>("ParaFlashPlayer")
						.def(constructor<>())
						.def("GetFileName", &ParaFlashPlayer::GetFileName)
						.def("GetIndex", &ParaFlashPlayer::GetIndex)
						.def("IsFree", &ParaFlashPlayer::IsFree)
						.def("LoadMovie", &ParaFlashPlayer::LoadMovie)
						.def("CallFlashFunction", &ParaFlashPlayer::CallFlashFunction)
						.def("SetFlashReturnValue", &ParaFlashPlayer::SetFlashReturnValue)
						.def("PutFlashVars", &ParaFlashPlayer::PutFlashVars)
						.def("UnloadMovie", &ParaFlashPlayer::UnloadMovie)
						.def("InvalidateDeviceObjects", &ParaFlashPlayer::InvalidateDeviceObjects)
						.def("GetTextureInfo", &ParaFlashPlayer::GetTextureInfo, pure_out_value(_4) + pure_out_value(_5))
						.def("IsWindowMode", &ParaFlashPlayer::IsWindowMode)
						.def("SetWindowMode", &ParaFlashPlayer::SetWindowMode)
						.def("MoveWindow", &ParaFlashPlayer::MoveWindow)
						.def("SendMessage", &ParaFlashPlayer::SendMessage)

						.def("IsValid", &ParaFlashPlayer::IsValid),
#endif
#endif
						class_<ParaUIFont>("ParaUIFont")
						.property("transparency", &ParaUIFont::GetTransparency, &ParaUIFont::SetTransparency)
						.property("color", &ParaUIFont::GetColor, &ParaUIFont::SetColor)
						.property("font", &ParaUIFont::GetFont, &ParaUIFont::SetFont)
						.property("format", &ParaUIFont::GetFormat, &ParaUIFont::SetFormat)
						.def("IsValid", &ParaUIFont::IsValid),
						class_<ParaUITexture>("ParaUITexture")
						.property("transparency", &ParaUITexture::GetTransparency, &ParaUITexture::SetTransparency)
						.property("color", &ParaUITexture::GetColor, &ParaUITexture::SetColor)
						.property("texture", &ParaUITexture::GetTexture, &ParaUITexture::SetTexture)
						.property("rect", &ParaUITexture::GetTextureRect, &ParaUITexture::SetTextureRect)
						.def("IsValid", &ParaUITexture::IsValid),
						// ParaUIObject class declarations
						class_<ParaUIObject>("ParaUIObject")
						.def(constructor<>())
						.property("text", &ParaUIObject::GetText, &ParaUIObject::SetText1)
						.property("id", &ParaUIObject::GetID, &ParaUIObject::SetID)
						.property("PasswordChar", &ParaUIObject::GetPasswordChar, &ParaUIObject::SetPasswordChar)
						.property("name", &ParaUIObject::GetName, &ParaUIObject::SetName)
						.property("enabled", &ParaUIObject::GetEnabled, &ParaUIObject::SetEnabled)
						.property("highlightstyle", &ParaUIObject::GetHighlightStyle, &ParaUIObject::SetHighlightStyle)
						.property("autosize", &ParaUIObject::GetAutoSize, &ParaUIObject::SetAutoSize)
						.property("visible", &ParaUIObject::GetVisible, &ParaUIObject::SetVisible)
						.property("candrag", &ParaUIObject::GetCanDrag, &ParaUIObject::SetCanDrag)
						.property("scrollable", &ParaUIObject::GetScrollable, &ParaUIObject::SetScrollable)
						.property("readonly", &ParaUIObject::GetReadOnly, &ParaUIObject::SetReadOnly)
						.property("position", &ParaUIObject::GetPosition, &ParaUIObject::SetPosition)
						.property("parent", &ParaUIObject::GetParent, &ParaUIObject::SetParent)
						.property("background", &ParaUIObject::GetBGImage, &ParaUIObject::SetBGImage1)
						.property("color", &ParaUIObject::GetColor, &ParaUIObject::SetColor)
						.property("button", &ParaUIObject::GetBtnImage, &ParaUIObject::SetBtnImage1)
						.property("font", &ParaUIObject::GetFontString, &ParaUIObject::SetFontString1)
						.property("type", &ParaUIObject::GetType)
						.property("shadow", &ParaUIObject::GetUseTextShadow, &ParaUIObject::SetUseTextShadow)
						.property("textscale", &ParaUIObject::GetTextScale, &ParaUIObject::SetTextScale)
						.property("script", &ParaUIObject::ToScript)
						.property("ismodified", &ParaUIObject::IsModified)
						.property("animstyle", &ParaUIObject::GetAnimationStyle, &ParaUIObject::SetAnimationStyle)
						.property("receivedrag", &ParaUIObject::GetReceiveDrag, &ParaUIObject::SetReceiveDrag)
						.property("wordbreak", &ParaUIObject::GetWordbreak, &ParaUIObject::SetWordbreak)
						.property("itemheight", &ParaUIObject::GetItemHeight, &ParaUIObject::SetItemHeight)
						.property("multiselect", &ParaUIObject::GetMultipleSelect, &ParaUIObject::SetMultipleSelect)
						.property("tooltip", &ParaUIObject::GetToolTip, &ParaUIObject::SetToolTip)
						.property("scrollbarwidth", &ParaUIObject::GetScrollbarWidth, &ParaUIObject::SetScrollbarWidth)
						.property("fastrender", &ParaUIObject::GetFastRender, &ParaUIObject::SetFastRender)
						.property("lifetime", &ParaUIObject::GetLifeTime, &ParaUIObject::SetLifeTime)
						.property("zdepth", &ParaUIObject::GetZDepth, &ParaUIObject::SetZDepth)
						.property("value", &ParaUIObject::GetValue, &ParaUIObject::SetValue)
						.property("fixedthumb", &ParaUIObject::GetFixedThumb, &ParaUIObject::SetFixedThumb)
						.property("thumbsize", &ParaUIObject::GetThumbSize, &ParaUIObject::SetThumbSize)
						.property("tooltip", &ParaUIObject::GetToolTip, &ParaUIObject::SetToolTip)
						.property("zorder", &ParaUIObject::GetZOrder, &ParaUIObject::SetZOrder)
						.property("cursor", &ParaUIObject::GetCursor, &ParaUIObject::SetCursor)
						.property("x", &ParaUIObject::GetX, &ParaUIObject::SetX)
						.property("y", &ParaUIObject::GetY, &ParaUIObject::SetY)
						.property("depth", &ParaUIObject::GetDepth, &ParaUIObject::SetDepth)
						.property("width", &ParaUIObject::Width, &ParaUIObject::SetWidth)
						.property("height", &ParaUIObject::Height, &ParaUIObject::SetHeight)
						.property("rotation", &ParaUIObject::GetRotation, &ParaUIObject::SetRotation)
						.property("scalingx", &ParaUIObject::GetScalingX, &ParaUIObject::SetScalingX)
						.property("scalingy", &ParaUIObject::GetScalingY, &ParaUIObject::SetScalingY)
						.property("translationx", &ParaUIObject::GetTranslationX, &ParaUIObject::SetTranslationX)
						.property("translationy", &ParaUIObject::GetTranslationY, &ParaUIObject::SetTranslationY)
						.property("colormask", &ParaUIObject::GetColorMask, &ParaUIObject::SetColorMask)
						.property("spacing", &ParaUIObject::GetSpacing, &ParaUIObject::SetSpacing)
						.property("popup", &ParaUIObject::GetPopUp, &ParaUIObject::SetPopUp)
						.property("onframemove", &ParaUIObject::GetOnFrameMove, &ParaUIObject::OnFrameMove)
						.property("ondraw", &ParaUIObject::GetOnDraw, &ParaUIObject::OnDraw)
						.property("oninputmethod", &ParaUIObject::GetOnInputMethod, &ParaUIObject::OnInputMethod)
						.property("onclick", &ParaUIObject::GetOnClick, &ParaUIObject::OnClick)
						.property("onchange", &ParaUIObject::GetOnChange, &ParaUIObject::OnChange)
						.property("onkeydown", &ParaUIObject::GetOnKeyDown, &ParaUIObject::OnKeyDown)
						.property("onkeyup", &ParaUIObject::GetOnKeyUp, &ParaUIObject::OnKeyUp)
						.property("ondoubleclick", &ParaUIObject::GetOnDoubleClick, &ParaUIObject::OnDoubleClick)
						.property("ondragbegin", &ParaUIObject::GetOnDragBegin, &ParaUIObject::OnDragBegin)
						.property("ondragend", &ParaUIObject::GetOnDragEnd, &ParaUIObject::OnDragEnd)
						.property("ondragmove", &ParaUIObject::GetOnDragOver, &ParaUIObject::OnDragOver)
						.property("onmousedown", &ParaUIObject::GetOnMouseDown, &ParaUIObject::OnMouseDown)
						.property("onmouseup", &ParaUIObject::GetOnMouseUp, &ParaUIObject::OnMouseUp)
						.property("onmousemove", &ParaUIObject::GetOnMouseMove, &ParaUIObject::OnMouseMove)
						.property("onmousewheel", &ParaUIObject::GetOnMouseWheel, &ParaUIObject::OnMouseWheel)
						.property("onmousehover", &ParaUIObject::GetOnMouseHover, &ParaUIObject::OnMouseHover)
						.property("onmouseenter", &ParaUIObject::GetOnMouseEnter, &ParaUIObject::OnMouseEnter)
						.property("onmouseleave", &ParaUIObject::GetOnMouseLeave, &ParaUIObject::OnMouseLeave)
						.property("ontouch", &ParaUIObject::GetOnTouch, &ParaUIObject::OnTouch)
						.property("onselect", &ParaUIObject::GetOnSelect, &ParaUIObject::OnSelect)
						.property("onfocusin", &ParaUIObject::GetOnFocusIn, &ParaUIObject::OnFocusIn)
						.property("onfocusout", &ParaUIObject::GetOnFocusOut, &ParaUIObject::OnFocusOut)
						.property("onmodify", &ParaUIObject::GetOnModify, &ParaUIObject::OnModify)
						.property("onactivate", &ParaUIObject::GetOnActivate, &ParaUIObject::OnActivate)
						.property("ondestroy", &ParaUIObject::GetOnDestroy, &ParaUIObject::OnDestroy)
						.property("onsize", &ParaUIObject::GetOnSize, &ParaUIObject::OnSize)
						.def("GetAbsPosition", &ParaUIObject::GetAbsPosition, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4) + pure_out_value(_5) + pure_out_value(_6))
						.def("AddTextItem", &ParaUIObject::AddTextItem)
						.def("SetTopLevel", &ParaUIObject::SetTopLevel)
						.def("BringToFront", &ParaUIObject::BringToFront)
						.def("BringToBack", &ParaUIObject::BringToBack)
						.def("Focus", &ParaUIObject::Focus)
						.def("LostFocus", &ParaUIObject::LostFocus)
						.def("ActivateScript", &ParaUIObject::ActivateScript)
						.def("GetChild", &ParaUIObject::GetChild)
						.def("GetChildAt", &ParaUIObject::GetChildAt)
						.def("GetChildCount", &ParaUIObject::GetChildCount)
						.def("GetChildByID", &ParaUIObject::GetChildByID)
						.def("GetChildByName", &ParaUIObject::GetChildByName)
						.def("GetID", &ParaUIObject::GetID)
						.def("IsValid", &ParaUIObject::IsValid)
						.def("SetDefault", &ParaUIObject::SetDefault)
						.def("SetNineElementBG", &ParaUIObject::SetNineElementBG)
						.def("SetUseTextShadow", &ParaUIObject::SetUseTextShadow)
						.def("GetUseTextShadow", &ParaUIObject::GetUseTextShadow)
						.def("SetBGImage", &ParaUIObject::SetBGImage5)
						.def("SetBGImageAndRect", &ParaUIObject::SetBGImageAndRect)
						.def("SetBGImage", &ParaUIObject::SetBGImage)
						.def("SetField", &ParaUIObject::SetField)
						.def("GetField", &ParaUIObject::GetField)
						.def("CallField", &ParaUIObject::CallField)
						.def("SetText", &ParaUIObject::SetText3)
						.def("SetText", &ParaUIObject::SetText1)
						.def("SetTextAutoTranslate", &ParaUIObject::SetTextAutoTranslate)
						.def("GetText", &ParaUIObject::GetText)
						.def("SetSize", &ParaUIObject::SetSize)
						.def("GetFirstVisibleCharIndex", &ParaUIObject::GetFirstVisibleCharIndex)
						.def("GetCaretPosition", &ParaUIObject::GetCaretPosition)
						.def("SetCaretPosition", &ParaUIObject::SetCaretPosition)
						.def("SetCursor", &ParaUIObject::SetCursorEx)
						.def("GetTextSize", &ParaUIObject::GetTextSize)
						.def("XYtoCP", &ParaUIObject::XYtoCP, pure_out_value(_6) + pure_out_value(_7))
						.def("CPtoXY", &ParaUIObject::CPtoXY, pure_out_value(_6) + pure_out_value(_7))
						.def("GetNextWordPos", &ParaUIObject::GetNextWordPos, pure_out_value(_4))
						.def("GetPriorWordPos", &ParaUIObject::GetPriorWordPos, pure_out_value(_4))
						.def("GetTextLineSize", &ParaUIObject::GetTextLineSize, pure_out_value(_2) + pure_out_value(_3))
						.def("RemoveAll", &ParaUIObject::RemoveAll)
						.def("RemoveItem", &ParaUIObject::RemoveItem)
						.def("SetBindingObj", &ParaUIObject::SetBindingObj)
						.def("GetRotOriginOffset", &ParaUIObject::GetRotOriginOffset, pure_out_value(_2) + pure_out_value(_3))
						.def("SetRotOriginOffset", &ParaUIObject::SetRotOriginOffset)
						.def("SetPosition", &ParaUIObject::SetPosition)
						.def("Reposition", &ParaUIObject::Reposition)
						.def("SetTrackRange", &ParaUIObject::SetTrackRange)
						.def("GetTrackRange", &ParaUIObject::GetTrackRange, pure_out_value(_2) + pure_out_value(_3))
						.def("SetPageSize", &ParaUIObject::SetPageSize)
						.def("SetStep", &ParaUIObject::SetStep)
						.def("GetStep", &ParaUIObject::GetStep)
						.def("InvalidateRect", &ParaUIObject::InvalidateRect)
						.def("UpdateRect", &ParaUIObject::UpdateRect)
						.def("AttachToRoot", &ParaUIObject::AttachToRoot)
						.def("AttachTo3D", &ParaUIObject::AttachTo3D)
						.def("AttachTo3D", &ParaUIObject::AttachTo3D_)
						.def("AddChild", &ParaUIObject::AddChild)
						.def("CreateFont", &ParaUIObject::CreateFont)
						.def("CreateTexture", &ParaUIObject::CreateTexture)
						.def("GetFont", &ParaUIObject::GetFont)
						.def("GetFont", &ParaUIObject::GetFont_)
						.def("GetTexture", &ParaUIObject::GetTexture)
						.def("DoAutoSize", &ParaUIObject::DoAutoSize)
						.def("CloneState", &ParaUIObject::CloneState)
						.def("SetCurrentState", &ParaUIObject::SetCurrentState)
						.def("SetActiveLayer", &ParaUIObject::SetActiveLayer)
						.def("HasLayer", &ParaUIObject::HasLayer)
						.def("SetAnimationStyle", &ParaUIObject::SetAnimationStyle)
						.def("GetAnimationStyle", &ParaUIObject::GetAnimationStyle)
						.def("ApplyAnim", &ParaUIObject::ApplyAnim)
						.def("GetAttributeObject", &ParaUIObject::GetAttributeObject),


						// function declarations
						def("PlaySound", &ParaUI::PlaySound),
						def("StopSound", &ParaUI::StopSound),
						def("Destroy", &ParaUI::Destroy),
						def("Destroy", &ParaUI::Destroy1),
						def("DestroyUIObject", &ParaUI::DestroyUIObject),
						def("AddDragReceiver", &ParaUI::AddDragReceiver),
						def("PostDestroy", &ParaUI::PostDestroy),
						def("GetUIObject", &ParaUI::GetUIObject_any),
						def("GetUIObjectAtPoint", &ParaUI::GetUIObjectAtPoint),
						def("Pick", &ParaUI::Pick),
						def("GetTopLevelControl", &ParaUI::GetTopLevelControl),
						def("GetMousePosition", &ParaUI::GetMousePosition, pure_out_value(_1) + pure_out_value(_2)),
						def("SetMousePosition", &ParaUI::SetMousePosition),
						def("IsKeyPressed", &ParaUI::IsKeyPressed),
						def("IsMousePressed", &ParaUI::IsMousePressed),
						def("SetCursorText", &ParaUI::SetCursorText),// obsoleted
						def("SetCursorTexture", &ParaUI::SetCursorTexture),// obsoleted
						def("SetCursorFont", &ParaUI::SetCursorFont),// obsoleted
						def("SetCursorFromFile", &ParaUI::SetCursorFromFile),
						def("SetCursorFromFile", &ParaUI::SetCursorFromFile_),
						def("GetCursorFile", &ParaUI::GetCursorFile),
						def("GetUseSystemCursor", &ParaUI::GetUseSystemCursor),
						def("SetUseSystemCursor", &ParaUI::SetUseSystemCursor),
						def("ToScript", &ParaUI::ToScript),
						def("SaveLayout", &ParaUI::SaveLayout),
						def("CreateUIObject", &ParaUI::CreateUIObject),
						def("GetDefaultObject", &ParaUI::GetDefaultObject),
						def("SetDesignTime", &ParaUI::SetDesignTime),
						def("ShowCursor", &ParaUI::ShowCursor),
						def("ResetUI", &ParaUI::ResetUI),
						def("SetHighlightParam", &ParaUI::SetHighlightParam),
#ifdef USE_DIRECTX_RENDERER
#ifdef USE_FLASH_MANAGER
						def("GetFlashPlayer", &ParaUI::GetFlashPlayer),
						def("GetFlashPlayer", &ParaUI::GetFlashPlayer1),
						def("CreateFlashPlayer", &ParaUI::CreateFlashPlayer),
#endif
#endif
						def("IsMouseLocked", &ParaUI::IsMouseLocked),
						def("GetIMEOpenStatus", &ParaUI::GetIMEOpenStatus),
						def("SetIMEOpenStatus", &ParaUI::SetIMEOpenStatus),
						def("SetUIScale", &ParaUI::SetUIScale),
						def("SetMinimumScreenSize", &ParaUI::SetMinimumScreenSize),
						def("SetMaximumScreenSize", &ParaUI::SetMaximumScreenSize),
						def("LockMouse", &ParaUI::LockMouse)
						//			def("CreateWin", & ParaUI::CreateWin)
				]
		];

	LoadHAPI_UI_Extension();
}

