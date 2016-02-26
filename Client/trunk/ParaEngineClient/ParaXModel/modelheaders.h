#pragma once

namespace ParaEngine
{

/* Specifies packing alignment for ParaEngine model file structures as seen in the file.*/
#pragma pack(push,1)

enum ParaXModelType{
	PARAX_MODEL_ANIMATED=0,
	PARAX_MODEL_STATIC,
	PARAX_MODEL_DX_STATIC, // directX's X file
	PARAX_MODEL_INVALID,
	PARAX_MODEL_BMAX, // block max model with vertex color.
	PARAX_MODEL_UNKNOWN
};

enum ParaXModelFormat{
	// this has to be 1 for the rest bitfield to take effect. if this is 0, all model format are ignored. 
	PARAX_FORMAT_USEFORMAT = 1,
	// vertex has normal
	PARAX_FORMAT_NORMAL = 2,
	// vertex has second UV set
	PARAX_FORMAT_UV2 = 4,
	// Maybe add whether using advanced effect like normal and environment mapping etc. 
};

/**
* ParaX header definition: this header is shared by all ParaEngine model files.
*/
struct ParaXHeaderDef{
	char id[4];
	unsigned char version[4];
	/// ParaXModelType
	DWORD type; 
	DWORD IsAnimated;
	Vector3 minExtent;
	Vector3 maxExtent;
	/// bit wise of ParaXModelFormat. default to 0
	DWORD nModelFormat;
public:
	ParaXHeaderDef(){
		memcpy(id, "para", 4);
		version[0] = 1;
		version[1] = 0;
		version[2] = 0;
		version[3] = 0;
		type = PARAX_MODEL_ANIMATED;
		IsAnimated = 0;
		minExtent = Vector3::ZERO;
		maxExtent = Vector3::ZERO;
		nModelFormat = 0;
	}
};
static_assert(sizeof(ParaXHeaderDef) == 44, "compiler breaks packing rules");

/** old header for md2 file */
struct ModelHeader {
	char id[4];
	uint8 version[4];
	uint32 nameLength;
	uint32 nameOfs;
	uint32 type;

	uint32 nGlobalSequences;
	uint32 ofsGlobalSequences;
	uint32 nAnimations;
	uint32 ofsAnimations;
	uint32 nC;
	uint32 ofsC;
	uint32 nD;
	uint32 ofsD;
	uint32 nBones;
	uint32 ofsBones;
	uint32 nF;
	uint32 ofsF;

	uint32 nVertices;
	uint32 ofsVertices;
	uint32 nViews;
	uint32 ofsViews;

	uint32 nColors;
	uint32 ofsColors;

	uint32 nTextures;
	uint32 ofsTextures;

	uint32 nTransparency; // H
	uint32 ofsTransparency;
	uint32 nI;   // always unused ?
	uint32 ofsI;
	uint32 nTexAnims;	// J
	uint32 ofsTexAnims;
	uint32 nTexReplace;
	uint32 ofsTexReplace;

	uint32 nTexFlags;
	uint32 ofsTexFlags;
	uint32 nY;
	uint32 ofsY;

	uint32 nTexLookup;
	uint32 ofsTexLookup;

	uint32 nTexUnitLookup;		// L
	uint32 ofsTexUnitLookup;
	uint32 nTransparencyLookup; // M
	uint32 ofsTransparencyLookup;
	uint32 nTexAnimLookup;
	uint32 ofsTexAnimLookup;

	float floats[14];

	uint32 nBoundingTriangles;
	uint32 ofsBoundingTriangles;
	uint32 nBoundingVertices;
	uint32 ofsBoundingVertices;
	uint32 nBoundingNormals;
	uint32 ofsBoundingNormals;

