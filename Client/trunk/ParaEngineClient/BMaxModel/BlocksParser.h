#pragma once
#include "BMaxParser.h"

namespace ParaEngine
{
	class BlocksParser : public BMaxParser
	{
	public:
		BlocksParser();
		virtual ~BlocksParser(void);

		void Load(const char* blocks);
		CParaXModel* ParseParaXModel();

	protected:
		void ParseBlocks_Internal(const char* value);
		void ParseVisibleBlocks();
		void MergeCoplanerBlockFace();
		void FindCoplanerFace(BMaxNode* node, uint32 nFaceIndex);
		void FillParaXModelData(CParaXModel *pMesh, int32 nMaxTriangleCount = -1);
		void FillVerticesAndIndices(int32 nMaxTriangleCount = -1);
		
	private:
		std::map<uint16_t, std::vector<RectanglePtr>> blockRectangles;
		std::vector<TextureEntity*> m_textures;
	};
}