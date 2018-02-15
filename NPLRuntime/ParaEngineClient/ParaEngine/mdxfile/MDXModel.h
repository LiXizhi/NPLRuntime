#pragma once

#include "skeleton.h"
#include "geoset.h"
#include "texture.h"
#include "material.h"
#include "sequence.h"
#include "ModelUtils.h"
#include "GeosetAnim.h"
#include "utils.h"

namespace ParaEngine
{
extern void *malloc_func(const size_t size);
extern void free_func(void* ptr);
extern DWORD FromMPQ1(char*szFichier, char*& buffer);
extern DWORD FromMPQ(char*szFichier, char*& buffer);


struct camera
{
	char cName[80];
	float posx;
	float posy;
	float posz;
	float cFOV;
	float farClip;
	float nearClip;
	float targetx;
	float targety;
	float targetz;
};

class CCameras
{
#define MAX_CAMERAS 32
public:
	int				mNumCameras;
	camera*			mCameras;
	BOOL			mLoaded;

	CCameras() : mNumCameras(NULL),mLoaded(false), mCameras(NULL)	{ }
	~CCameras();

	void			Read				(TypePointer inP, int inSize);
	void			Render				(void);
public:
	void			Cleanup();
};

struct SceneState;
struct ParaXFile;
class ShadowVolume;
struct LightParams;
/**
This is the ParaX mesh and animation file format model. 
It can load from Para X file or MDX file, and save to X file.
Please refer to "ParaEngineXFileDef.h" for Para X file template definition.
It also supports shadow casting.
*/
class CMDXModel
{
public:
	CMDXModel(void);
	~CMDXModel(void);

	bool		Load(const char* inName);
	
	bool		LoadFromX(const char* inName);
	bool		SaveToX(const char* inName, bool bText);
	
	void		InitDeviceObjects();
	void		DeleteDeviceObjects();
	
	void		AdvanceTime( double dTimeDelta );
	void		Render(SceneState* sceneState);

	void		UpdateAnim(int animNum);
	int			GetAnimNum(const char* animName);
	void		PreviousAnim();
	void		NextAnim();
	void		SetModelColor(float r,float g,float b);
	void		SetModelColor(D3DXCOLOR modelcolor);
	D3DXCOLOR*	GetModelColor(){return &modelColor;};
	void		SetBlendingTime(float fTime);
	void		BuildShadowVolume(ShadowVolume * pShadowVolume, LightParams* pLight, D3DXMATRIX* mxWorld);

	/**
	* @params pMin:[out] Pointer to a D3DXVECTOR3 structure, describing the returned lower-left 
	*	corner of the bounding box. See Remarks. 
	* @params pMax:[out] Pointer to a D3DXVECTOR3 structure, describing the returned upper-right 
	*	corner of the bounding box. See Remarks.
	* @ return: bounding sphere's radius
	*/
	float ComputeBoundingBox(D3DXVECTOR3 *pMin, D3DXVECTOR3 *pMax );

public:
	CSkeleton		skeleton;
	CGeoset			geosets;
	CMaterialMap	materialMap;
	CTexture		texture;
	CSequence		sequences;
	CCameras		cameras;

	AnimInfo		currentAnimInfo;
	AnimInfo		nextAnimInfo;
	float			fBlendingTime;	// blending time in seconds
	
	// shadow cap
	int				m_nUseShadowCap; // value >0, if enforcing the model to use caps in its shadow volume, default is 0
	// index of the default animation
	int				nDefaultStandIndex;
	int				nDefaultWalkIndex;
protected:
private:
	long			dataSize;
	static char*			mdxData;
	static ParaXFile*		xData;
	D3DXCOLOR		modelColor;

	void		ParseXFileData(ID3DXFileData *pData);
	bool		BuildFromX();

public:
	// Set/get the current Animation info
	void SetCurrentAnimInfo(AnimInfo* ai){currentAnimInfo = *ai;};
	void GetCurrentAnimInfo(AnimInfo* ai){*ai = currentAnimInfo;};
};
}