#include "D3DMapping.h"
using namespace ParaEngine;
D3DRENDERSTATETYPE D3DMapping::toD3DRenderState(const ERenderState& rs)
{
	switch (rs)
	{
	case ERenderState::ZENABLE:
		return D3DRS_ZENABLE;
	case ERenderState::FILLMODE:
		return D3DRS_FILLMODE;
	case ERenderState::SHADEMODE:
		return D3DRS_SHADEMODE;
	case ERenderState::ZWRITEENABLE:
		return D3DRS_ZWRITEENABLE;
	case ERenderState::ALPHATESTENABLE:
		return D3DRS_ALPHATESTENABLE;
	case ERenderState::LASTPIXEL:
		return D3DRS_LASTPIXEL;
	case ERenderState::SRCBLEND:
		return D3DRS_SRCBLEND;
	case ERenderState::DESTBLEND:
		return D3DRS_DESTBLEND;
	case ERenderState::CULLMODE:
		return D3DRS_CULLMODE;
	case ERenderState::ZFUNC:
		return D3DRS_ZFUNC;
	case ERenderState::ALPHAREF:
		return D3DRS_ALPHAREF;
	case ERenderState::ALPHAFUNC:
		return D3DRS_ALPHAFUNC;
	case ERenderState::DITHERENABLE:
		return D3DRS_DITHERENABLE;
	case ERenderState::ALPHABLENDENABLE:
		return D3DRS_ALPHABLENDENABLE;
	case ERenderState::FOGENABLE:
		return D3DRS_FOGENABLE;
	case ERenderState::SPECULARENABLE:
		return D3DRS_SPECULARENABLE;
	case ERenderState::FOGCOLOR:
		return D3DRS_FOGCOLOR;
	case ERenderState::FOGTABLEMODE:
		return D3DRS_FOGTABLEMODE;
	case ERenderState::FOGSTART:
		return D3DRS_FOGSTART;
	case ERenderState::FOGEND:
		return D3DRS_FOGEND;
	case ERenderState::FOGDENSITY:
		return D3DRS_FOGDENSITY;
	case ERenderState::RANGEFOGENABLE:
		return D3DRS_RANGEFOGENABLE;
	case ERenderState::STENCILENABLE:
		return D3DRS_STENCILENABLE;
	case ERenderState::STENCILFAIL:
		return D3DRS_STENCILFAIL;
	case ERenderState::STENCILZFAIL:
		return D3DRS_STENCILZFAIL;
	case ERenderState::STENCILPASS:
		return D3DRS_STENCILPASS;
	case ERenderState::STENCILFUNC:
		return D3DRS_STENCILFUNC;
	case ERenderState::STENCILREF:
		return D3DRS_STENCILREF;
	case ERenderState::STENCILMASK:
		return D3DRS_STENCILMASK;
	case ERenderState::STENCILWRITEMASK:
		return D3DRS_STENCILWRITEMASK;
	case ERenderState::TEXTUREFACTOR:
		return D3DRS_TEXTUREFACTOR;
	case ERenderState::WRAP0:
		return D3DRS_WRAP0;
	case ERenderState::WRAP1:
		return D3DRS_WRAP1;
	case ERenderState::WRAP2:
		return D3DRS_WRAP2;
	case ERenderState::WRAP3:
		return D3DRS_WRAP3;
	case ERenderState::WRAP4:
		return D3DRS_WRAP4;
	case ERenderState::WRAP5:
		return D3DRS_WRAP5;
	case ERenderState::WRAP6:
		return D3DRS_WRAP6;
	case ERenderState::WRAP7:
		return D3DRS_WRAP7;
	case ERenderState::CLIPPING:
		return D3DRS_CLIPPING;
	case ERenderState::LIGHTING:
		return D3DRS_LIGHTING;
	case ERenderState::AMBIENT:
		return D3DRS_AMBIENT;
	case ERenderState::FOGVERTEXMODE:
		return D3DRS_FOGVERTEXMODE;
	case ERenderState::COLORVERTEX:
		return D3DRS_COLORVERTEX;
	case ERenderState::LOCALVIEWER:
		return D3DRS_LOCALVIEWER;
	case ERenderState::NORMALIZENORMALS:
		return D3DRS_NORMALIZENORMALS;
	case ERenderState::DIFFUSEMATERIALSOURCE:
		return D3DRS_DIFFUSEMATERIALSOURCE;
	case ERenderState::SPECULARMATERIALSOURCE:
		return D3DRS_SPECULARMATERIALSOURCE;
	case ERenderState::AMBIENTMATERIALSOURCE:
		return D3DRS_AMBIENTMATERIALSOURCE;
	case ERenderState::EMISSIVEMATERIALSOURCE:
		return D3DRS_EMISSIVEMATERIALSOURCE;
	case ERenderState::VERTEXBLEND:
		return D3DRS_VERTEXBLEND;
	case ERenderState::CLIPPLANEENABLE:
		return D3DRS_CLIPPLANEENABLE;
	case ERenderState::POINTSIZE:
		return D3DRS_POINTSIZE;
	case ERenderState::POINTSIZE_MIN:
		return D3DRS_POINTSIZE_MIN;
	case ERenderState::POINTSPRITEENABLE:
		return D3DRS_POINTSPRITEENABLE;
	case ERenderState::POINTSCALEENABLE:
		return D3DRS_POINTSCALEENABLE;
	case ERenderState::POINTSCALE_A:
		return D3DRS_POINTSCALE_A;
	case ERenderState::POINTSCALE_B:
		return D3DRS_POINTSCALE_B;
	case ERenderState::POINTSCALE_C:
		return D3DRS_POINTSCALE_C;
	case ERenderState::MULTISAMPLEANTIALIAS:
		return D3DRS_MULTISAMPLEANTIALIAS;
	case ERenderState::MULTISAMPLEMASK:
		return D3DRS_MULTISAMPLEMASK;
	case ERenderState::PATCHEDGESTYLE:
		return D3DRS_PATCHEDGESTYLE;
	case ERenderState::DEBUGMONITORTOKEN:
		return D3DRS_DEBUGMONITORTOKEN;
	case ERenderState::POINTSIZE_MAX:
		return D3DRS_POINTSIZE_MAX;
	case ERenderState::INDEXEDVERTEXBLENDENABLE:
		return D3DRS_INDEXEDVERTEXBLENDENABLE;
	case ERenderState::COLORWRITEENABLE:
		return D3DRS_COLORWRITEENABLE;
	case ERenderState::TWEENFACTOR:
		return D3DRS_TWEENFACTOR;
	case ERenderState::BLENDOP:
		return D3DRS_BLENDOP;
	case ERenderState::POSITIONDEGREE:
		return D3DRS_POSITIONDEGREE;
	case ERenderState::NORMALDEGREE:
		return D3DRS_NORMALDEGREE;
	case ERenderState::SCISSORTESTENABLE:
		return D3DRS_SCISSORTESTENABLE;
	case ERenderState::SLOPESCALEDEPTHBIAS:
		return D3DRS_SLOPESCALEDEPTHBIAS;
	case ERenderState::ANTIALIASEDLINEENABLE:
		return D3DRS_ANTIALIASEDLINEENABLE;
	case ERenderState::MINTESSELLATIONLEVEL:
		return D3DRS_MINTESSELLATIONLEVEL;
	case ERenderState::MAXTESSELLATIONLEVEL:
		return D3DRS_MAXTESSELLATIONLEVEL;
	case ERenderState::ADAPTIVETESS_X:
		return D3DRS_ADAPTIVETESS_X;
	case ERenderState::ADAPTIVETESS_Y:
		return D3DRS_ADAPTIVETESS_Y;
	case ERenderState::ADAPTIVETESS_Z:
		return D3DRS_ADAPTIVETESS_Z;
	case ERenderState::ADAPTIVETESS_W:
		return D3DRS_ADAPTIVETESS_W;
	case ERenderState::ENABLEADAPTIVETESSELLATION:
		return D3DRS_ENABLEADAPTIVETESSELLATION;
	case ERenderState::TWOSIDEDSTENCILMODE:
		return D3DRS_TWOSIDEDSTENCILMODE;
	case ERenderState::CCW_STENCILFAIL:
		return D3DRS_CCW_STENCILFAIL;
	case ERenderState::CCW_STENCILZFAIL:
		return D3DRS_CCW_STENCILZFAIL;
	case ERenderState::CCW_STENCILPASS:
		return D3DRS_CCW_STENCILPASS;
	case ERenderState::CCW_STENCILFUNC:
		return D3DRS_CCW_STENCILFUNC;
	case ERenderState::COLORWRITEENABLE1:
		return D3DRS_COLORWRITEENABLE1;
	case ERenderState::COLORWRITEENABLE2:
		return D3DRS_COLORWRITEENABLE2;
	case ERenderState::COLORWRITEENABLE3:
		return D3DRS_COLORWRITEENABLE3;
	case ERenderState::BLENDFACTOR:
		return D3DRS_BLENDFACTOR;
	case ERenderState::SRGBWRITEENABLE:
		return D3DRS_SRGBWRITEENABLE;
	case ERenderState::DEPTHBIAS:
		return D3DRS_DEPTHBIAS;
	case ERenderState::WRAP8:
		return D3DRS_WRAP8;
	case ERenderState::WRAP9:
		return D3DRS_WRAP9;
	case ERenderState::WRAP10:
		return D3DRS_WRAP10;
	case ERenderState::WRAP11:
		return D3DRS_WRAP11;
	case ERenderState::WRAP12:
		return D3DRS_WRAP12;
	case ERenderState::WRAP13:
		return D3DRS_WRAP13;
	case ERenderState::WRAP14:
		return D3DRS_WRAP14;
	case ERenderState::WRAP15:
		return D3DRS_WRAP15;
	case ERenderState::SEPARATEALPHABLENDENABLE:
		return D3DRS_SEPARATEALPHABLENDENABLE;
	case ERenderState::SRCBLENDALPHA:
		return D3DRS_SRCBLENDALPHA;
	case ERenderState::DESTBLENDALPHA:
		return D3DRS_DESTBLENDALPHA;
	case ERenderState::BLENDOPALPHA:
		return D3DRS_BLENDOPALPHA;
	default:
		assert(false);
		return D3DRS_FORCE_DWORD;

	}
}


