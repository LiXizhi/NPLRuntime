#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "utils.h"
#include "ModelUtils.h"

#include <vector>
using namespace std;
namespace ParaEngine
{
struct LinearAlpha{
	int		frameNum;
	float	alphaValue;		//(0.0f -> 1.0f)
};

struct NonLinearAlpha{
	int		frameNum;
	float	alphaValue;		//(0.0f -> 1.0f)
	float	inTan;
	float	outTan;
};


struct LayerAlpha{
	long	chunkNum;
	long	LineType;		//(0:don't interp;1:linear;2:hermite;3:bezier)
	//0xFFFFFFFF
	void*	data;			//LinearAlpha or NonLinearAlpha
public:
	LayerAlpha()
	{
		data = NULL;
	}
	~LayerAlpha()
	{
		delete [] data;
	}
};

struct Layer
{ 
	int			layerSize;		// in Bytes
	int			filterMode;		//(0:none;1:transparent;2:blend;3:additive)
	int			shading;		//(1:unshaded;+16:two sided;+32:unfogged)
	int			textureID;
	int			unk5;			//0xFFFFFFFF
	int			unk6;			//(0)
	float		alpha;			//(0(transparent)->1(opaque))
	BOOL		existKMTA;
	LayerAlpha	KMTA;
};

class CMaterial
{
public:
	CMaterial ();
	~CMaterial();
	void Cleanup();
	void Read(TypePointer inP,int inSize);
	float getFrameAlpha(int frameNum,int layerNum);

	enum{
		FILTER_NONE=0,
		FILTER_TRANSPARENT,
		FILTER_BLEND,
		FILTER_ADDITIVE
	};

	Layer* CreateLayer();
	vector <Layer*>	layers;		//dynamic array

private:
};

class CMaterialMap
{
public:
	CMaterialMap(void):materials(NULL) {}
	~CMaterialMap();
	void Cleanup();
	void Read(TypePointer inP,int inSize);

	CMaterial* CreateMaterial();

#define MAX_MATERIALS_BLOCK 16

	vector <CMaterial*>	materials;		//array

private:
};
}
#endif//__MATERIAL_H__