	uint32 nAttachments; // O
	uint32 ofsAttachments;
	uint32 nAttachLookup; // P
	uint32 ofsAttachLookup;
	uint32 nQ; // Q
	uint32 ofsQ;
	uint32 nLights; // R
	uint32 ofsLights;
	uint32 nCameras; // S
	uint32 ofsCameras;
	uint32 nT;
	uint32 ofsT;
	uint32 nRibbonEmitters; // U
	uint32 ofsRibbonEmitters;
	uint32 nParticleEmitters; // V
	uint32 ofsParticleEmitters;

};

/** old texture definition for md2 file*/
struct ModelTextureDef {
	uint32 type;
	uint32 flags;
	uint32 nameLen;
	uint32 nameOfs;
};

/** ParaX vertices definition */
struct XVerticesDef
{
	/** 0 stand for ModelVertex */
	DWORD nType;
	DWORD nVertexBytes;
	DWORD nVertices;
	DWORD ofsVertices;
};
/** ParaX indice for view 0 */
struct Indice0Def{
	DWORD nIndices;
	DWORD ofsIndices;
};

// block B - animations
struct ModelAnimation {
	uint32 animID;
	uint32 timeStart;
	uint32 timeEnd;

	float moveSpeed;

	uint32 loopType; /// 1 for non-looping
	uint32 flags;
	uint32 d1;
	uint32 d2;
	uint32 playSpeed;  // note: this can't be play speed because it's 0 for some models

	Vector3 boxA, boxB;
	float rad;

	int16 s[2];
};


// sub-block in block E - animation data
struct AnimationBlock {
	int16 type;		// interpolation type (0=none, 1=linear, 2=hermite)
	int16 seq;		// global sequence id or -1
	uint32 nRanges;
	uint32 ofsRanges;
	uint32 nTimes;
	uint32 ofsTimes;
	uint32 nKeys;
	uint32 ofsKeys;
};
static_assert(sizeof(AnimationBlock) == 28, "compiler breaks packing rules");

// block E - bones
struct ModelBoneDef {
	int32 animid;
	int32 flags;
	int16 parent; // parent bone index
	int16 boneid; // id of a known (predefined) bone. this can be 0 or -1 which means unknown bones.
	AnimationBlock translation;
	AnimationBlock rotation;
	AnimationBlock scaling;
	Vector3 pivot;
};

struct ModelTexAnimDef {
	AnimationBlock trans, rot, scale;
};

struct ModelVertex {
	Vector3 pos;
	uint8 weights[4];
	uint8 bones[4];
	Vector3 normal;
	Vector2 texcoords;
	DWORD color0; // always 0,0 if they are unused
	DWORD color1;
};

struct ModelView {
    uint32 nIndex, ofsIndex; // Vertices in this model (index into vertices[])
    uint32 nTris, ofsTris;	 // indices
    uint32 nProps, ofsProps; // additional vtx properties
    uint32 nSub, ofsSub;	 // materials/renderops/submeshes
    uint32 nTex, ofsTex;	 // material properties/textures
	int32 lod;				 // LOD bias?
};


/// One material + render operation
struct ModelGeoset {
	uint16 id;		// mesh part id
	uint16 d2;		// 
	uint16 vstart;	// first vertex
	uint16 vcount;	// num vertices
	uint16 istart;	// first index
	uint16 icount;	// num indices
	union{
		struct {
			uint16 d3;	// first vertex
			uint16 d4;	// num vertices
		};
		int32 m_nVertexStart; // 32bits vertex start used in bmax model
	};
	uint16 d5;		// 
	uint16 d6;		// root bone
	Vector3 v;

