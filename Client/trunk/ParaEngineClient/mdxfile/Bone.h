#ifndef __BONE_H__
#define __BONE_H__

#include "utils.h"
#include "ModelUtils.h"
#include "GeosetAnim.h"
#include "Texture.h"
#include <vector>
using namespace std;

namespace ParaEngine
{
class CSkeleton;

//line near 
struct LinearKeyFrame3{
	int			frameNum;
	D3DXVECTOR3 vec;
};
struct NonLinearKeyFrame3{
	int			frameNum;
	D3DXVECTOR3 vec;
	D3DXVECTOR3 inTan;
	D3DXVECTOR3 outTan;
};
struct LinearKeyFrame4{
	int			frameNum;
	D3DXQUATERNION 	vec;
};
struct NonLinearKeyFrame4{
	int			frameNum;
	D3DXQUATERNION 	vec;
	D3DXQUATERNION 	inTan;
	D3DXQUATERNION 	outTan;
};

/**
The class stands for the bone or frame in the character's skeleton. Animation data for the associated bone is also managed by this
class. It supports motion blending between bone poses. Bone matrices are updated from parent to children, through
the CalcTransformMatrix() function.

Fixes0.0:animation warping is added.modify Keyframe search now uses binary search, instead of linear search
However, there are still some unfixed problems. Meshes can hidded and unhidden using alpha frames.
We should prohibit motion warping between hidden frame and unhidden frames.  Yet all bones are 
calculated regardless of whether they are visible or not in current frame.
Fixes0.1: Now scaling animation is enabled. However, I will only support linear transformation, even if
hermite or bezier may be preferred. I may add support for the latter in the future.
Fixes0.2: Currently, animation uses offset matrix instead of pivot point. 
In the original pivot point approach, each bone will store a pivot point in the character mesh space.
and the animation of the vertices bound to a certain bone is calculated by applying RST(rotation|Scaling|translation )
around its pivot point in the character space. In the current approach, an offset matrix is used instead of the pivot points
offsetmatrix will first transform vertices to the local bone space, then RST is applied from child bones to parent bones
around the (0,0,0) point. This second approach is formal, and agrees with many 3D animation format such as X-file.
*/
class CBone1
{
public:
	CBone1(void);
	~CBone1(void);

	void		Read(TypePointer inP,int inSize);

	int			GetGeosetID()		{ return geosetID; };
	int			GetGeosetAnimID()	{ return geosetAnimID; };
	void		SetGeosetAlpha(float alpha)	{geosetAnimAlpha=alpha;};
	float		GetGeosetAlpha()	{return geosetAnimAlpha;};

	char*		GetBoneName()	{ return boneName; };
	int			GetObjectID()	{ return objectID; };
	int			GetParentID()	{ return parentID; };

	D3DXMATRIX*	GetFinalTransformMatrix() { return &finalMatrix; };
	D3DXMATRIX*	GetCombinedTransformMatrix() { return &combinedMatrix; };
	D3DXMATRIX*	CalcTransformMatrix(D3DXVECTOR3* center, const AnimInfo& animInfo,const D3DXMATRIX* parentMatrix);

	D3DXMATRIX* TransformMatrixMultiply(const D3DXMATRIX* matrix);

private:
bool			GetTransferMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo);
bool			GetTransferMatrixNoneBlending(D3DXVECTOR3* vec,const AnimInfo& animInfo);
bool			GetRotationMatrix(D3DXQUATERNION* quat,const AnimInfo& animInfo);
bool			GetRotationMatrixNoneBlending(D3DXQUATERNION* quat,const AnimInfo& animInfo);
bool			GetScaleMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo);
bool			GetScaleMatrixNoneBlending(D3DXVECTOR3* vec,const AnimInfo& animInfo);
bool			CheckHideByScale(const AnimInfo& animInfo);

public:

	/// Linear Type (0:don't interpret;1:linear;2:hermite;3:bezier)
	enum LineType{
		NOT_INTERP=0,
		LINEAR,			/// this is the most oftenly used.
		HERMITE,
		BEZIER
	};

	/** LiXizhi: this is the order defined by X file template */
	/// Rotation, Scaling, Translation Motion Keyframe Data
	enum MotionType{
		KGRT=0, 
		KGSC=1,
		KGTR=2,
		MAX_MOTION=3
	};

	int			objectID; /// the object ID, which is the same as the its index in the skeleton list
	int			parentID; /// the object ID of its parent bone
	int			nextID;	  /// the object ID of its next bone in the skeleton.-1 if it is the last bone
	D3DXMATRIX	localMatrix;	/// the localMatrix in the bone pose. localMatrix is used if no animation or translation keyframes is available for this bone
	D3DXMATRIX	combinedMatrix;	/// combinedMatrix will transform the vertices from the local bone space to its character space.
	D3DXMATRIX	offsetMatrix;	/// offsetMatrix will transform original mesh vertice from their bone pose to local bone space.
	D3DXMATRIX	finalMatrix;	/// finalMatrix = offsetMatrix*combinedMatrix or finalMatrix = localMatrix*combinedMatrix, this will animation vertices to its animated position


private:
	char*		boneName;
	int			nodeType;

	//key frames count
	int					keyFrameCount[MAX_MOTION];	/// how many keyframes in each motion type.
	//Linear Type 
	int					lineType[MAX_MOTION];	/// MotionType, how the pointers in keyFrames are interpreted
	void*				keyFrames[MAX_MOTION];	/// array of pointers to the first keyframe for each key type

	int			geosetID;		/// unused
	int			geosetAnimID;	/// alpha animation index, when the bone is hidden, transparent or solid.

	float		geosetAnimAlpha;/// by default it is 1.0