ERenderState D3DMapping::toRenderState(D3DRENDERSTATETYPE format)
{
	static std::unordered_map<DWORD, ERenderState > enum_map;
	if (enum_map.empty())
	{
		int count = (int)ERenderState::COUNT;
		for (int i = 0; i < count; i++)
		{
			ERenderState pixelFormat = (ERenderState)(i);
			enum_map[toD3DRenderState(pixelFormat)] = pixelFormat;
		}
	}

	auto it = enum_map.find(format);
	if (it != enum_map.end())
	{
		return it->second;
	}
	return ERenderState::Unknown;
}


D3DFORMAT D3DMapping::toD3DFromat(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::Unknown:
		return D3DFMT_UNKNOWN;
	case PixelFormat::R8G8B8:
		return D3DFMT_R8G8B8;
	case PixelFormat::A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case PixelFormat::X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case PixelFormat::R5G6B5:
		return D3DFMT_R5G6B5;
	case PixelFormat::X1R5G5B5:
		return D3DFMT_X1R5G5B5;
	case PixelFormat::A1R5G5B5:
		return D3DFMT_A1R5G5B5;
	case PixelFormat::A4R4G4B4:
		return D3DFMT_A4R4G4B4;
	case PixelFormat::R3G3B2:
		return D3DFMT_R3G3B2;
	case PixelFormat::A8:
		return D3DFMT_A8;
	case PixelFormat::A8R3G3B2:
		return D3DFMT_A8R3G3B2;
	case PixelFormat::X4R4G4B4:
		return D3DFMT_X4R4G4B4;
	case PixelFormat::A2B10G10R10:
		return D3DFMT_A2B10G10R10;
	case PixelFormat::A8B8G8R8:
		return D3DFMT_A8B8G8R8;
	case PixelFormat::X8B8G8R8:
		return D3DFMT_X8B8G8R8;
	case PixelFormat::G16R16:
		return D3DFMT_G16R16;
	case PixelFormat::A2R10G10B10:
		return D3DFMT_A2R10G10B10;
	case PixelFormat::A16B16G16R16:
		return D3DFMT_A16B16G16R16;
	case PixelFormat::A8P8:
		return D3DFMT_A8P8;
	case PixelFormat::P8:
		return D3DFMT_P8;
	case PixelFormat::L8:
		return D3DFMT_L8;
	case PixelFormat::A8L8:
		return D3DFMT_A8L8;
	case PixelFormat::A4L4:
		return D3DFMT_A4L4;
	case PixelFormat::V8U8:
		return D3DFMT_V8U8;
	case PixelFormat::L6V5U5:
		return D3DFMT_L6V5U5;
	case PixelFormat::X8L8V8U8:
		return D3DFMT_X8L8V8U8;
	case PixelFormat::Q8W8V8U8:
		return D3DFMT_Q8W8V8U8;
	case PixelFormat::V16U16:
		return D3DFMT_V16U16;
	case PixelFormat::A2W10V10U10:
		return D3DFMT_A2W10V10U10;
	case PixelFormat::UYVY:
		return D3DFMT_UYVY;
	case PixelFormat::R8G8_B8G8:
		return D3DFMT_R8G8_B8G8;
	case PixelFormat::YUY2:
		return D3DFMT_YUY2;
	case PixelFormat::G8R8_G8B8:
		return D3DFMT_G8R8_G8B8;
	case PixelFormat::DXT1:
		return D3DFMT_DXT1;
	case PixelFormat::DXT2:
		return D3DFMT_DXT2;
	case PixelFormat::DXT3:
		return D3DFMT_DXT3;
	case PixelFormat::DXT4:
		return D3DFMT_DXT4;
	case PixelFormat::DXT5:
		return D3DFMT_DXT5;
	case PixelFormat::D16_LOCKABLE:
		return D3DFMT_D16_LOCKABLE;
	case PixelFormat::D32:
		return D3DFMT_D32;
	case PixelFormat::D15S1:
		return D3DFMT_D15S1;
	case PixelFormat::D24S8:
		return D3DFMT_D24S8;
	case PixelFormat::D24X8:
		return D3DFMT_D24X8;
	case PixelFormat::D24X4S4:
		return D3DFMT_D24X4S4;
	case PixelFormat::D16:
		return D3DFMT_D16;
	case PixelFormat::D32F_LOCKABLE:
		return D3DFMT_D32F_LOCKABLE;
	case PixelFormat::D24FS8:
		return D3DFMT_D24FS8;
	case PixelFormat::INDEX16:
		return D3DFMT_INDEX16;
	case PixelFormat::INDEX32:
		return D3DFMT_INDEX32;
	default:
		return D3DFMT_UNKNOWN;
	}
}




