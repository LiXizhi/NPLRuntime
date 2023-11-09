//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Co. All Rights Reserved.
// Author: LiXizhi
// Date: 2006.8
// Description:	API for ParaEngine types. 
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdarg.h> 
#include <algorithm> 
#include <math.h>
#include <limits>
#include <stack>
#include <iostream>
using namespace std;

#ifdef USE_DIRECTX_RENDERER
#include <d3dx9math.h>
#endif

// Cross-platform type definitions
#ifdef WIN32
#ifndef PARAENGINE_CLIENT
#ifndef WIN32_LEAN_AND_MEAN
/* Prevent inclusion of winsock.h in windows.h, otherwise boost::Asio will produce error in ParaEngineServer project: WinSock.h has already been included*/ 
#define WIN32_LEAN_AND_MEAN    
#endif
#endif
#ifndef NOMINMAX
#define  NOMINMAX
#endif
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>  

#else// For LINUX
#include <ctype.h>
#include <wctype.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#if (defined(__APPLE__)) || ((defined PARA_TARGET_PLATFORM) && (PARA_TARGET_PLATFORM == PARA_PLATFORM_IOS || PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC ))
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#endif // WIN32

#include <assert.h>

// Cross-platform type definitions
#ifdef WIN32
typedef signed char int8;
typedef short int16;
typedef long int32;
typedef __int64 int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned __int64 uint64;
typedef unsigned char byte;
#ifndef STDCALL 
#define STDCALL __stdcall
#endif 

#ifdef PARAENGINE_MOBILE
#ifndef SUCCEEDED
#define SUCCEEDED(hr)  ((HRESULT)(hr) >= 0)
#endif
#endif

#else // For LINUX
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#ifndef EMSCRIPTEN_SINGLE_THREAD
typedef uint8_t byte;
#endif
#define FALSE 0
#define TRUE 1
#define VOID            void
typedef void * HANDLE;
typedef void * HWND;
typedef void * HANDLE;
typedef void * HMODULE;
typedef void * HINSTANCE;
typedef void *PVOID;
typedef void *LPVOID;
typedef float FLOAT;
typedef uint32_t DWORD;
typedef DWORD *LPDWORD;
typedef const void * LPCVOID;
typedef char CHAR;
typedef char TCHAR;
typedef wchar_t WCHAR;
typedef uint16_t WORD;
typedef float               FLOAT;
#if !defined(OBJC_BOOL_DEFINED)
typedef signed char BOOL;
#endif
typedef unsigned char       BYTE;
typedef int                 INT;
typedef unsigned int        UINT;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int32_t HRESULT;

#ifndef LRESULT
#define LRESULT     int32
#endif
#ifndef WPARAM
#define WPARAM     uint32
#endif
#ifndef LPARAM
#define LPARAM     uint32
#endif
#ifndef CONST
#define CONST const
#endif
typedef WCHAR *LPWSTR;
typedef TCHAR *LPTSTR;
typedef const WCHAR *LPCWSTR;
typedef const TCHAR *LPCTSTR;
typedef const CHAR *LPCSTR;

typedef struct tagPOINT
{
	LONG  x;
	LONG  y;
} POINT;

typedef struct tagRECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} 	RECT;

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#ifndef STDCALL 
#define STDCALL
#endif 

#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#define E_FAIL		((HRESULT)(0x80000008L))
// emulate windows error msg in linux
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80070005L)
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80000003L)
#define E_PENDING                        _HRESULT_TYPEDEF_(0x8000000AL)

typedef struct _FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} 	FILETIME;
#define MAX_PATH          1024
#endif // WIN32

#ifndef SUCCEEDED
#define SUCCEEDED(hr)  ((HRESULT)(hr) >= 0)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)  if((x)!=0){delete (x);x=0;}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)  if((x)!=0){delete [] (x);x=0;}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef FAILED
#define FAILED(hr)  ((HRESULT)(hr) < 0)
#endif

#ifndef S_OK
#define S_OK  ((HRESULT)0x00000000L)
#endif

#ifndef S_FALSE
#define S_FALSE ((HRESULT)0x00000001L)
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX     0xff      /* maximum unsigned char value */
#endif

#ifndef MAX_PATH_LENGTH
/* this is fix for win32 MAX_PATH=260 limitation */
#define MAX_PATH_LENGTH     1024      
#endif


namespace ParaEngine
{
	/** event type */
	enum EventType{
		EVENT_MOUSE=0, // mouse click
		EVENT_KEY,
		EVENT_EDITOR,
		EVENT_SYSTEM,
		EVENT_NETWORK,
		EVENT_MOUSE_MOVE,
		EVENT_MOUSE_DOWN,
		EVENT_MOUSE_UP,
		EVENT_KEY_UP,
		EVENT_MOUSE_WHEEL,
		EVENT_TOUCH,
		EVENT_ACCELEROMETER,
		EVENT_LAST
	};
	/** bit fields */
	enum EventHandler_type
	{
		EH_MOUSE = 0x1, // mouse click
		EH_KEY = 0x1<<1,
		EH_EDITOR = 0x1<<2,
		EH_SYSTEM = 0x1<<3,
		EH_NETWORK = 0x1<<4,
		EH_MOUSE_MOVE = 0x1<<5,
		EH_MOUSE_DOWN = 0x1<<6,
		EH_MOUSE_UP = 0x1<<7,
		EH_KEY_UP = 0x1<<8,
		EH_MOUSE_WHEEL = 0x1<<9,
		EH_TOUCH = 0x1 << 10,
		EH_ACCELEROMETER = 0x1 << 11,
		EH_ALL = 0xffff,
	};