	void SetVertexStart(int32 nStart){ 
		m_nVertexStart = nStart; 
	}
	int32 GetVertexStart() { return m_nVertexStart; }
};

/// A texture unit (sub of material)
struct ModelTexUnit{
	// probably the texture units
	// size always >=number of materials it seems
	uint16 flags;		// Flags
	uint16 order;		// ?
	uint16 op;			// Material this texture is part of (index into mat)
	uint16 op2;			// Always same as above?
	int16 colorIndex;	// color or -1
	uint16 flagsIndex;	// more flags...
	uint16 texunit;		// Texture unit (0 or 1)
	uint16 d4;			// ? (seems to be always 1)
	uint16 textureid;	// Texture id (index into global texture list)
	uint16 texunit2;	// copy of texture unit value?
	uint16 transid;		// transparency id (index into transparency list)
	uint16 texanimid;	// texture animation id
};

// block X - render flags
struct ModelRenderFlags {
	uint16 flags;
	uint16 blend;
};

// block G - color defs
struct ModelColorDef {
	AnimationBlock color;
	AnimationBlock opacity;
};

// block H - transp defs
struct ModelTransDef {
	AnimationBlock trans;
};

struct ModelLightDef {
	int16 type;
	int16 bone;
	Vector3 pos;
	AnimationBlock ambColor;
	AnimationBlock ambIntensity;
	AnimationBlock color;
	AnimationBlock intensity;
	AnimationBlock attStart;
	AnimationBlock attEnd;
	AnimationBlock unk1;
};

struct ModelCameraDef {
	int32 id;
	float fov, farclip, nearclip;
	AnimationBlock transPos;
	Vector3 pos;
	AnimationBlock transTarget;
	Vector3 target;
	AnimationBlock rot;
};


struct ModelParticleParams {
	float mid;
	uint32 colors[3];
	float sizes[3];
	int16 d[10];
	float unk[3];
	float scales[3];
	float slowdown;
	float rotation;
	float f2[16];
};

struct ModelParticleEmitterDef {
    int32 id;
	int32 flags;
	Vector3 pos;
	int16 bone;
	int16 texture;
	int32 nZero1;
	int32 ofsZero1;
	int32 nZero2;
	int32 ofsZero2;
	int16 blend;
	int16 type;
	int16 s1;
	int16 s2;
	int16 cols;
	int16 rows;
	AnimationBlock params[10];
	ModelParticleParams p;
	AnimationBlock unk;
};


struct ModelRibbonEmitterDef {
	int32 id;
	int32 bone;
	Vector3 pos;
	int32 nTextures;
	int32 ofsTextures;
	int32 nUnknown;
	int32 ofsUnknown;
	AnimationBlock color;
	AnimationBlock opacity;
	AnimationBlock above;
	AnimationBlock below;
	float res, length, unk;
	int16 s1, s2;
	AnimationBlock unk1;
	AnimationBlock unk2;
};


struct ModelBlockQ {
	char id[4];
	int32 dbid;
	int32 bone;
	Vector3 pos;
	int16 type;
	int16 seq;
	uint32 nRanges;
	uint32 ofsRanges;
	uint32 nTimes;
	uint32 ofsTimes;
};


struct ModelAttachmentDef {
	int32 id;
	int32 bone;
	Vector3 pos;
	AnimationBlock unk;
};
#pragma pack(pop)

/** all known bones*/
enum KNOWN_BONE_NODES
{
	Bone_Unknown = 0,//

	Bone_Root,
	Bone_Pelvis,
	Bone_Spine,
	Bone_L_Thigh,
	Bone_L_Calf,
	Bone_L_Foot,
	Bone_R_Thigh,
	Bone_R_Calf,
	Bone_R_Foot,

	Bone_L_Clavicle,
	Bone_L_UpperArm,
	Bone_L_Forearm,
	Bone_L_Hand,
	Bone_R_Clavicle,
	Bone_R_UpperArm,
	Bone_R_Forearm,
	Bone_R_Hand,

	Bone_Head,
	Bone_Neck,
	Bone_L_Toe0,
	Bone_R_Toe0,

	Bone_R_Finger0,
	Bone_L_Finger0,

	Bone_Spine1,
	Bone_Spine2,
	Bone_Spine3,

	Bone_forehand,
	Bone_L_eyelid,
	Bone_R_eyelid,
	Bone_L_eye,
	Bone_R_eye,
	Bone_B_eyelid,
	Bone_upper_lip,
	Bone_L_lip,
	Bone_R_lip,
	Bone_B_lip,
	Bone_chin,
	
	Bone_R_Finger01,
	Bone_L_Finger01,
	Bone_R_Finger1,
	Bone_L_Finger1,
	Bone_R_Finger11,
	Bone_L_Finger11,
	Bone_R_Finger2,
	Bone_L_Finger2,
	Bone_R_Finger21,
	Bone_L_Finger21,

	MAX_KNOWN_BONE_NODE,
};
}