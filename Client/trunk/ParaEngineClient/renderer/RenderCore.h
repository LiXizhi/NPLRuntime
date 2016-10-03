#pragma once
#ifdef USE_DIRECTX_RENDERER
#include "common/dxstdafx.h"
#elif defined(USE_OPENGL_RENDERER)
#include "platform/OpenGLWrapper.h"
#else
#include "PEtypes.h"

#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#endif

#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

#include "PEtypes.h"

#include "math/ParaMathUtility.h"
#ifndef USE_DIRECTX_RENDERER
#include "RenderCoreOpenGL.h"
#endif

//  defining structures as if DirectX exist to be used with OpengGL or Null renderer.

namespace ParaEngine
{
	typedef struct _Para3DLight {
		D3DLIGHTTYPE    Type;            /* Type of light source */
		LinearColor   Diffuse;         /* Diffuse color of light */
		LinearColor   Specular;        /* Specular color of light */
		LinearColor   Ambient;         /* Ambient color of light */
		Vector3       Position;         /* Position in world space */
		Vector3       Direction;        /* Direction in world space */
		float           Range;            /* Cutoff range */
		float           Falloff;          /* Falloff */
		float           Attenuation0;     /* Constant attenuation */
		float           Attenuation1;     /* Linear attenuation */
		float           Attenuation2;     /* Quadratic attenuation */
		float           Theta;            /* Inner angle of spotlight cone */
		float           Phi;              /* Outer angle of spotlight cone */
	} Para3DLight;

	typedef struct _ParaMaterial {
		LinearColor   Diffuse;        /* Diffuse color RGBA */
		LinearColor   Ambient;        /* Ambient color RGB */
		LinearColor   Specular;       /* Specular 'shininess' */
		LinearColor   Emissive;       /* Emissive color RGB */
		float           Power;          /* Sharpness if specular highlight */
	} ParaMaterial;
}

namespace ParaEngine
{
#ifdef USE_DIRECTX_RENDERER
	/** Get the texture for rendering */
	typedef IDirect3DTexture9 * DeviceTexturePtr_type;
	typedef IDirect3DDevice9* RenderDevicePtr;
	typedef LPDIRECT3DVERTEXDECLARATION9 VertexDeclarationPtr;
#elif defined(USE_OPENGL_RENDERER)
	typedef GLuint DeviceTexturePtr_type;
	class RenderDevice; typedef RenderDevice* RenderDevicePtr;
	class CVertexDeclaration; typedef CVertexDeclaration* VertexDeclarationPtr;
#else
	typedef uint32_t DeviceTexturePtr_type;
	class RenderDevice; typedef RenderDevice* RenderDevicePtr;
	class CVertexDeclaration; typedef CVertexDeclaration* VertexDeclarationPtr;
#endif
}

#include "ParaViewport.h"
#include "ParaVertexBuffer.h"
#include "RenderDevice.h"
