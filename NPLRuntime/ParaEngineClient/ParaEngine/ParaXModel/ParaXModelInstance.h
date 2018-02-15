#pragma once

namespace ParaEngine
{
	/** ParaXModel instance 
	* all instanced data should go to this class. 
	*/
	class ParaXModelInstance
	{
	public:
		class BoneInstance
		{
			Matrix4 mat;
			Matrix4 mrot;
		};
		vector<BoneInstance> m_bones;

		std::map<int, bool> m_showGeosets;

		std::map<int, int> m_specialTextures;
		std::map<int, TextureEntity*>  m_replaceTextures;
		std::map<int, bool> m_useReplaceTextures;

		/** current animation index, this is different from sequence ID 
		* an absolute ParaX frame number denoting the current animation frame. It is always within
		* the range of the current animation sequence's start and end frame number. */
		AnimIndex m_CurrentAnim;

		/** the next animation index.if it is -1, the next animation will 
		* depends on the loop property of the current sequenc; otherwise, the animation specified
		* by the ID will be played next, after the current animation reached the end. */
		AnimIndex m_NextAnim;

		/** the animation sequence with which the current animation should be blended.
		* an absolute ParaX frame number denoting the blending animation frame. It is always within
		* the range of the blending animation sequence's start and end frame number. */
		AnimIndex m_BlendingAnim;

		/** by how much the blending frame should be blended with the current frame. 
		* 1.0 will use solely the blending frame, whereas 0.0 will use only the current frame.
		* [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame */
		float m_blendingFactor;

		/// blending time in seconds
		float m_fBlendingTime;
	};
}

