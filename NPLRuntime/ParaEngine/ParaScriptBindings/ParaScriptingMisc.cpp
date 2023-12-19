//-----------------------------------------------------------------------------
// Class:	ParaMisc
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2005.11
// Desc: Cross platformed 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "ParaEngineInfo.h"
#include "IParaEngineApp.h"
#include "ParaEngineSettings.h"
#include "ParaWorldAsset.h"
#include "ParaScriptingCommon.h"
#include "ParaScriptingMisc.h"
#include "ParaXSerializer.h"
#include "ParaWorldAsset.h"
#include "ViewportManager.h"

#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif

#include "NPLHelper.h"
#include "util/StringHelper.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

using namespace luabind;

#ifdef PARAENGINE_CLIENT
#include "DirectXEngine.h"
#include "EffectManager.h"
#endif

namespace ParaScripting
{
	string ParaMisc::md5(const std::string& source)
	{
		return ParaEngine::StringHelper::md5(source);
	}

	std::string ParaMisc::md5_(const std::string& source, bool bBinary)
	{
		return ParaEngine::StringHelper::md5(source, bBinary);
	}

	std::string ParaMisc::sha1(const std::string& source)
	{
		return ParaEngine::StringHelper::sha1(source);
	}

	std::string ParaMisc::sha1_(const std::string& source, bool bBinary)
	{
		return ParaEngine::StringHelper::sha1(source, bBinary);
	}


	std::string ParaMisc::base64(const std::string& source)
	{
		return ParaEngine::StringHelper::base64(source);
	}

	std::string ParaMisc::unbase64(const string& source)
	{
		return ParaEngine::StringHelper::unbase64(source);
	}

	int ParaMisc::GetUnicodeCharNum(const char* str)
	{
		return ParaEngine::StringHelper::GetUnicodeCharNum(str);
	}

	std::string ParaMisc::UniSubString( const char* szText, int nFrom, int nTo )
	{
		return ParaEngine::StringHelper::UniSubString(szText, nFrom, nTo);
	}

	std::string ParaMisc::UTF8ToUTF16(const std::string& utf8)
	{
		std::u16string outUtf16;
		ParaEngine::StringHelper::UTF8ToUTF16_Safe(utf8, outUtf16);
		return std::string((const char*)outUtf16.c_str(), outUtf16.size() * 2);
	}

	std::string ParaMisc::UTF16ToUTF8(const std::string& utf16_)
	{
		std::string utf8;
		std::u16string utf16((const char16_t*)utf16_.c_str(), utf16_.size()/2);
		bool ret = ParaEngine::StringHelper::UTF16ToUTF8(utf16, utf8);
		return utf8;
	}

	std::string ParaMisc::SimpleEncode(const std::string& source)
	{
		return ParaEngine::StringHelper::SimpleEncode(source);
	}

	std::string ParaMisc::SimpleDecode( const std::string& source )
	{
		return ParaEngine::StringHelper::SimpleDecode(source);
	}


	const std::string& ParaMisc::EncodingConvert(const std::string& srcEncoding, const std::string& dstEncoding, const std::string& bytes)
	{
		return ParaEngine::StringHelper::EncodingConvert(srcEncoding, dstEncoding, bytes);
	}

	bool ParaMisc::CopyTextToClipboard( const char* text )
	{
		if(text!=NULL)
			return ParaEngine::StringHelper::CopyTextToClipboard(text);
		else
			return false;
	}

	const char* ParaMisc::GetTextFromClipboard()
	{
		return ParaEngine::StringHelper::GetTextFromClipboard();
	}

	long ParaMisc::RandomLong( const object& seedTable )
	{
		if( type(seedTable) == LUA_TTABLE) 
		{
			int32_t _seed = object_cast<int32_t>(seedTable["_seed"]);
			_seed ^= (_seed << 21);
			_seed ^= ((int32_t)(((uint32_t)_seed) >> 5));
			_seed ^= (_seed << 4);
			seedTable["_seed"] = _seed;
			return _seed;
		}
		else
			return rand();
	}