D3DMULTISAMPLE_TYPE D3DMapping::toD3DMSAAType(MultiSampleType msaa)
{
	switch (msaa)
	{
	case MultiSampleType::None:
		return D3DMULTISAMPLE_NONE;
		break;
	case MultiSampleType::NONMASKABLE:
		return D3DMULTISAMPLE_NONMASKABLE;
		break;
	case MultiSampleType::Samples_2:
		return D3DMULTISAMPLE_2_SAMPLES;
		break;
	case MultiSampleType::Samples_3:
		return D3DMULTISAMPLE_3_SAMPLES;
		break;
	case MultiSampleType::Samples_4:
		return D3DMULTISAMPLE_4_SAMPLES;
		break;
	case MultiSampleType::Samples_5:
		return D3DMULTISAMPLE_5_SAMPLES;
		break;
	case MultiSampleType::Samples_6:
		return D3DMULTISAMPLE_6_SAMPLES;
		break;
	case MultiSampleType::Samples_7:
		return D3DMULTISAMPLE_7_SAMPLES;
		break;
	case MultiSampleType::Samples_8:
		return D3DMULTISAMPLE_8_SAMPLES;
		break;
	case MultiSampleType::Samples_9:
		return D3DMULTISAMPLE_9_SAMPLES;
		break;
	case MultiSampleType::Samples_10:
		return D3DMULTISAMPLE_10_SAMPLES;
		break;
	case MultiSampleType::Samples_11:
		return D3DMULTISAMPLE_11_SAMPLES;
		break;
	case MultiSampleType::Samples_12:
		return D3DMULTISAMPLE_12_SAMPLES;
		break;
	case MultiSampleType::Samples_13:
		return D3DMULTISAMPLE_13_SAMPLES;
		break;
	case MultiSampleType::Samples_14:
		return D3DMULTISAMPLE_14_SAMPLES;
		break;
	case MultiSampleType::Samples_15:
		return D3DMULTISAMPLE_15_SAMPLES;
		break;
	case MultiSampleType::Samples_16:
		return D3DMULTISAMPLE_16_SAMPLES;
		break;
	default:
		assert(false);
		return D3DMULTISAMPLE_NONE;
		break;
	}
}

