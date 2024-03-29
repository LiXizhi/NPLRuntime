#pragma once

namespace ParaEngine
{
	// Primitives supported by draw-primitive API
	class EPrimitiveType {
	public:
		static const D3DPRIMITIVETYPE POINTLIST = D3DPT_POINTLIST;
		static const D3DPRIMITIVETYPE LINELIST = D3DPT_LINELIST;
		static const D3DPRIMITIVETYPE LINESTRIP = D3DPT_LINESTRIP;
		static const D3DPRIMITIVETYPE TRIANGLELIST = D3DPT_TRIANGLELIST;
		static const D3DPRIMITIVETYPE TRIANGLESTRIP = D3DPT_TRIANGLESTRIP;
		static const D3DPRIMITIVETYPE TRIANGLEFAN = D3DPT_TRIANGLEFAN;
	};

	class ETransformsStateType
	{
	public:
		static const D3DTRANSFORMSTATETYPE WORLD = D3DTS_WORLD;
		static const D3DTRANSFORMSTATETYPE VIEW = D3DTS_VIEW;
		static const D3DTRANSFORMSTATETYPE PROJECTION = D3DTS_PROJECTION;
		static const D3DTRANSFORMSTATETYPE TEXTURE0 = D3DTS_TEXTURE0;
		static const D3DTRANSFORMSTATETYPE TEXTURE1 = D3DTS_TEXTURE1;
		static const D3DTRANSFORMSTATETYPE TEXTURE2 = D3DTS_TEXTURE2;
		static const D3DTRANSFORMSTATETYPE TEXTURE3 = D3DTS_TEXTURE3;
		static const D3DTRANSFORMSTATETYPE TEXTURE4 = D3DTS_TEXTURE4;
		static const D3DTRANSFORMSTATETYPE TEXTURE5 = D3DTS_TEXTURE5;
		static const D3DTRANSFORMSTATETYPE TEXTURE6 = D3DTS_TEXTURE6;
		static const D3DTRANSFORMSTATETYPE TEXTURE7 = D3DTS_TEXTURE7;
	};

