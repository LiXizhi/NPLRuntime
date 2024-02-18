#pragma once
#include "BlockCommon.h"
#include "BlockModel.h"
#include "ShapeAABB.h"

namespace ParaEngine
{
	class CBlockWorld;
	class BlockTemplate;
	class IBlockModelProvider;
	class Block;
	class CShapeAABB;

	/** block template base class. */
	class BlockTemplate
	{
	public:
		static const uint16_t g_maxRenderPriority;
		enum BlockAttrubiteFlag
		{
			batt_obstruction = 0x0000001,		//physical obstruction
			batt_breakable = 0x0000002,
			batt_solid = 0x0000004,		//light obstruction
			batt_liquid = 0x0000008,
			batt_blendedTexture = 0x0000010,
			batt_light = 0x0000020,
			batt_cubeModel = 0x0000040,
			batt_customModel = 0x0000080,

			batt_transparent = 0x0000100,
			batt_twoTexture = 0x0000200,		//two texture blend together
			batt_alphaTestTexture = 0x0000400,		//use alpha test

			batt_singleSideTex = 0x0000800,
			batt_threeSideTex = 0x0001000,
			batt_fourSideTex = 0x0002000,
			batt_sixSideTex = 0x0004000,
			batt_climbable = 0x0008000,		// whether a biped can climb up/down vertically using this block. 
			batt_blockcamera = 0x0010000,	// whether the block will block camera line of sight. 

			batt_framemove = 0x0020000,	// whether the block has a frame move function.
			batt_onload = 0x0040000,	// whether the block has a OnBlockLoaded function.
			batt_color_data = 0x0080000,	// whether the block contains color in its block data.
			batt_invisible = 0x0100000,// whether the block is invisible.
			batt_random_tiling = 0x0200000, // whether the block's texture is randomly fetched from its tiled texture
			batt_color8_data = 0x0400000,	// whether the block uses only the high 8 bits as color in its block data.
			batt_pos_tiling = 0x0800000,	// whether the block's texture is fetched from its tiled texture according to its coords
		};

		BlockTemplate(uint16_t id, uint32_t attFlag, uint16_t category_id);

		~BlockTemplate();

		void Init(uint32_t attFlag, uint16_t category_id);

		inline uint16_t GetID() const
		{
			return m_id;
		}

		inline uint16_t GetCategoryID() const
		{
			return m_category_id;
		}

		inline uint32_t GetAttFlag()  const
		{
			return m_attFlag;
		}

		/** if match any of the given attributes */
		inline bool IsMatchAttribute(uint32_t attFlags) const
		{
			return ((m_attFlag & attFlags) > 0);
		}

		/** all attributes as specified in dwMask must match the value of attFlags*/
		inline bool IsMatchAttributes(uint32_t dwMask, uint32_t attFlags) const
		{
			return ((m_attFlag & dwMask) == attFlags);
		}

		/** if match all of the given attributes */
		inline bool IsMatchAttributeAll(uint32_t attFlags) const
		{
			return ((m_attFlag & attFlags) == attFlags);
		}


		/** whether the block contains color data.
		* whether it is a cube block without texture and whose data field contains color index.
		* we will use a single pass to draw all blocks of different colors.
		*/
		inline bool HasColorData() const
		{
			return ((m_attFlag & batt_color_data) != 0);
		}

		/* by default, color uses the full 16bits block data. in some cases, It only use the high 8bits of the user data,
		* so that the low 8bits can still be used to store other block specific data. */
		inline bool IsColorData8Bits() const
		{
			return ((m_attFlag & batt_color8_data) != 0);
		}

		/**
		* enable or disable a given attribute.
		* @param dwAtt
		* @param bTurnOn: true to turn on, false to turn off.
		*/
		void SetAttribute(DWORD dwAtt, bool bTurnOn = true);

		/** torch default to 15*/
		inline uint8_t GetTorchLight()
		{
			return m_nLightValue;
		}

		/** get light value. */
		inline uint8_t GetLightValue() {
			return m_nLightValue;
		};

		/** set torch light value */
		void SetTorchLight(uint8_t value);

		/** whether it is shadow caster. most blocks default to true, except for water blocks.  */
		inline bool IsShadowCaster() { return m_bIsShadowCaster; };

		void SetTexture0(const char* texName, int nIndex = 0);

		void SetTexture1(const char* texName);

		void SetNormalMap(const char* texName);

		TextureEntity* GetTexture0(uint32 nUserData = 0);

		TextureEntity* GetTexture1();

		inline TextureEntity* GetNormalMap()
		{
			return m_pNormalMap;
		}

		inline uint16_t GetRenderPriority()
		{
			return m_renderPriority;
		}

		/** load from known model name */
		void LoadModel(const std::string& sModelName);

		/** set associated block */
		void SetAssociatedBlock(uint16_t associated_blockid);

		/** get the default block model.
		* @param nIndex: model index default to 0
		*/
		BlockModel& GetBlockModel(int nIndex = 0);
		BlockModel& CreateGetBlockModel(int nIndex = 0);

		BlockModel& GetBlockModelByData(uint32 nData);

		/** get the face shape on the given side
		*/
		int GetFaceShape(int nSide, int32_t nData);

		int GetFaceShapeDirect(int nSide, int32_t nData);

		/** get model by block position.
		* @param neighborBlocks: 27 neighbor blocks
		*/
		BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);