DWORD D3DMapping::toD3DRenderStateValue(const ERenderState& rs, const uint32_t value)
{
	switch (rs)
	{
	case ERenderState::CULLMODE:
	{
		if (value == RSV_CULL_NONE) return D3DCULL_NONE;
		if (value == RSV_CULL_CW) return D3DCULL_CW;
		if (value == RSV_CULL_CCW) return D3DCULL_CCW;
		assert(false);
	}
	break;
	}
	return value;
}

D3DPOOL D3DMapping::toD3DPool(const EPoolType& pool)
{
	switch (pool)
	{
	case EPoolType::Default:
		return D3DPOOL_DEFAULT;
	case EPoolType::Managed:
		return D3DPOOL_MANAGED;
	case EPoolType::Scratch:
		return D3DPOOL_SCRATCH;
	case EPoolType::SystemMem:
		return D3DPOOL_SYSTEMMEM;
	default:
		return D3DPOOL_DEFAULT;
		break;
	}
}

D3DSAMPLERSTATETYPE D3DMapping::toD3DSamplerSatetType(const ESamplerStateType type)
{
	switch (type)
	{
	default:
		assert(false);
		return D3DSAMP_ADDRESSU;
	case ESamplerStateType::ADDRESSU:
		return D3DSAMP_ADDRESSU;
	case ESamplerStateType::ADDRESSV:
		return D3DSAMP_ADDRESSV;
	case ESamplerStateType::ADDRESSW:
		return D3DSAMP_ADDRESSW;
	case ESamplerStateType::BORDERCOLOR:
		return D3DSAMP_BORDERCOLOR;
	case ESamplerStateType::MAGFILTER:
		return D3DSAMP_MAGFILTER;
	case ESamplerStateType::MINFILTER:
		return D3DSAMP_MINFILTER;
	case ESamplerStateType::MIPFILTER:
		return D3DSAMP_MIPFILTER;
	case ESamplerStateType::MIPMAPLODBIAS:
		return D3DSAMP_MIPMAPLODBIAS;
	case ESamplerStateType::MAXMIPLEVEL:
		return D3DSAMP_MAXMIPLEVEL;
	case ESamplerStateType::MAXANISOTROPY:
		return D3DSAMP_MAXANISOTROPY;
	case ESamplerStateType::SRGBTEXTURE:
		return D3DSAMP_SRGBTEXTURE;
	case ESamplerStateType::ELEMENTINDEX:
		return D3DSAMP_ELEMENTINDEX;
	case ESamplerStateType::DMAPOFFSET:
		return D3DSAMP_DMAPOFFSET;
	}
}

D3DPRIMITIVETYPE D3DMapping::toD3DPrimitiveType(const EPrimitiveType type)
{
	switch (type)
	{
	default:
		return D3DPT_TRIANGLELIST;
	case EPrimitiveType::POINTLIST:
		return D3DPT_POINTLIST;
	case EPrimitiveType::LINELIST:
		return D3DPT_LINELIST;
	case EPrimitiveType::LINESTRIP:
		return D3DPT_LINESTRIP;
	case EPrimitiveType::TRIANGLELIST:
		return D3DPT_TRIANGLELIST;
	case EPrimitiveType::TRIANGLESTRIP:
		return D3DPT_TRIANGLESTRIP;
	case EPrimitiveType::TRIANGLEFAN:
		return D3DPT_TRIANGLEFAN;
	}
}