	class ERenderState
	{
	public:
		static const D3DRENDERSTATETYPE ZENABLE = D3DRS_ZENABLE;
		static const D3DRENDERSTATETYPE FILLMODE = D3DRS_FILLMODE;
		static const D3DRENDERSTATETYPE SHADEMODE = D3DRS_SHADEMODE;
		static const D3DRENDERSTATETYPE ZWRITEENABLE = D3DRS_ZWRITEENABLE;
		static const D3DRENDERSTATETYPE ALPHATESTENABLE = D3DRS_ALPHATESTENABLE;
		static const D3DRENDERSTATETYPE LASTPIXEL = D3DRS_LASTPIXEL;
		static const D3DRENDERSTATETYPE SRCBLEND = D3DRS_SRCBLEND;
		static const D3DRENDERSTATETYPE DESTBLEND = D3DRS_DESTBLEND;
		static const D3DRENDERSTATETYPE CULLMODE = D3DRS_CULLMODE;
		static const D3DRENDERSTATETYPE ZFUNC = D3DRS_ZFUNC;
		static const D3DRENDERSTATETYPE ALPHAREF = D3DRS_ALPHAREF;
		static const D3DRENDERSTATETYPE ALPHAFUNC = D3DRS_ALPHAFUNC;
		static const D3DRENDERSTATETYPE DITHERENABLE = D3DRS_DITHERENABLE;
		static const D3DRENDERSTATETYPE ALPHABLENDENABLE = D3DRS_ALPHABLENDENABLE;
		static const D3DRENDERSTATETYPE FOGENABLE = D3DRS_FOGENABLE;
		static const D3DRENDERSTATETYPE SPECULARENABLE = D3DRS_SPECULARENABLE;
		static const D3DRENDERSTATETYPE FOGCOLOR = D3DRS_FOGCOLOR;
		static const D3DRENDERSTATETYPE FOGTABLEMODE = D3DRS_FOGTABLEMODE;
		static const D3DRENDERSTATETYPE FOGSTART = D3DRS_FOGSTART;
		static const D3DRENDERSTATETYPE FOGEND = D3DRS_FOGEND;
		static const D3DRENDERSTATETYPE FOGDENSITY = D3DRS_FOGDENSITY;
		static const D3DRENDERSTATETYPE RANGEFOGENABLE = D3DRS_RANGEFOGENABLE;
		static const D3DRENDERSTATETYPE STENCILENABLE = D3DRS_STENCILENABLE;
		static const D3DRENDERSTATETYPE STENCILFAIL = D3DRS_STENCILFAIL;
		static const D3DRENDERSTATETYPE STENCILZFAIL = D3DRS_STENCILZFAIL;
		static const D3DRENDERSTATETYPE STENCILPASS = D3DRS_STENCILPASS;
		static const D3DRENDERSTATETYPE STENCILFUNC = D3DRS_STENCILFUNC;
		static const D3DRENDERSTATETYPE STENCILREF = D3DRS_STENCILREF;
		static const D3DRENDERSTATETYPE STENCILMASK = D3DRS_STENCILMASK;
		static const D3DRENDERSTATETYPE STENCILWRITEMASK = D3DRS_STENCILWRITEMASK;
		static const D3DRENDERSTATETYPE TEXTUREFACTOR = D3DRS_TEXTUREFACTOR;
		static const D3DRENDERSTATETYPE WRAP0 = D3DRS_WRAP0;
		static const D3DRENDERSTATETYPE WRAP1 = D3DRS_WRAP1;
		static const D3DRENDERSTATETYPE WRAP2 = D3DRS_WRAP2;
		static const D3DRENDERSTATETYPE WRAP3 = D3DRS_WRAP3;
		static const D3DRENDERSTATETYPE WRAP4 = D3DRS_WRAP4;
		static const D3DRENDERSTATETYPE WRAP5 = D3DRS_WRAP5;
		static const D3DRENDERSTATETYPE WRAP6 = D3DRS_WRAP6;
		static const D3DRENDERSTATETYPE WRAP7 = D3DRS_WRAP7;
		static const D3DRENDERSTATETYPE CLIPPING = D3DRS_CLIPPING;	
		static const D3DRENDERSTATETYPE LIGHTING = D3DRS_LIGHTING;
		static const D3DRENDERSTATETYPE AMBIENT = D3DRS_AMBIENT;
		static const D3DRENDERSTATETYPE FOGVERTEXMODE = D3DRS_FOGVERTEXMODE;
		static const D3DRENDERSTATETYPE COLORVERTEX = D3DRS_COLORVERTEX;
		static const D3DRENDERSTATETYPE LOCALVIEWER = D3DRS_LOCALVIEWER;
		static const D3DRENDERSTATETYPE NORMALIZENORMALS = D3DRS_NORMALIZENORMALS;
		static const D3DRENDERSTATETYPE DIFFUSEMATERIALSOURCE = D3DRS_DIFFUSEMATERIALSOURCE;
		static const D3DRENDERSTATETYPE SPECULARMATERIALSOURCE = D3DRS_SPECULARMATERIALSOURCE;
		static const D3DRENDERSTATETYPE AMBIENTMATERIALSOURCE = D3DRS_AMBIENTMATERIALSOURCE;
		static const D3DRENDERSTATETYPE EMISSIVEMATERIALSOURCE = D3DRS_EMISSIVEMATERIALSOURCE;
		static const D3DRENDERSTATETYPE VERTEXBLEND = D3DRS_VERTEXBLEND;
		static const D3DRENDERSTATETYPE CLIPPLANEENABLE = D3DRS_CLIPPLANEENABLE;
		static const D3DRENDERSTATETYPE POINTSIZE = D3DRS_POINTSIZE;
		static const D3DRENDERSTATETYPE POINTSIZE_MIN = D3DRS_POINTSIZE_MIN;
		static const D3DRENDERSTATETYPE POINTSPRITEENABLE = D3DRS_POINTSPRITEENABLE;
		static const D3DRENDERSTATETYPE POINTSCALEENABLE = D3DRS_POINTSCALEENABLE;
		static const D3DRENDERSTATETYPE POINTSCALE_A = D3DRS_POINTSCALE_A;
		static const D3DRENDERSTATETYPE POINTSCALE_B = D3DRS_POINTSCALE_B;
		static const D3DRENDERSTATETYPE POINTSCALE_C = D3DRS_POINTSCALE_C;
		static const D3DRENDERSTATETYPE MULTISAMPLEANTIALIAS = D3DRS_MULTISAMPLEANTIALIAS;
		static const D3DRENDERSTATETYPE MULTISAMPLEMASK = D3DRS_MULTISAMPLEMASK;
		static const D3DRENDERSTATETYPE PATCHEDGESTYLE = D3DRS_PATCHEDGESTYLE;
		static const D3DRENDERSTATETYPE DEBUGMONITORTOKEN = D3DRS_DEBUGMONITORTOKEN;
		static const D3DRENDERSTATETYPE POINTSIZE_MAX = D3DRS_POINTSIZE_MAX;
		static const D3DRENDERSTATETYPE INDEXEDVERTEXBLENDENABLE = D3DRS_INDEXEDVERTEXBLENDENABLE;
		static const D3DRENDERSTATETYPE COLORWRITEENABLE = D3DRS_COLORWRITEENABLE;
		static const D3DRENDERSTATETYPE TWEENFACTOR = D3DRS_TWEENFACTOR;
		static const D3DRENDERSTATETYPE BLENDOP = D3DRS_BLENDOP;
		static const D3DRENDERSTATETYPE POSITIONDEGREE = D3DRS_POSITIONDEGREE;
		static const D3DRENDERSTATETYPE NORMALDEGREE = D3DRS_NORMALDEGREE;
		static const D3DRENDERSTATETYPE SCISSORTESTENABLE = D3DRS_SCISSORTESTENABLE;
		static const D3DRENDERSTATETYPE SLOPESCALEDEPTHBIAS = D3DRS_SLOPESCALEDEPTHBIAS;
		static const D3DRENDERSTATETYPE ANTIALIASEDLINEENABLE = D3DRS_ANTIALIASEDLINEENABLE;
		static const D3DRENDERSTATETYPE MINTESSELLATIONLEVEL = D3DRS_MINTESSELLATIONLEVEL;
		static const D3DRENDERSTATETYPE MAXTESSELLATIONLEVEL = D3DRS_MAXTESSELLATIONLEVEL;
		static const D3DRENDERSTATETYPE ADAPTIVETESS_X = D3DRS_ADAPTIVETESS_X;
		static const D3DRENDERSTATETYPE ADAPTIVETESS_Y = D3DRS_ADAPTIVETESS_Y;
		static const D3DRENDERSTATETYPE ADAPTIVETESS_Z = D3DRS_ADAPTIVETESS_Z;
		static const D3DRENDERSTATETYPE ADAPTIVETESS_W = D3DRS_ADAPTIVETESS_W;
		static const D3DRENDERSTATETYPE ENABLEADAPTIVETESSELLATION = D3DRS_ENABLEADAPTIVETESSELLATION;
		static const D3DRENDERSTATETYPE TWOSIDEDSTENCILMODE = D3DRS_TWOSIDEDSTENCILMODE;
		static const D3DRENDERSTATETYPE CCW_STENCILFAIL = D3DRS_CCW_STENCILFAIL;
		static const D3DRENDERSTATETYPE CCW_STENCILZFAIL = D3DRS_CCW_STENCILZFAIL;
		static const D3DRENDERSTATETYPE CCW_STENCILPASS = D3DRS_CCW_STENCILPASS;
		static const D3DRENDERSTATETYPE CCW_STENCILFUNC = D3DRS_CCW_STENCILFUNC;
		static const D3DRENDERSTATETYPE COLORWRITEENABLE1 = D3DRS_COLORWRITEENABLE1;
		static const D3DRENDERSTATETYPE COLORWRITEENABLE2 = D3DRS_COLORWRITEENABLE2;
		static const D3DRENDERSTATETYPE COLORWRITEENABLE3 = D3DRS_COLORWRITEENABLE3;
		static const D3DRENDERSTATETYPE BLENDFACTOR = D3DRS_BLENDFACTOR;
		static const D3DRENDERSTATETYPE SRGBWRITEENABLE = D3DRS_SRGBWRITEENABLE;
		static const D3DRENDERSTATETYPE DEPTHBIAS = D3DRS_DEPTHBIAS;
		static const D3DRENDERSTATETYPE WRAP8 = D3DRS_WRAP8;
		static const D3DRENDERSTATETYPE WRAP9 = D3DRS_WRAP9;
		static const D3DRENDERSTATETYPE WRAP10 = D3DRS_WRAP10;
		static const D3DRENDERSTATETYPE WRAP11 = D3DRS_WRAP11;
		static const D3DRENDERSTATETYPE WRAP12 = D3DRS_WRAP12;
		static const D3DRENDERSTATETYPE WRAP13 = D3DRS_WRAP13;
		static const D3DRENDERSTATETYPE WRAP14 = D3DRS_WRAP14;
		static const D3DRENDERSTATETYPE WRAP15 = D3DRS_WRAP15;
		static const D3DRENDERSTATETYPE SEPARATEALPHABLENDENABLE = D3DRS_SEPARATEALPHABLENDENABLE;
		static const D3DRENDERSTATETYPE SRCBLENDALPHA = D3DRS_SRCBLENDALPHA;
		static const D3DRENDERSTATETYPE DESTBLENDALPHA = D3DRS_DESTBLENDALPHA;
		static const D3DRENDERSTATETYPE BLENDOPALPHA = D3DRS_BLENDOPALPHA;
		static const D3DRENDERSTATETYPE FORCE_DWORD = D3DRS_FORCE_DWORD;	
	};