	/**
	* class ID for built-in classes only. 
	*/
	typedef int SClass_ID;  


	/** activation file type in the plug-in's Activate() function. */
	enum PluginActivationType
	{
		PluginActType_NONE = 0,
		/// this is obsoleted, use PluginActType_STATE
		PluginActType_SCODE, 
		/// from the NPL activate function call. the second paramter to LibActivate() will be pointer of INPLRuntimeState. Use GetCurrentMsg() and GetCurrentMsgLength() to retrieve the message. 
		PluginActType_STATE, 
	};

	/**
	* This class represents the unique class ID for a ParaEngine plug-in. A plug-ins Class_ID must be unique. 
	A Class_ID consists of two unsigned 32-bit quantities. The constructor assigns a value to each of these, 
	for example Class_ID(0x11261982, 0x19821126).
	@remark: Only the built-in classes (those that ship with ParaEngine) should have the second 32 bits equal to 0. 
	All plug-in developers should use both 32 bit quantities.
	*/
	class Class_ID 
	{
		unsigned long a,b;
	public:
		Class_ID() { a = b = 0xffffffff; }
		Class_ID(const Class_ID& cid) { a = cid.a; b = cid.b;	}
		Class_ID(unsigned long  aa, unsigned long  bb) { a = aa; b = bb; }
		unsigned long  PartA() { return a; }
		unsigned long  PartB() { return b; }
		void SetPartA( unsigned long  aa ) { a = aa; } //-- Added 11/21/96 GG
		void SetPartB( unsigned long  bb ) { b = bb; }
		int operator==(const Class_ID& cid) const { return (a==cid.a&&b==cid.b); }
		int operator!=(const Class_ID& cid) const { return (a!=cid.a||b!=cid.b); }
		Class_ID& operator=(const Class_ID& cid)  { a=cid.a; b = cid.b; return (*this); }
		// less operator - allows for ordering Class_IDs (used by stl maps for example) 
		bool operator<(const Class_ID& rhs) const
		{
			if ( a < rhs.a || ( a == rhs.a && b < rhs.b ) )
				return true;

			return false;
		}
	};

	/**
	* an interface ID 
	*/
	class Interface_ID 
	{
		unsigned long a,b;
	public:
		Interface_ID() { a = b = 0xffffffff; }
		Interface_ID(const Interface_ID& iid) { a = iid.a; b = iid.b;	}
		Interface_ID(unsigned long  aa, unsigned long  bb) { a = aa; b = bb; }
		unsigned long  PartA() { return a; }
		unsigned long  PartB() { return b; }
		void SetPartA( unsigned long  aa ) { a = aa; }
		void SetPartB( unsigned long  bb ) { b = bb; }
		int operator==(const Interface_ID& iid) const { return (a==iid.a&&b==iid.b); }
		int operator!=(const Interface_ID& iid) const { return (a!=iid.a||b!=iid.b); }
		Interface_ID& operator=(const Interface_ID& iid)  { a=iid.a; b = iid.b; return (*this); }
		// less operator - allows for ordering Class_IDs (used by stl maps for example) 
		bool operator<(const Interface_ID& rhs) const
		{
			if ( a < rhs.a || ( a == rhs.a && b < rhs.b ) )
				return true;

			return false;
		}
	};

	struct PARAVECTOR2	{
		float x;
		float y;
		PARAVECTOR2(float x_, float y_):x(x_), y(y_){}
		PARAVECTOR2(){};
	};

	struct PARAVECTOR3	{
		float x;
		float y;
		float z;
		PARAVECTOR3(float x_, float y_, float z_):x(x_), y(y_),z(z_){}
		PARAVECTOR3(){};
	};

	struct PARAVECTOR4	{
		float x;
		float y;
		float z;
		float w;
		PARAVECTOR4(float x_, float y_, float z_, float w_) :x(x_), y(y_), z(z_), w(w_){}
		PARAVECTOR4(){};
	};

	struct PARAMATRIX {
		union {
			struct {
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;
			};
			float m[4][4];
		};
	};

	struct PARAMATRIX3x3 {
		union {
			struct {
				float        _11, _12, _13;
				float        _21, _22, _23;
				float        _31, _32, _33;
			};
			float m[3][3];
		};
	};

	struct PARARECT {
		int32 x1;
		int32 y1;
		int32 x2;
		int32 y2;
	};

