#pragma once
#include "utils.h"

#include <vector>
using namespace std;

namespace ParaEngine
{
struct TextureEntity;
struct MyD3DTexture
{
	struct TextureBitmap{
		long	replaceableID; 
		char	texturePath[0x100];	//(0x100 bytes)
		long	unk0;				//(0)
		long	unk1;				//(0)
	}bitmap;

	DWORD width,height,type,subtype;

	asset_ptr<TextureEntity> pTextureEntity;
};
/**
Model textures are managed separately with the paraengine asset manager's texture pool.Textures used by model 
will be created and released at the same time of that of the 3D X file model. So if two models
uses the same texture it will be loaded twice. The 3D model is managed by the engine's asset manager
as one entity, including all its textures, mesh, and animation data.

Currently all DirectX supported texture file format and BLP format which is used by MDX are supported. 
All textures are in the managed pool.
*/
class CTexture
{
public:
	CTexture(void){};
	~CTexture(void);
	void Cleanup();

	void	Read(TypePointer inP,int inSize);
	LPDIRECT3DTEXTURE9 GetBindTexture(int index);

	/// create BLP based texture from mdx file
	MyD3DTexture* CreateTexture(const char * buffer, DWORD nBufferSize);
	/// create para x file based static texture
	MyD3DTexture* CreateTexture(const char* filename);
	
public:
	void InitDeviceObjects();
	void DeleteDeviceObjects();
	vector <MyD3DTexture*>	m_pTextures;	// dynamic arrays
};
}