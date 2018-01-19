 

#ifndef __MODEL_UTILS_H__
#define __MODEL_UTILS_H__
namespace ParaEngine
{
struct AnimInfo{
	int		currentAnim;
	int		startFrame;
	int		endFrame;
	int		currentFrame;
	int		blendingStartFrame;
	int		blendingEndFrame;
	int		blendingFrame; // with which frame the current frame is blended
	float	blendingFactor; // [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame
	bool	update;	
};


//Linear Type (0:don't interpret;1:linear;2:hermite;3:bezier)
enum LineType{
	NOT_INTERP=0,
	LINEAR,
	HERMITE,
	BEZIER
};
}
#endif//__MODEL_UTILS_H__