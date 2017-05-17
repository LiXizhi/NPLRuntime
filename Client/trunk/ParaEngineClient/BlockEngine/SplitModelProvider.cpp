/**
深圳市爱致科技有限公司
@version 1.0.0 
@data 2017.4.1
*/
#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockRegion.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "SplitModelProvider.h"
#include "ParaEngine.h"
#include "IO/ParaFile.h"

#include "2dengine/GUIRoot.h"
#include "SceneObject.h"
#include "BaseCamera.h"
#include "ViewportManager.h"

namespace ParaEngine
{
    //-----------------------------------------------------
	CSplitModelProvider::CSplitModelProvider(BlockTemplate* pBlockTemplate):
        IBlockModelProvider(pBlockTemplate)
    {
        mFrameModel.LoadModelByTexture(1);
    }
	//-----------------------------------------------------
	CSplitModelProvider::~CSplitModelProvider()
	{
	}
	//-----------------------------------------------------
	BlockModel & CSplitModelProvider::GetBlockModel(int nIndex)
	{
		return mFrameModel;
	}
    //-----------------------------------------------------
	BlockModel & CSplitModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData, Block** neighborBlocks)
    {
        return mFrameModel;
    }
	//-----------------------------------------------------
	BlockModel & CSplitModelProvider::GetBlockModelByData(uint32 nData)
	{
		return mFrameModel;
	}
    //-----------------------------------------------------
    int CSplitModelProvider::GetModelIndex(CBlockWorld * pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData, Block** neighborBlocks)
    {
        return 0;
    }
    //-----------------------------------------------------
    bool CSplitModelProvider::isComBlock() const 
    {
        return true;
    }
    //-----------------------------------------------------
	void CSplitModelProvider::splitCom(Block * src, const std::string & level)
	{
		assert(src);
        SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if(stemp->childs[num])
				stemp = stemp->childs[num];
			else
				return;
		}
		assert(stemp);
		splitLevel(stemp);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::mergeCom(Block * src, const std::string & level)
	{
		assert(src);
		SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (stemp->childs[num])
				stemp = stemp->childs[num];
			else
				return;
		}
		assert(stemp);
		mergeLevel(stemp);
	}
	//-----------------------------------------------------
	bool CSplitModelProvider::destroyCom(Block * src, const std::string & level)
	{
		assert(src);
        SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		int lnum = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (stemp->childs[num])
			{
				stemp = stemp->childs[num];
				lnum = num;
			}
			else
				return true;
		}
		return destroyLevel(stemp);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::restoreCom(Block * src, const std::string & level)
	{
		assert(src);
		SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (stemp->childs[num])
			{
				stemp = stemp->childs[num];
			}
			else
			{
				assert((i + 1) == level.end());
				SplitBlock * temp = new SplitBlock();
				stemp->add(num, temp);
			}
		}
	}
	//-----------------------------------------------------
	void CSplitModelProvider::setComColour(Block * src, const std::string & level, DWORD colour)
	{
		assert(src);
		SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (stemp->childs[num])
			{
				stemp = stemp->childs[num];
			}
			else
				return;
		}
		assert(stemp);
		stemp->color = colour;
	}
	//-----------------------------------------------------
	DWORD CSplitModelProvider::getComColour(const Block * src, const std::string & level) const
	{
		assert(src);
		SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (stemp->childs[num])
			{
				stemp = stemp->childs[num];
			}
			else
				return 0;
		}
		assert(stemp);
		return stemp->color;
	}
	//-----------------------------------------------------
	void CSplitModelProvider::setComTexture(Block * src, const std::string & level, int texture)
	{
		assert(src);
		SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (num != -1 && stemp->childs[num])
				stemp = stemp->childs[num];
			else
				return;
		}
		assert(stemp);
		stemp->textureIdx = texture;
	}
	//-----------------------------------------------------
	int CSplitModelProvider::getComTexture(const Block * src, const std::string & level) const
	{
		assert(src);
		SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
		int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if (num != -1 && stemp->childs[num])
				stemp = stemp->childs[num];
			else
				return -1;
		}
		return stemp->textureIdx;
	}
    //-----------------------------------------------------
    std::string CSplitModelProvider::getComByCursor(const Block * src) const 
    {
		assert(src);
		return SplitBlock::last;
    };
    //-----------------------------------------------------
    int CSplitModelProvider::getComFaceCount(Block * src) const 
    {
		assert(src);
		int cnt = 0;
		if(src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
			cnt += getBlockModelCount(stemp);
		}
		return cnt * 6;
    }
    //-----------------------------------------------------
    int CSplitModelProvider::getComModelList(Block * src, BlockModelList & out) const
    {
		assert(src); 
        BlockModel temp;
        temp.LoadModelByTexture(1);
        int cnt = 0;
		bool nochild = true;
		if (src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock *>(src->getExtData());
            cnt += getSplitLevel(out, stemp, &temp, 0, nochild);
			if (nochild)
			{
				temp.SetColor(stemp->color);
				temp.SetTextureIndex(stemp->textureIdx);
				out.push_back(temp);
			}
		}
		return cnt;
    }
	//-----------------------------------------------------
	void CSplitModelProvider::initBlockData(Block * src) const
	{
		assert(src);
        assert(!src->getExtData());
        
		uint16_t templateId = src->GetTemplate()->GetID();
		SplitBlock * stemp = new SplitBlock();
		stemp->templateId = templateId;

		src->setExtData(stemp);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::destroyBlockData(Block * src) const 
	{
		assert(src);
        assert(src->getExtData());
        
        SplitBlock * stemp = static_cast<SplitBlock *>(src->getExtData());
		delete stemp;
		src->setExtData(0);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::splitLevel(SplitBlock * parent)
	{
		assert(parent);
		SplitBlock * temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(0, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(1, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(2, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(3, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(4, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(5, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(6, temp);

		temp = new SplitBlock();
		temp->set(parent->color);
		temp->textureIdx = parent->textureIdx;
		parent->add(7, temp);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::mergeLevel(SplitBlock * level)
	{
		level->remove(0);
		level->remove(1);
		level->remove(2);
		level->remove(3);
		level->remove(4);
		level->remove(5);
		level->remove(6);
		level->remove(7);
	}
	//-----------------------------------------------------
	bool CSplitModelProvider::destroyLevel(SplitBlock * level)
	{
		assert(level);
		SplitBlock * temp = level->parent;
		if (temp)
		{
			int i, iend = 8;
			for(i = 0; i < iend; ++i)
			{
				if (temp->childs[i] == level)
				{
					temp->remove(i);
					if (temp->isNoChild())
						return destroyLevel(temp);
					else
						return false;
				}
			}
		}
		return true;
	}
    //-----------------------------------------------------
    int CSplitModelProvider::getSplitLevel(BlockModelList & out, const SplitBlock * sparent, const BlockModel * bparent, int level, bool & nochild) const
    {
        int cnt = 0;
        int i, iend = 8;
        for (i = 0; i < iend; ++i)
        {
            if(sparent->childs[i])
            {
                cnt += getSplitLevel(out, sparent->childs[i], bparent, level + 1, i);
				nochild = false;
            }
        }
        return cnt;
    }
	//-----------------------------------------------------
	void CSplitModelProvider::getCom(const BlockModel * parent, const BlockModel & out, int i)
	{
		switch (i)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		}
	}
    //-----------------------------------------------------
	int CSplitModelProvider::getSplitLevel(BlockModelList & out, const SplitBlock * sparent, const BlockModel * bparent, int level, int i) const
    {
        int cnt = 0;
		bool nochild = true;
		float levelf = pow(2, level);
		BlockModel temp;
		temp.Clone(*bparent);
		temp.TranslateByVertex(4);
		temp.SetVerticalScale(0.5f);
		switch (i)
		{
		case 0:
		{
			temp.TranslateVertices(0.0f, 0.0f, 0.0f);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 1:
		{
			temp.TranslateVertices(1.0f / levelf, 0, 0);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 2:
		{
			temp.TranslateVertices(1.0f / levelf, 0, 1.0f / levelf);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 3:
		{
			temp.TranslateVertices(0, 0, 1.0f / levelf);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 4:
		{
			temp.TranslateVertices(0, 1.0f / levelf, 0);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 5:
		{
			temp.TranslateVertices(1.0f / levelf, 1.0f / levelf, 0);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 6:
		{
			temp.TranslateVertices(1.0f / levelf, 1.0f / levelf, 1.0f / levelf);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		case 7:
		{
			temp.TranslateVertices(0, 1.0f / levelf, 1.0f / levelf);
			temp.reflushAABB();
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			if (nochild)
			{
				temp.SetColor(sparent->color);
				out.push_back(temp);
			}
		}
		break;
		default:
			break;
		}
		return cnt;
    }
	//-----------------------------------------------------
	int CSplitModelProvider::getBlockModelCount(SplitBlock * parent) const
	{
		int cnt = 0;
		int i, iend = 8;
		bool nochild = true;
		for (i = 0; i < iend; ++i)
		{
			if (parent->childs[i])
			{
				cnt += getBlockModelCount(parent->childs[i]);
				nochild = false;
			}
		}

		if (nochild)
		{
			++cnt;
		}
		return cnt;
	}
	//-----------------------------------------------------
	int CSplitModelProvider::getLevelNum(char num) const
	{
		switch (num)
		{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		default:
			assert(false && "节点错误");
			return -1;
		}
		assert(false && "节点错误");
		return -1;
	}
	//-----------------------------------------------------
/*    void CSplitModelProvider::ExportXML(const std::string & out, VariableBlockModel * in)
    {
		CParaFile file(out.c_str());
		ExportXML(file.getBuffer(), file.getSize());
    }
    //-----------------------------------------------------
    void CSplitModelProvider::ExportXML(const char * out, int32 & size, VariableBlockModel * in)
    {
        
    }
    //-----------------------------------------------------
    void CSplitModelProvider::InportXML(const std::string & in, VariableBlockModel * out)
    {
		CParaFile file(in.c_str());
		InportXML(file.getBuffer(), file.getSize());
    }
    //-----------------------------------------------------
    void CSplitModelProvider::InportXML(const char * in, int32 size, VariableBlockModel * out)
    {
    }
	//-----------------------------------------------------
    void CSplitModelProvider::ExportBinary(const std::string & out, VariableBlockModel * in)
    {
		CParaFile file(out.c_str());
		//BMaxParser p(file.getBuffer(), file.getSize());
    }
    //-----------------------------------------------------
    void CSplitModelProvider::ExportBinary(const char * out, int32 & size, VariableBlockModel * in)
    {
        
    }
    //-----------------------------------------------------
    void CSplitModelProvider::InportBinary(const std::string & in, VariableBlockModel * out)
    {
		CParaFile file(in.c_str());
		//BMaxParser p(file.getBuffer(), file.getSize());
    }
    //-----------------------------------------------------
    void CSplitModelProvider::InportBinary(const char * in, int32 size, VariableBlockModel * out)
    {
    }
    //-----------------------------------------------------
	*/
}