	class RenderDeviceBase
	{
	public:
		enum StatisticsType
		{
			DRAW_PERF_TRIANGLES_UNKNOWN = 0,
			DRAW_PERF_TRIANGLES_UI = 1,
			DRAW_PERF_TRIANGLES_TERRAIN = 2,
			DRAW_PERF_TRIANGLES_CHARACTER = 3,
			DRAW_PERF_TRIANGLES_MESH = 4,
			// total number of draw calls
			DRAW_PERF_DRAW_CALL_COUNT = 5,
			// total triangles
			DRAW_PERF_TOTAL_TRIANGLES = 6,
			DRAW_PERF_END,
		};
	public:
		/** get the performance counter for a given type */
		static int GetPerfCount(StatisticsType nStatisticsType);
		/** reset all the performance counters to 0. this is usually called at the beginning of each frame */
		static void ClearAllPerfCount();

		/** increment both draw count and vertices */
		static void IncrementDrawBatchAndVertices(int nDrawCount, int nVertices, int nPrimitiveType = DRAW_PERF_TRIANGLES_UNKNOWN);

		/* max number of simultaneous textures. in most cards, it is 8, some is 4. */
		static int GetMaxSimultaneousTextures();
	public:

		static int g_perfCounters[DRAW_PERF_END];
	};
}

#ifdef USE_DIRECTX_RENDERER
#include "RenderDeviceDirectX.h"
#elif defined(USE_OPENGL_RENDERER)
#include "RenderDeviceOpenGL.h"
#else
#include "RenderDeviceOpenGL.h"
#endif