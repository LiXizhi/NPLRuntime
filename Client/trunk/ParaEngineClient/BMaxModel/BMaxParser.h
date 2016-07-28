#pragma once
#include "BlockEngine/BlockCoordinate.h"
#include "math/ShapeAABB.h"
#include "BMaxFrameNode.h"
#include "ParaXModel/ParaXModel.h"

class TiXmlDocument;
class TiXmlElement;
class TiXmlNode;
typedef TiXmlDocument BMaxXMLDocument;
typedef TiXmlElement BMaxXMLElement;
typedef TiXmlNode BMaxXMLNode;

namespace ParaEngine
{
	class CParaXModel;
	class BlockModel;
	class Bone;
	class BMaxParser;
	struct ModelAnimation;
	struct ModelVertex;
	class BMaxAnimGenerator;

	/** Block Max file format parser. For *.bmax
	*/
	class BMaxParser
	{
	public:
		/** block id */
		enum BlockIDNum
		{
			// transparent block (cob web) will transmit bone weight, but will not be rendered. 
			TransparentBlockId = 118,
			// bones
			BoneBlockId = 253,
			// block model are extracted. 
			BlockModelBlockId = 254,
		};

		BMaxParser(const char* pBuffer, int32 nSize, const char* filename = NULL, BMaxParser* pParent = NULL);
		virtual ~BMaxParser(void);
		CParaXModel* ParseParaXModel();
		
		const std::string& GetFilename() const;
		void SetFilename(const std::string& val);
	protected:
		/** check if the given filename belongs to one of its parent's filename*/
		bool IsFileNameRecursiveLoaded(const std::string& filename);
		void AddAnimation(const ModelAnimation& anim);
		int GetAnimationsCount();
		BMaxParser* GetParent() const { return m_pParent; }
		void SetParent(BMaxParser* val) { m_pParent = val; }

		void SetAutoScale(bool value);
		bool IsAutoScale();
		const Vector3& GetCenterPos() const;
	
		inline uint32 GetNodeIndex(uint16 x, uint16 y, uint16 z)
		{
			return (DWORD)x + ((DWORD)z << 8) + ((DWORD)y << 16);
		}
		/* same as GetNode except that it checks for boundary condition. */
		BMaxNode* GetBMaxNode(int x, int y, int z);

		inline BMaxNode* GetNode(uint16 x, uint16 y, uint16 z)
		{
			int index = GetNodeIndex(x, y, z);
			auto iter = m_nodes.find(index);
			return (iter != m_nodes.end()) ? iter->second.get() : NULL;
		}
		inline BMaxNode* GetNodeByIndex(int index)
		{
			auto iter = m_nodes.find(index);
			return (iter != m_nodes.end()) ? iter->second.get() : NULL;
		}
		/** return node index*/
		int InsertNode(BMaxNodePtr& nodePtr);
		void Load(const char* pBuffer, int32 nSize);
		void ParseBlocks(BMaxXMLDocument& doc);
		void ParseBlocks_Internal(const char* value);
		
		void ParseVisibleBlocks();
		void ParseHead(BMaxXMLDocument& doc);
		// parse bones
		void ParseBlockFrames();

		void CreateDefaultAnimations();

		void AutoAddWalkAnimation(int nAnimID, int nStartTime, int nEndTime, float fMoveSpeed=4.f, bool bMoveForward = true);

		BMaxFrameNode* GetFrameNode(int16 x, int16 y, int16 z);

		// bone kins
		void CalculateBoneWeights();

		void CalculateBoneWeightFromNeighbours(BMaxNode* node);

		void CalculateBoneSkin(BMaxFrameNode* pBoneNode);
		/**
		* @param bMustBeSameColor: if true we will only add bone if color is the same as the bone. 
		*/
		void CalculateBoneWeightForBlock(BMaxFrameNode* pBoneNode, BMaxNode* node, bool bMustBeSameColor = true);
		/** find the first root bone index. In most cases it is 0.
		* the first bone without parent is the root bone
		*/
		int FindRootBoneIndex();
		// animations 
		void ParseBlockAnimations(BMaxXMLDocument& doc);
		void ParseBlockAnimationSet(BMaxXMLElement* node);
		void ParseBlockAnimation(BMaxXMLElement* node);
		void ParseBlockAnimationKeys(uint16 x, uint16 y, uint16 z, BMaxXMLElement* node, int nIndex);
		void ParseBlockAnimationKey(BMaxXMLElement* node,Bone* bone,const std::string propertyType);

		void FillParaXModelData(CParaXModel *pMesh);
		void FillVerticesAndIndices();
		void ProcessBoneNodes();
		int CreateBoneIndex(uint16 x, uint16 y, uint16 z, int parentIndex);
		int GetBoneIndex(uint16 x, uint16 y, uint16 z);
		void ScaleModels();
		float CalculateScale(float length);
		int GetIndexFromXmlElement(BMaxXMLElement* node, const char* name, int& x, int& y, int& z);

		bool ReadValue(BMaxXMLElement* node, const char* name, int32_t& value);
		bool ReadValue(BMaxXMLElement* node, const char* name, float& value);
		bool ReadValue(BMaxXMLElement* node, const char* name, Vector3& value);
		bool ReadValue(BMaxXMLElement* node, const char* name, std::string& value);
		bool ReadValue(BMaxXMLElement* node, const char* name, Vector4& value);

		/** how many times that a given name has appeared. */
		int GetNameAppearanceCount(const std::string& name, bool bAdd = true);

		/** filename is relative to current world directory. */
		CParaXModel* CreateGetRefModel(const std::string& sFilename);

		ModelGeoset* AddGeoset();
		ModelRenderPass* AddRenderPass();

		/* create or get the default animation generator */
		BMaxAnimGenerator* GetAnimGenerator();
	protected:
		BMaxParser* m_pParent;
		std::string m_filename;
		std::vector<BlockModel*> m_blockModels;
		std::map<uint32, BMaxNodePtr> m_nodes;
		std::map<std::string, ref_ptr<CParaXModel> > m_refModels;
		
		/// array of bone nodes, array index is the bone index. 
		vector<BMaxFrameNodePtr> m_bones;
		// how many times that a given name has appeared. 
		std::map<std::string, int> m_name_occurances;
		bool m_bAutoScale;
		float m_fScale;
		map<BlockModel*, BMaxNode*> m_blockModelsMapping;
		vector<ModelAnimation> m_anims;
		bool m_bHasAnimation;

		vector<ModelVertex> m_vertices;
		vector<uint16> m_indices;
		std::vector<ModelGeoset> m_geosets;
		std::vector<ModelRenderPass> m_renderPasses;
		/** aabb in block world coordinate */
		CShapeBox m_blockAABB;
		/** the center point in block world coordinate */
		Vector3 m_centerPos;
		// AABB in model space
		Vector3 m_minExtent;
		Vector3 m_maxExtent;

		/** the block id used to extend AABB. */
		int m_nHelperBlockId;
		
		BMaxAnimGenerator* m_pAnimGenerator;

		static const int  MaxBoneLengthHorizontal;
		static const int  MaxBoneLengthVertical;
		friend struct BMaxNode;
		friend struct BMaxFrameNode;
		friend struct BMaxBlockModelNode;
		friend class BMaxAnimGenerator;
	};
}