	double ParaMisc::RandomDouble( const object& seedTable )
	{
		if( type(seedTable) == LUA_TTABLE) 
		{
			int32_t _seed = object_cast<int32_t>(seedTable["_seed"]);
			_seed ^= (_seed << 21);
			_seed ^= ((int32_t)(((uint32_t)_seed) >> 5));
			_seed ^= (_seed << 4);
			seedTable["_seed"] = _seed;
			return (_seed&0x7fffffff) / (double)0x7fffffff;
		}
		else
			return (double)rand() / (double)RAND_MAX;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// ParaEngine
	//
	//////////////////////////////////////////////////////////////////////////

	ParaAttributeObject CParaEngine::GetAttributeObject()
	{
		return ParaAttributeObject(&CGlobals::GetSettings());
	}

	void CParaEngine::GetAttributeObject_(ParaAttributeObject& output)
	{
		output = GetAttributeObject();
	}

	string CParaEngine::GetVersion()
	{
		return ParaEngineInfo::CParaEngineInfo::GetVersion();
	}
	string CParaEngine::GetCopyright()
	{
		return ParaEngineInfo::CParaEngineInfo::GetCopyright();
	}
	string CParaEngine::GetAuthorTo()
	{
		return ParaEngineInfo::CParaEngineInfo::GetAuthorizedTo();
	}
	string CParaEngine::GetWatermark()
	{
		return ParaEngineInfo::CParaEngineInfo::GetWaterMarkText();
	}
	bool CParaEngine::ForceRender()
	{
		return ParaEngine::CGlobals::GetApp()->ForceRender();
	}
	bool CParaEngine::Sleep(float fSeconds)
	{
		SLEEP(static_cast<DWORD>(fSeconds * 1000));
		return true;
	}

	bool CParaEngine::SaveParaXMesh(const char* filename, ParaAssetObject& xmesh, bool bBinaryEncoding)
	{
#ifdef USE_DIRECTX_RENDERER
		if( !xmesh.IsValid() || (xmesh.GetType() != AssetEntity::parax) || (filename==0))
			return false;
		CParaXSerializer serializer;
		string sFileName = filename;
		if(sFileName.empty())
		{
			sFileName = xmesh.GetKeyName();
			sFileName.append(".x");
		}
		if(serializer.SaveParaXMesh(sFileName, * (((ParaXEntity*)(xmesh.m_pAsset))->GetModel()), &SerializerOptions(bBinaryEncoding)) == true)
		{
			OUTPUT_LOG("%s is exported successfully from %s\r\n", sFileName.c_str(), xmesh.GetKeyName().c_str());
			return true;
		}
#endif
		return false;
	}

	ParaAssetObject CParaEngine::GetRenderTarget()
	{
#ifdef USE_DIRECTX_RENDERER
		static TextureEntityDirectX g_rt;
		static ParaAssetObject g_rt_wrapper(&g_rt);
		g_rt.SurfaceType = TextureEntity::TextureSurface;
		g_rt.SetSurface( CGlobals::GetDirectXEngine().GetRenderTarget() );
		return g_rt_wrapper;
#else
		return ParaAssetObject();
#endif
	}

	bool CParaEngine::SetRenderTarget( ParaAssetObject& pSrcRenderTarget )
	{
		if(pSrcRenderTarget.IsValid())
		{
			return ((TextureEntity*)pSrcRenderTarget.m_pAsset)->SetRenderTarget();
		}
		return false;
	}

	bool CParaEngine::SetRenderTarget2(int nIndex, const char* render_targetname)
	{
		if (!render_targetname || render_targetname[0] == '\0')
		{
#ifdef USE_DIRECTX_RENDERER
			GETD3D(CGlobals::GetRenderDevice())->SetRenderTarget(nIndex, NULL);
			if (nIndex == 0){
				CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
			}
#endif
			return true;
		}
		else
		{
			TextureEntity* pTexture = ParaEngine::CGlobals::GetAssetManager()->GetTextureManager().GetByName(render_targetname);
			if (pTexture)
			{
				pTexture->SetRenderTarget(nIndex);
				return true;
			}
		}
		return false;
	}
	bool CParaEngine::StretchRect( ParaAssetObject& pSrcRenderTarget, ParaAssetObject& pDestRenderTarget )
	{
		if(pSrcRenderTarget.IsValid() && pDestRenderTarget.IsValid() )
		{
#ifdef USE_DIRECTX_RENDERER
			TextureEntityDirectX * pSrcTexture = (TextureEntityDirectX*)(pSrcRenderTarget.m_pAsset);
			TextureEntityDirectX * pDestTexture = (TextureEntityDirectX*)(pDestRenderTarget.m_pAsset);
			return TextureEntityDirectX::StretchRect(pSrcTexture, pDestTexture);
#endif
		}
		return false;
	}

	bool CParaEngine::SetVertexDeclaration( int nIndex )
	{
		return SUCCEEDED(CGlobals::GetEffectManager()->SetVertexDeclaration(nIndex));
	}

	// draw in the current viewport
	bool CParaEngine::DrawQuad()
	{
		return CGlobals::GetViewportManager()->GetActiveViewPort()->DrawQuad();
	}

	bool CParaEngine::DrawQuad2()
	{
		return CGlobals::GetViewportManager()->GetActiveViewPort()->DrawQuad2();
	}

	ParaScripting::ParaAttributeObject CParaEngine::GetViewportAttributeObject(int nViewportIndex)
	{
		CViewport* pViewport = CGlobals::GetViewportManager()->CreateGetViewPort(nViewportIndex);
		return ParaAttributeObject(pViewport);
	}


}