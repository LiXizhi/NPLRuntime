#pragma once
#include "BMaxNode.h"

namespace ParaEngine
{
	class Bone;

	/** block model node in bmax */
	struct BMaxGlassModelNode : public BMaxNode
	{
	public:
		BMaxGlassModelNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_);
		virtual ~BMaxGlassModelNode();
	public:
		virtual int TessellateBlock(BlockModel* tessellatedModel);
	};
	typedef ref_ptr<BMaxGlassModelNode> BMaxGlassModelNodePtr;
}