public:
	friend	class CMDXModel;
	friend	class CSkeleton;
};



class CBone2
{
public:
	CBone2(void);
	~CBone2(void);

	void		Read(TypePointer &inP);

	D3DXMATRIX*	GetFinalTransformMatrix() { return &transformMatrix; };
	D3DXMATRIX*	CalcTransformMatrix(D3DXVECTOR3* center, const AnimInfo& animInfo,const D3DXMATRIX* parentMatrix);

private:
	
bool			GetTransferMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo);
bool			GetRotationMatrix(D3DXQUATERNION* quat,const AnimInfo& animInfo);
bool			GetScaleMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo);
bool			CheckHideByScale(const AnimInfo& animInfo);

public:

	int count;

	//Linear Type (0:don't interpret;1:linear;2:hermite;3:bezier)
	enum LineType{
		NOT_INTERP=0,
		LINEAR,
		HERMITE,
		BEZIER
	};

	enum MotionType{
		KGTR=0,
		KGRT,
		KGSC,
		MAX_MOTION
	};

private:
	//key frames count
	int					keyFrameCount[MAX_MOTION];
	//Linear Type 
	int					lineType[MAX_MOTION];
	void*				keyFrames[MAX_MOTION];

	D3DXMATRIX	transformMatrix;
};





#define MDX_MAX_PARTICLES	100

typedef struct						// Create A Structure For Particle
{
	bool	used;
	bool	activated;					// Active (Yes/No)
	float	life;					// Particle Life
	float	fade;					// Fade Speed
	float	r;						// Red Value
	float	g;						// Green Value
	float	b;						// Blue Value
	float	x;						// X Position
	float	y;						// Y Position
	float	z;						// Z Position
	float	xi;						// X Direction
	float	yi;						// Y Direction
	float	zi;						// Z Direction
	float	xg;						// X Gravity
	float	yg;						// Y Gravity
	float	zg;						// Z Gravity
} MyParticle;							// Particle Structure




class CEmitter
{
public:
	CEmitter(void);
	~CEmitter(void);

	void		Read(TypePointer inP,int inSize);
	void		Render(D3DXMATRIX *parentMatrix, D3DXVECTOR3 *center, const AnimInfo& animInfo, long texbindid, long texwidth, long texlength);
	void		SetCurrentOptions(const AnimInfo& animInfo);
	
	char*		GetName()		{ return eName; };
	long		GetObjectID()	{ return objectID; };
	long		GetParentID()	{ return parentID; };
	
	struct TVisibility
	{
		long kframe;
		float f;
	};


	MyParticle Particles[MDX_MAX_PARTICLES];	// Particle Array (Room For Particle Info)	

	
	char*			eName;
	long			objectID;
	long			parentID;
	long			Flags;
	CBone2			bone;
	float			curSpeed;
	float			Variation;
	float			Latitude;
	float			Gravity;
	float			Lifespan;
	float			curEmissionRate;
	float			curLength;
	float			curWidth;
	long			FilterMode;
	long			Rows;
	long 			Columns;
	long			Flag2;
	float			TailLength;
	float			Time;

	struct colors3
	{			
		float	B;
		float	G;
		float	R;
	} SegmentColor[3];

	unsigned char	Alpha1, Alpha2, Alpha3;
	float			ScalingX, ScalingY, ScalingZ;
	long			LifeSpanUVAnim1, LifeSpanUVAnim2, LifeSpanUVAnim3;
	long			DecayUVAnim1, DecayUVAnim2, DecayUVAnim3;
	long			TailUVAnim1, TailUVAnim2, TailUVAnim3;
	long			TailDecayUVAnim1, TailDecayUVAnim2, TailDecayUVAnim3;
	long			TextureID;
	long			Squirt;
	long			PriorityPlane;
	long			ReplaceableID;

	long			numParticles;

	TVisibility*	Visibility;
	long			numVisibility;
	float			curVisibility;

	TVisibility*	EmissnRate;
	long			numEmissnRate;

	TVisibility*	Length;
	long			numLength;

	TVisibility*	Width;
	long			numWidth;

	TVisibility*	Speed;
	long			numSpeed;
};



#define MAX_NUM_EMMITTERS	4

class CEmitters
{
public:
	CEmitters(void);
	~CEmitters(void);
	void Cleanup();

	void		Read(TypePointer inP,int inSize);
	void		Render(const AnimInfo& animInfo, const CSkeleton *skel, CTexture *curtex);

	long		numEmitters;
	UINT		nEmitterArraySize;
	CEmitter*	Emitters;		// array
};

/**
Each x file model must has a skeleton(including static object) A skeleton contains a collection of bones.
call CalcTransformMatrix() to update the bone matrix
call GetTransformMatrix() for a weighted sum of a bone groups.(skinned animation)
*/
class CSkeleton
{
public:
	CSkeleton(void);
	~CSkeleton(void);
	
	void		Cleanup();

	void		ReadGeosetAnims(TypePointer inP,int inSize);
	void		ReadHelpers(TypePointer inP,int inSize);
	void		ReadBones(TypePointer inP,int inSize);
	void		ReadPivotPoints(TypePointer inP,int inSize);

	void		SortBones();
	void		CalcTransformMatrix(const AnimInfo& animInfo);

	float		GetTransformMatrix(D3DXMATRIX &mat,float& animAlpha,int *index,int count);

	CBone1*		CreateNewBone();

private:
	void		CalcTransformMatrix(const AnimInfo& animInfo,D3DXMATRIX *parentMatrix,int parentID);

public:
	CGeosetAnim		geosetAnims;
	int			nFirstBoneID;

	vector <CBone1*> bones;

	CEmitters		BlizEmitters2;			
};

}
#endif //__BONE_H__