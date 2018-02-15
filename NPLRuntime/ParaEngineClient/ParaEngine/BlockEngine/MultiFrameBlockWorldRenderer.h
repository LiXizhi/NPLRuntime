#pragma once
#include "IAttributeFields.h"
#include "math/ShapeSphere.h"
#include "BaseCamera.h"

namespace ParaEngine
{
	class BlockWorldClient;
	class CRenderTarget;
	class RenderableChunk;
	class BlockRenderTask;

	/** this is used for rendering a super large block world across several frames. 
	* because chunk buffer of the world usually can not be held all in video memory. 
	* In general, this renderer is only used to generate large preview image of the world offline. 
	* However, it is also possible to use this as environment map or right after the sky box. 
	* A new image will be generated when the camera changed. 
	*/
	class CMultiFrameBlockWorldRenderer : public IAttributeFields
	{
	public:
		CMultiFrameBlockWorldRenderer(BlockWorldClient* pWorld);
		~CMultiFrameBlockWorldRenderer();
		ATTRIBUTE_DEFINE_CLASS(CMultiFrameBlockWorldRenderer);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, IsEnabled_s, bool*)	{ *p1 = cls->IsEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, SetEnabled_s, bool)	{ cls->SetEnabled(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, IsDirty_s, bool*)	{ *p1 = cls->IsDirty(); return S_OK; }
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, SetDirty_s, bool)	{ cls->SetDirty(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, IsUseMyCamera_s, bool*)	{ *p1 = cls->IsUseMyCamera(); return S_OK; }
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, SetUseMyCamera_s, bool)	{ cls->SetUseMyCamera(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, IsDebugImage_s, bool*)	{ *p1 = cls->IsDebugImage(); return S_OK; }
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, SetDebugImage_s, bool)	{ cls->SetDebugImage(p1); return S_OK; }


		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, IsFinished_s, bool*)	{ *p1 = cls->IsFinished(); return S_OK; }
		

		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, GetMaxChunksToDrawPerTick_s, int*)	{ *p1 = cls->GetMaxChunksToDrawPerTick(); return S_OK; }
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, SetMaxChunksToDrawPerTick_s, int)	{ cls->SetMaxChunksToDrawPerTick(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, GetRenderDistance_s, int*)	{ *p1 = cls->GetRenderDistance(); return S_OK; }
		ATTRIBUTE_METHOD1(CMultiFrameBlockWorldRenderer, SetRenderDistance_s, int)	{ cls->SetRenderDistance(p1); return S_OK; }
		
	public:
		/** draw some chunks if not finished. 
		* @param nMaxChunks: how many chunks to draw. if -1 (default), we will use m_nMaxChunksToDrawPerTick
		* @return true if finished. 
		*/
		bool Draw(int nMaxChunks = -1);

		/** render the image to the screen, with z test enabled and z-write disabled. As if drawing on the skybox. */
		bool DrawToSkybox();

		/** show the image on left top corner for debugging */
		bool IsDebugImage() const;
		void SetDebugImage(bool val);

		/** whether multi frame rendering is enabled. */
		bool IsEnabled() const;
		void SetEnabled(bool val);

		/** default to false. if true, we will use a separate camera other than the global scene's camera.
		* if false, we will use copy scene camera to my camera whenever it moves.
		*/
		bool IsUseMyCamera() const;
		void SetUseMyCamera(bool val);

		/** max number of chunks to draw per tick*/
		int GetMaxChunksToDrawPerTick() const;
		void SetMaxChunksToDrawPerTick(int val);

		void CheckCreateActiveChunks();

		/** whether render the image to sky box. true by default. */
		bool IsDrawToSkybox() const;
		void SetDrawToSkybox(bool val);

		/** this is true if camera has moved. and we need to start a repaint.
		Please note even it is not dirty, it does not mean the image is finished.*/
		bool IsDirty() const;
		void SetDirty(bool val);

		bool IsFinished() const;
		void SetFinished(bool val);

		/** max number of blocks to render from eye to farthest one*/
		int GetRenderDistance() const;
		void SetRenderDistance(int val);

		void DeleteDeviceObjects();
		void RendererRecreated();

		/** get the camera object. create if the camera does not exist.*/
		CBaseCamera* GetCamera();

	protected:
		/** a render target will be created */
		virtual CRenderTarget* CreateGetRenderTarget(bool bCreateIfNotExist = true);

		bool DrawInternal(int nMaxChunks);

		void DrawRenderTasks(BlockRenderPass nCurRenderPass);

		/**
		* @param pbFromMainBuffer: output a boolean. if return it means that the returned chunk is from the main renderer buffer. 
		* if false, it means that it is from this class's own buffer. 
		*/
		RenderableChunk* GetRenderableChunkByPosition(const Int16x3& chunkPos, int nBufferIndex, bool* pbFromMainBuffer);

		/** get chunk column position according to render progress.
		* @param nProgress: the render progress. 
		* @param outChunkPos: output chunk position. 
		* @return true if chunk position is found. if false, it means we have finished all chunks. 
		*/
		bool GetChunkByProgress(int nProgress, Int16x3& outChunkPos, bool bIsResuming = false);

		void ResetProgress();

		/** calculate camera related parameters for used in later multi-frame rendering. */
		void UpdateViewParams();


		void SetEyeBlockId(const Uint16x3& eyePos);
		const Uint16x3& GetEyeBlockId();

		const Uint16x3& GetEyeChunkId();
	protected:
		BlockWorldClient* m_pWorld;

		/** the render target that stores the final image. */
		WeakPtr m_renderTarget;

		/** max number of blocks to render from eye to farthest one*/
		int m_nRenderDistance;

		/** max number of chunks to draw per tick*/
		int m_nMaxChunksToDrawPerTick;

		/** this is true if camera has moved. and we need to start a repaint. 
		Please note even it is not dirty, it does not mean the image is finished.*/
		bool m_bIsDirty;

		/** if the image is ready for use. Please note that one can still use the image even if it is not ready 
		for example to show in-progress result to the user. */
		bool m_bIsFinished;

		/** whether multi frame rendering is enabled. */
		bool m_bEnabled;

		/** whether render the image to sky box. true by default. */
		bool m_bDrawToSkybox;

		/** get the current progress*/
		int m_nProgress;
		/** get the current render pass */
		int m_nCurRenderPass;
				
		/** renderable chunks that is rebuilt each frame with some new data. */
		std::vector<RenderableChunk*> m_activeChunks;

		/** only used temporarily when rendering blocks*/
		std::vector<BlockRenderTask*> m_tempRenderTasks;

		/** pending render task. Because RenderableChunk is not valid across frames, we can only cache their chunk locations 
		* Array of chunk position
		*/
		std::vector<Int16x3> m_alphaTestTasks;
		std::vector<Int16x3> m_alphaBlendTasks;
		std::vector<Int16x3> m_reflectedWaterTasks;

		//
		// camera related parameters
		//
		CBaseCamera* m_pCamera;
		Uint16x3 m_curCamChunkId;
		Uint16x3 m_curCamBlockId;
		CShapeSphere m_sEyeSphere;
		int32 m_chunkViewSize;
		CCameraFrustum m_frustum;
		Uint16x3 m_startIdx, m_endIdx;
		Vector3 m_renderOrig;
		/** default to false. if true, we will use a separate camera other than the global scene's camera.
		* if false, we will use copy scene camera to my camera whenever it moves. 
		*/
		bool m_bUseMyCamera;

		/** show the image on left top corner for debugging */
		bool m_bDebugImage;

	private:
		int32 m_nIndex;
		int32 m_dx;
		int32 m_dz;
		int32 m_length;
		int32 m_k;
		int32 m_i;
		int m_nProgressIndex;
		bool m_bCanResumeProgress;
	};
}

