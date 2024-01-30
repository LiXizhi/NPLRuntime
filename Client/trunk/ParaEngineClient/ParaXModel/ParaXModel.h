#pragma once
#include "IAttributeFields.h"
#include "modelheaders.h"
#include "animated.h"
#include "AnimInstanceBase.h"
#include "ParameterBlock.h"
#include "ParaXModelInstance.h"
#include "TextureAnim.h"
#include "ParaXModelCommon.h"
#include "ModelRenderPass.h"

namespace ParaEngine
{
	class Bone;
	class ParticleSystem;
	class RibbonEmitter;
	class CParaXModel;
	struct TextureEntity;
	class CEffectFile;
	struct CharacterPose;
	class ShadowVolume;
	struct LightParams;
	class CFaceGroup;
	struct ModelAttachment;
	class ParaVoxelModel;

	/**
	* ParaX model is the model file for character animation, etc in the game world.
	*/
	class CParaXModel : public IAttributeFields
	{
	public:
		CParaXModel(const ParaXHeaderDef& xheader);
		virtual ~CParaXModel(void);
		void SetHeader(const ParaXHeaderDef& xheader);

		/** in what method to render the mesh */
		enum RENDER_METHOD {
			/** using software skinning */
			SOFT_ANIM = 0,
			/** using hardware skinning, need vertex shader 1.1 or later */
			SHADER_ANIM,
			/** render without animation. It is just a solid mesh. */
			NO_ANIM,
			/** BMAX model color model.*/
			BMAX_MODEL,
		};
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CParaXModel; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CParaXModel"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char* sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CParaXModel, DumpTextureUsage_s, const char**) { *p1 = cls->DumpTextureUsage(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetPolyCount_s, int*) { *p1 = cls->GetPolyCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetPhysicsCount_s, int*) { *p1 = cls->GetPhysicsCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetGeosetsCount_s, int*) { *p1 = (int)cls->geosets.size(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetRenderPassesCount_s, int*) { *p1 = (int)cls->passes.size(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetObjectNum_s, void**) { *p1 = (void*)(&(cls->GetObjectNum())); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetVertices_s, void**) { *p1 = (void*)(cls->m_origVertices); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetRenderPasses_s, void**) { *p1 = (void*)(&(cls->passes[0])); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetGeosets_s, void**) { *p1 = (void*)(&(cls->geosets[0])); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetIndices_s, void**) { *p1 = (void*)(&(cls->m_indices[0])); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetAnimations_s, void**) { *p1 = (void*)(cls->anims); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, SaveToDisk_s, char*) { cls->SaveToDisk(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, SaveToGltf_s, char*) { cls->SaveToGltf(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXModel, GetStrAnimIds_s, const char**) { *p1 = cls->GetStrAnimIds(); return S_OK; }
	public:
		/** get polycount of this mesh object */
		int GetPolyCount();
		/** get physics polycount of this mesh object */
		int GetPhysicsCount();
		/** get texture usage such as the number of textures and their sizes. */
		const char* DumpTextureUsage();

		/** get the number of objects in the model.*/
		inline const ParaXModelObjNum& GetObjectNum() const { return m_objNum; };
		inline const ParaXHeaderDef& GetHeader() const { return m_header; };
		/** check the minimum file version. Return true if file version is equal or higher than the given one. */
		bool CheckMinVersion(int v0, int v1 = 0, int v2 = 0, int v3 = 0);

		/** file loading for ParaX file type*/
		void initVertices(int nVertices, ModelVertex* pVertices);
		void initIndices(int nIndices, uint16* pIndices);


		/** call this function only once after the model data is loaded. It will search for translucent pass
		* and build face groups to store them for later sorting and rendering.
		*/
		void initTranslucentFaceGroups();

		/** create new attachment */
		ModelAttachment& NewAttachment(bool bOverwrite, int nAttachmentID, int nBoneIndex, const Vector3& pivotPoint);

		/** whether this is a valid model */
		bool IsValid();

		/** TODO: currently this is not implemented*/
		bool InitDeviceObjects();
		/** TODO: currently this is not implemented*/
		bool DeleteDeviceObjects();

		/** set the transform matrix for the specified attachment ID on this model.
		* it is assumed that the transform of this model has already been set in the render device
		* hence it will just multiple over the one in the device.
		* return true, if the nID is a valid attachment point on the model.
		*/
		bool SetupTransformByID(int nID);

		void drawModel(SceneState* pSceneState, CParameterBlock* pMaterialParam = NULL, int nRenderMethod = -1);

		/** animate the entire model according to the current animation and blending settings.
		* @param pPose: it will override the poses defined in the model.:*/
		void animate(SceneState* pSceneState, CharacterPose* pPose, IAttributeFields* pAnimInstance = NULL);

		/** render the model, according to the current animation states. make sure to call animate before calling this functions.*/
		void draw(SceneState* pSceneState, CParameterBlock* materialParams = NULL, int nRenderMethod = -1);