		/** get AABB in local space */
		void GetAABB(CBlockWorld* pBlockManager, uint16_t bx, uint16_t by, uint16_t bz, CShapeAABB* pOutAABB);

		/** get bounding box vertices */
		void GetBoundingBoxVertices(Vector3 * pVertices, int* pNumber);

		/** whether associated */
		bool IsAssociatedBlockID(uint16_t block_id);

		/**
		* Can this block provide power. Only wire currently seems to have this change based on its state.
		*/
		inline bool canProvidePower() { return m_bProvidePower; }

		void setProvidePower(bool bValue);

		bool isBlockNormalCube();
		bool isSolidBlock();

		/** physical height of this block. for non-obstruction block, this will always return -1.f.
		for obstruction block, this usually returns 1.0f. for slabs, it will return 0.5. for lily pad, it will return 0. */
		float GetPhysicalHeight();
		float GetPhysicalHeight(CBlockWorld* pBlockManager, uint16_t bx, uint16_t by, uint16_t bz);
		void SetPhysicalHeight(float fHeight);

		//transparent object will always be renderer after solid object.
		//default value are 0 for all templates
		//@param: priority range in [0,15] higher value get higher priority
		inline void SetRenderPriority(uint16_t priority)
		{
			if (priority > g_maxRenderPriority)
				priority = g_maxRenderPriority;
			m_renderPriority = priority;
		}

		/** how many light to scatter between neighbor blocks.
		*/
		inline int32_t GetLightScatterStep() {
			return m_lightScatterStep;
		}

		/* value between [1,15].  1 for air which is most transparent one.
		* 15 for solid blocks.
		*/
		inline int32_t GetLightOpacity() {
			return m_lightOpacity;
		}
		void SetLightOpacity(int32 nValue);


		/** solid block without alpha blending is fully opaque*/
		inline bool IsFullyOpaque() {
			return m_lightOpacity >= 15;
		}

		/** if true, it is not a fully transparent block.*/
		inline bool HasOpacity() {
			return m_lightOpacity > 1;
		}

		/** if true, it is fully transparent block.*/
		inline bool IsTransparent() {
			return m_lightOpacity <= 1;
		}

		/** whether the block model contains no alpha tested texture. Since we will use a fast shader and early Z for such block rendering. */
		inline bool IsAlphaTestTexture() {
			return IsMatchAttribute(batt_alphaTestTexture);
		}

		inline bool IsAlphaBlendedTexture() {
			return IsMatchAttribute(batt_blendedTexture);
		};

		inline bool IsTransparentModel() {
			return IsMatchAttribute(batt_transparent);
		};


		/** [0,1] when player is walking into this block, this is the speed reduction. default to 1.0, which is no speed reduction. */
		float GetSpeedReductionPercent() const;

		/** [0,1] when player is walking into this block, this is the speed reduction. default to 1.0, which is no speed reduction. */
		void SetSpeedReductionPercent(float val);

		/** make this model provider as linear for custom models.*/
		void MakeCustomLinearModelProvider(int nModelCount);

		inline BlockRenderPass GetRenderPass() const { return m_renderPass; }
		void SetRenderPass(BlockRenderPass val) { m_renderPass = val; }

		/** a single color as shown on the mini-map. It is usually the average color of the block's texture.
		* This color is also used in BMax Model
		*/
		Color GetMapColor() const;
		void SetMapColor(Color val);
		DWORD GetBlockColor(int32_t blockData);
		DWORD GetDiffuseColor(int32_t blockData);
		void setUnderWaterColor(const Color & val);
		const Color & getUnderWaterColor()const;
		int getTileSize()const;
		void setTileSize(int nTile);
		inline const std::string& GetModelName() {
			return modelName;
		}
		void SetPhysicsProperty(const char* property) { m_physics_property = property; }
		std::string& GetPhysicsProperty() { return m_physics_property; }

	private:
		/** unique id */
		uint16_t m_id;
		/** category id is non-unique. it usually match to mc id. */
		uint16_t m_category_id;
		/** associated block is considered the same block. such as water and still_water. door, open_door, etc. */
		uint16_t m_associated_blockid;

		uint16_t m_renderPriority;
		/** 0 for opaque, 1 for alpha tested, 2 for alpha blended. */
		BlockRenderPass m_renderPass;

		uint32_t m_attFlag;

		/** default to 1, for liquid this is 3*/
		int32_t m_lightScatterStep;
		/** [1,15]. 1 for air. 2 for water and ice, 15 for solid block. */
		int32 m_lightOpacity;

		uint32_t m_value;

		float m_fPhysicalHeight;
		/** [0,1] when player is walking into this block, this is the speed reduction. default to 1.0, which is no speed reduction. */
		float m_fSpeedReductionPercent;

		bool m_bIsShadowCaster;
		bool m_bProvidePower;

		std::vector< asset_ptr<TextureEntity> > m_textures0;
		std::vector< asset_ptr<TextureEntity> > m_textures1;
		TextureEntity*  m_pNormalMap;
		uint8_t m_nLightValue;
		std::string m_secondTexName;
		std::string m_normalMapName;
		std::vector<BlockModel> m_block_models;
		IBlockModelProvider* m_pBlockModelFilter;
		/** color as shown on the map*/
		Color m_dwMapColor;
		Color m_UnderWaterColor;

		int m_nTileSize;
		std::string modelName;
		friend class IBlockModelProvider;
		std::string m_physics_property;
	};
}