	struct PARACOLORVALUE {
		float r;
		float g;
		float b;
		float a;
	};

	
#if defined(WIN32) && defined(USE_DIRECTX_RENDERER)
	typedef D3DXMATRIX   DeviceMatrix;
	typedef D3DXMATRIX*   DeviceMatrix_ptr;
	typedef D3DXVECTOR2*  DeviceVector2_ptr;
	typedef D3DXVECTOR3*  DeviceVector3_ptr;
	typedef D3DXVECTOR4*  DeviceVector4_ptr;
	typedef D3DXVECTOR2  DeviceVector2;
	typedef D3DXVECTOR3  DeviceVector3;
	typedef D3DXVECTOR4  DeviceVector4;

	//typedef PARAMATRIX  DeviceMatrix;
	//typedef PARAMATRIX*  DeviceMatrix_ptr;
	//typedef PARAVECTOR2*  DeviceVector2_ptr;
	//typedef PARAVECTOR3*  DeviceVector3_ptr;
	//typedef PARAVECTOR4*  DeviceVector4_ptr;
	//typedef PARAVECTOR2  DeviceVector2;
	//typedef PARAVECTOR3  DeviceVector3;
	//typedef PARAVECTOR4  DeviceVector4;
#else
	typedef PARAMATRIX  DeviceMatrix;
	typedef PARAMATRIX*  DeviceMatrix_ptr;
	typedef PARAVECTOR2*  DeviceVector2_ptr;
	typedef PARAVECTOR3*  DeviceVector3_ptr;
	typedef PARAVECTOR4*  DeviceVector4_ptr;
	typedef PARAVECTOR2  DeviceVector2;
	typedef PARAVECTOR3  DeviceVector3;
	typedef PARAVECTOR4  DeviceVector4;
#endif

	enum EnumForceInit
	{
		ForceInit,
		ForceInitToZero
	};
	enum EnumNoInit { NoInit };
}

// forward declare in rough alphabetic order
namespace ParaEngine
{
	struct ActiveBiped;
	class Angle;
	class AxisAlignedBox;
	struct AssetEntity;
	class CAIBase;
	class CAISimulator;
	class CAnimInstanceBase;
	class CAttributeField;
	class CAttributeClass;
	class CAttributesManager;
	class CAudioEngine;
	class CAutoCamera;
	class CBaseCamera;
	class CBaseObject;
	class CBipedStateManager;
	class CDataProviderManager;
	class CEventsCenter;
	class CEventBinding;
	class CFileManager;
	class CFrameRateController;
	class CGUIBase;
	class CGUIResource;
	class CGUIRoot;
	class CGUIEvent;
	class CharModelInstance;
	struct CharacterPose;
	class CLightManager;
	class CMeshObject;
	class CMoviePlatform;
	class CMiniSceneGraph;
	class CManagedLoader;
	class CMissileObject;
	class CMirrorSurface;
	struct CNpcDbItem;
	class CSceneObject;
	class CSelectionManager;
	class COceanManager;
	class CParaFile;
	class CParameterBlock;
	class CParaWorldAsset;
	class CParaXAnimInstance;
	class CPhysicsWorld;
	class CPluginManager;
	class CPortalNode;
	class CReport;
	class CRpgCharacter;
	class CShapeAABB;
	class CShapeSphere;
	class CShapeOBB;
	class CSunLight;
	class CSkyMesh;
	class CTerrainTile;
	class CWorldInfo;
	class CZipWriter;
	class CZoneNode;
	struct DatabaseEntity;
	class Degree;
	class DirectXEngine;
	class TransformStack;
	class EffectManager;
	struct GUILAYER;
	struct CGUIPosition;
	struct GUIFontElement;
	struct GUITextureElement;
	class IAttributeFields;
	class IEnvironmentSim;
	class IGameObject;
	struct MeshEntity;
	class Math;
	class Matrix3;
	class Matrix4;
	struct MultiAnimationEntity;
	struct MDXEntity;
	class ParaEngineSettings;
	class Plane;
	class PlaneBoundedVolume;
	class Quaternion;
	class Radian;
	class Ray;
	struct SceneState;
	class ShadowVolume;
	class Sphere;
	struct TextureEntity;
	struct ParaXEntity;
	class Vector2;
	class Vector3;
	class Vector4;
	class XRefObject;
	class CViewportManager;
}

namespace ParaTerrain{
	class CGlobalTerrain;
}

namespace NPL
{
	struct NPLAddress;
	class CNPLConnection;
	class CNPLUDPRoute;
	class CNPLConnectionManager;
	class CNPLDispatcher;
	struct NPLFileName;
	class CNPLNetServer;
	struct NPLMessage;
	class CNPLMessageQueue;
	struct NPLMsgHeader;
	struct NPLMsgIn;
	class NPLMsgIn_parser;
	class NPLMsgOut;
	class CNPLRuntime;
	struct NPLRuntimeAddress;
	class CNPLRuntimeState;
	class CNPLScriptingState;
	class NPLServerInfo;
	class INPLStimulationPipe;

	struct NPLTimer;
}

namespace ParaInfoCenter{
	class CICConfigManager;
}

namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;
}

namespace ParaScripting
{
	class ParaObject;
	class ParaAssetObject;
}
struct lua_State;
class TiXmlNode;
namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;
}


