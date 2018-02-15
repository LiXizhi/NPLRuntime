#pragma once
#include "animated.h"

namespace ParaEngine
{
	class CParaXModel;

	struct ModelColor {
		Animated<Vector3> color;
		AnimatedShort opacity;
	};

	struct ModelTransparency {
		AnimatedShort trans;
	};

	enum BlendModes {
		BM_OPAQUE,
		BM_TRANSPARENT,
		BM_ALPHA_BLEND,
		BM_ADDITIVE,
		BM_ADDITIVE_ALPHA,
		BM_MODULATE,
		BM_TEMP_FORCEALPHABLEND = 0X8000,
	};

	struct ModelCamera {
		bool m_bIsValid;

		Vector3 pos, target;
		float nearclip, farclip, fov;
		Animated<Vector3> tPos, tTarget;
		Animated<float> rot;

		void setup(int time = 0);

		ModelCamera() :m_bIsValid(false) {}
	};

	struct ModelLight {
		int type, parent;
		Vector3 pos, tpos, dir, tdir;
		Animated<Vector3> diffColor, ambColor;
		Animated<float> diffIntensity, ambIntensity;

		void setup(int time, uint32 l);
	};


	struct ModelAttachment {
		int id;
		Vector3 pos;
		int bone;

		void setup(CParaXModel *model);
	};
	struct ParaXModelObjNum{
		uint32 nGlobalSequences;
		uint32 nAnimations;
		uint32 nBones;
		uint32 nVertices;
		uint32 nViews;
		uint32 nColors;
		uint32 nTextures;
		uint32 nTransparency; // H
		uint32 nTexAnims;	// J
		uint32 nTexReplace;
		uint32 nTexFlags;
		uint32 nTexLookup;
		uint32 nTexUnitLookup;		// L
		uint32 nTransparencyLookup; // M
		uint32 nTexAnimLookup;
		uint32 nAttachments; // O
		uint32 nAttachLookup; // P
		uint32 nLights; // R
		uint32 nCameras; // S
		uint32 nRibbonEmitters; // U
		uint32 nParticleEmitters; // V
		uint32 nIndices;
	};

	/**
	* replaceable skins for non-customizable characters.
	*/
	struct ReplaceableTextureGroup {
		static const int num = 3;
		static const int ReplaceableSkinBase = 11;
		// base index in ParaX model's replaceable texture array
		int base;
		// the number of textures. it is no larger than num.
		int count;
		std::string tex[num];
		ReplaceableTextureGroup()
			:base(ReplaceableSkinBase), count(0)
		{
			for (int i = 0; i<num; i++) {
				tex[i] = "";
			}
		}
		// default copy constructor
		ReplaceableTextureGroup(const ReplaceableTextureGroup&grp)
		{
			for (int i = 0; i<num; i++) {
				tex[i] = grp.tex[i];
			}
			base = grp.base;
			count = grp.count;
		}
		const bool operator<(const ReplaceableTextureGroup &grp) const
		{
			for (int i = 0; i<num; i++) {
				if (tex[i]<grp.tex[i]) return true;
			}
			return false;
		}
	};
}