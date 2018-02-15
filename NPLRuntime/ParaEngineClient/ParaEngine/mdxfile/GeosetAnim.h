#pragma once

#include "utils.h"
#include "ModelUtils.h"
#include <vector>

using namespace std;
namespace ParaEngine
{
struct AnimAlpha{
	long	frameNum;
	float	alphaValue;
};


//	KGAO
struct KGAO{
long	chunkNum;
long	lineType;	//(0:don't interp;1:linear;2:hermite;3:bezier)
long	flag;		//0xFFFFFFFF
void*	data;		// it is acutally AnimAlpha* 
public:
	KGAO(){data=NULL;}
	~KGAO(){
		if (data)
			delete [] data;
	}
};

struct GEOA{
	float		unk0;
	int			type;	//0:none 1:Drop Shadow 2:static color
	float		blue;	//static Color
	float		green;	//static Color
	float		red;	//static Color
	long	GeosetID;
};

/**
For animating the alpha of bones. This will cause bones to hide or unhide during animation.
TODO: currently X file does not support it. Only MDX file support it.
*/
class CGeosetAnim
{
public:
	CGeosetAnim(void);
	~CGeosetAnim(void);

	void	Read(TypePointer inP,int inSize);
	float	GetAnimAlpha(int geoID,const AnimInfo& animInfo);

private:
	int		numGeosetAnim;
	vector <GEOA*>	geosetAnim;
	vector <KGAO*>	geosetAlpha;

};
}