		/** Build Shadow Volume*/
		void BuildShadowVolume(ShadowVolume* pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

		void updateEmitters(SceneState* pSceneState, float dt);
		void drawBones();
		void drawBoundingVolume();

		/**
		* calc all Bones in the model with motion blending with a specified blending frame.
		* @param pPose: it will override the poses defined in the model.
		* @param nCurrentAnim: current animation sequence ID
		* @param currentFrame: an absolute ParaX frame number denoting the current animation frame. It is always within
		* the range of the current animation sequence's start and end frame number.
		* @param nBlendingAnim: the animation sequence with which the current animation should be blended.
		* @param blendingFrame: an absolute ParaX frame number denoting the blending animation frame. It is always within
		* the range of the blending animation sequence's start and end frame number.
		* @param blendingFactor: by how much the blending frame should be blended with the current frame.
		* 1.0 will use solely the blending frame, whereas 0.0 will use only the current frame.
		* [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame
		*/
		void calcBones(CharacterPose* pPose, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, const AnimIndex& upperAnim, const AnimIndex& upperBlendingAnim, float upperBlendingFactor, IAttributeFields* pAnimInstance = NULL);

		/** for model without animations, but with bones.*/
		void calcBones();

		void PostCalculateBoneMatrix(uint32 nBones);

		/** calculate only specified bone in the attachment, all parent bones will also be calculated in order to get the matrix for the specified bone.
		* @param pOut: the bone transform matrix.
		* @param nAttachmentID: the bone index.
		* @param bRecalcBone: whether we will recalculate bone chains according to the current animation pose.
		* @return: NULL if not successful, otherwise it is pOut.
		*/
		Matrix4* GetAttachmentMatrix(Matrix4* pOut, int nAttachmentID, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, const AnimIndex& upperAnim, const AnimIndex& upperBlendingAnim, float upperBlendingFactor, bool bRecalcBone = true, IAttributeFields* pAnimInstance = NULL);

		/** whether we have attachment matrix.
		* @param nAttachmentID: the attachment id.
		*/
		bool HasAttachmentMatrix(int nAttachmentID);

		/**
		* @param nIndex: the index into the global indices.
		*/
		inline Vector3 GetWeightedVertexByIndex(unsigned short nIndex);
		void lightsOn(uint32 lbase);
		void lightsOff(uint32 lbase);
		/** return true if the model has animation.*/
		bool HasAnimation();

		/** whether it is bmax model with vertex color. */
		bool IsBmaxModel();
		void SetBmaxModel();

		/** load textures */
		void LoadTextures();

		/** return the animation index by animation ID.
		* the animation <ID, name> pairs are stored in the AnimDB table.
		* if the animation ID is not found, 0 (usually the stand animation) is returned.
		* a sequential search is used for the specified animation. Below is some common mappings.
		* <0, stand> <1, death> <2, spell> <3, stop> <4, walk> <5, run>
		* @retrun: return index of the animation. If the animation is not found, then -1 is returned.
		*/
		AnimIndex GetAnimIndexByID(int nAnimID);
		/** return the ID of the animation at the specified index.
		* if there is no animation at the index, 0 (default animation ID) is returned.
		*/
		int GetAnimIDByIndex(int nAnimIndex);

		/** return NULL if not exist */
		const ModelAnimation* GetModelAnimByIndex(int nAnimIndex);

		/** get model animations as string */
		const char* GetStrAnimIds();

		/** get the mesh radius */
		float GetBoundingRadius();
		/** whether the model has a specified attachment point */
		bool canAttach(int id);

		int GetRenderPass(CParameterBlock* pMaterialParams = NULL);
		void RenderNoAnim(SceneState* pSceneState);
		void RenderSoftAnim(SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void RenderSoftNoAnim(SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void RenderShaderAnim(SceneState* pSceneState);
		void RenderBMaxModel(SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		/** only called inside Render* function*/
		void DrawPass(ModelRenderPass& p);
		void DrawPass_NoAnim(ModelRenderPass& p);
		void DrawPass_NoAnim_VB(ModelRenderPass& p, size_t start);
		void DrawPass_BMax_VB(ModelRenderPass& p, size_t start);
		void DrawPass_BMax(ModelRenderPass& p);
		/** clear all face groups. */
		void ClearFaceGroups();
		bool HasAlphaBlendedObjects();

		/** return the physics group id that is closest to nPhysicsGroup. or -1 if there is none. */
		int GetNextPhysicsGroupID(int nPhysicsGroup = -1);

		/**
		* Get the physics mesh in terms of vertices and indices.
		* @param pNumVertices [out] number of vertices
		* @param ppVerts [out] buffer contains all vertices. The caller needs to release the buffer using delete [] buffer.
		* @param pNumTriangles [out] number of triangles, each triangle has three indices.
		*	Please note that if the mesh contains no physics faces, the pNumTriangles is 0. However pNumVertices might be positive.
		* @param ppIndices [out] buffer contains all indices. The caller needs to release the buffer using delete [] buffer.
		* @param nMeshPhysicsGroup [in|out]: the mesh physics group to get. On return it will be assigned with the next mesh group.
		* @return S_OK, if succeed.
		*/
		HRESULT ClonePhysicsMesh(DWORD* pNumVertices, Vector3** ppVerts, DWORD* pNumTriangles, DWORD** ppIndices, int* pnMeshPhysicsGroup = NULL, int* pnTotalMeshGroupCount = NULL);

		void SaveToDisk(const char* path);

		// export ParaX model to gltf(glb) file
		void SaveToGltf(const char* path);
		/** call this before save to disk to compress the size of the file, if it is loaded from FBX files */
		void RemoveUnusedAnimKeys();

		void SetRenderMethod(RENDER_METHOD method);

		void SetVertexBufferDirty();

		HRESULT RendererRecreated();

		friend struct ModelRenderPass;

	private:
		void InitVertexBuffer_NOANIM();
		void InitVertexBuffer_BMAX();
		void InitVertexBuffer();

	public:
		/** model header */
		ParaXHeaderDef	m_header;
		/** object counts*/
		ParaXModelObjNum m_objNum;
		/// bounding radius
		float m_radius;
		/** -1 uninitialized. 0 false, 1 true. */
		int m_nHasAlphaBlendedRenderPass;

		RENDER_METHOD m_RenderMethod;

		/** max number of textures per model*/
		const static int MAX_MODEL_TEXTURES = 32;
		/** max number of attachment points per model*/
		const static int MAX_MODEL_ATTACHMENTS = 40;

		/** animation info for the current model*/
		ModelAnimation* anims;

		/**	The effect file entity used to render the animated model */
		static CEffectFile* m_pEffectFile;
		/** vertex declaration used in the vertex shader of the effect file */
		static VertexDeclarationPtr m_pVertexDeclaration;
		/** vertex buffer for skinned vertex*/
		ParaVertexBuffer m_pVertexBuffer;
		/** index buffer*/
		ParaIndexBuffer m_pIndexBuffer;

		size_t vbufsize;
		bool animated : 1;
		bool animGeometry : 1, animTextures : 1, animBones : 1, animTexRGB : 1;
		bool rotatePartice2SpeedVector : 1;
		bool forceAnim : 1;

		TextureAnim* texanims;
		int* globalSequences;
		ModelColor* colors;
		ModelTransparency* transparency;
		ModelLight* lights;
		ParticleSystem* particleSystems;
		RibbonEmitter* ribbons;

		ModelVertex* m_origVertices;
		/** this always contains the same number of items as m_origVertices.
		* it stores the frame number that a certain vertices are used. This avoids the same vertex to be calculated multiple times in a single animation pose.
		*/
		int* m_frame_number_vertices;
		int m_nCurrentFrameNumber;
		Vector3* m_vertices, * m_normals;// the position and normals for the vertices
		Vector2* texcoords1;// the texture coordinates for the vertices
		uint16* m_indices;

		Vector3* bounds;
		uint16* boundTris;

		ModelCamera cam;
		Bone* bones;
		asset_ptr<TextureEntity>* textures;
		ParaVoxelModel* m_pVoxelModel;

		std::vector<ModelRenderPass> passes;
		/** it has the same number of element as the passes. the int value denotes the index into the m_faceGroups*/
		std::vector<int> m_TranslucentPassIndice;
		/** translucent face groups. */
		std::vector<CFaceGroup*>  m_faceGroups;

		std::vector<ModelGeoset> geosets;
		/** a list of face group */

		bool* showGeosets;

		int specialTextures[MAX_MODEL_TEXTURES];
		TextureEntity* replaceTextures[MAX_MODEL_TEXTURES];
		bool useReplaceTextures[MAX_MODEL_TEXTURES];

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
		float blendingFactor;

		/// blending time in seconds
		float fBlendingTime;

		bool m_bIsValid;
		bool hasCamera;
		bool m_bTextureLoaded;

		float m_trans;
		std::vector<ModelAttachment> m_atts;
		int32 m_attLookup[MAX_MODEL_ATTACHMENTS];

		/** default to unitY */
		Vector3 m_vNeckYawAxis;
		/** default to unitZ */
		Vector3 m_vNeckPitchAxis;

		/** a mapping from known bone id to bone index. */
		int m_boneLookup[MAX_KNOWN_BONE_NODE];

		AnimIndex mUpperAnim;
		AnimIndex mUpperBlendingAnim;
		float mUpperBlendingFactor;

	private:
		enum VERTEX_BUFFER_STATE
		{
			NOT_SET = 0,
			NEED_INIT = 1,
			INITED = 2,
			NOT_USE = 3,
		};

		VERTEX_BUFFER_STATE m_vbState;

		static const size_t MAX_USE_VERTEX_BUFFER_SIZE = 1024 * 1024 * 256;
		static size_t m_uUsedVB;
	};

	/** the pose of the character. It will override the one in the model.*/
	struct CharacterPose
	{
		float m_fUpperBodyFacingAngle;
		float m_fUpperBodyUpDownAngle;
	};

	typedef asset_ptr<CParaXModel> CParaXModelPtr;
}
