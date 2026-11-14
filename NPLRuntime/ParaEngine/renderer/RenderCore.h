#pragma once
#include "PEtypes.h"

#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

#include "math/ParaMathUtility.h"
#include "RenderCoreOpenGL.h"
#include "Framework/Interface/Render/IRenderDevice.h"

//  defining structures as if DirectX exist to be used with ParaRHI backends.

namespace ParaEngine
{
	typedef struct _Para3DLight {
		ELightType    Type;            /* Type of light source */
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
	typedef IRHITexture* DeviceTexturePtr_type;
	class RenderDevice;
	typedef IRenderDevice* RenderDevicePtr;
	class CVertexDeclaration;
	typedef CVertexDeclaration* VertexDeclarationPtr;
}

#include "math/ParaViewport.h"
#include "ParaVertexBuffer.h"
#include "RenderDevice.h"
