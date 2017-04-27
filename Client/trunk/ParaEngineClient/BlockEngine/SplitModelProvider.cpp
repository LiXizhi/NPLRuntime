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

namespace ParaEngine
{
    //-----------------------------------------------------
	CSplitModelProvider::CSplitModelProvider(BlockTemplate* pBlockTemplate):
        IBlockModelProvider(pBlockTemplate)
    {
        mFrameModel.LoadModelByTexture(0);

        mSplitModel[0].LoadModelByTexture(3);
        mSplitModel[0].SetVerticalScale(0.5f);
        mSplitModel[0].TranslateVertices(0, 0, 0);
		mSplitModel[0].SetColor(0x000000ff);

        mSplitModel[1].LoadModelByTexture(3);
        mSplitModel[1].SetVerticalScale(0.5f);
        mSplitModel[1].TranslateVertices(0.5, 0, 0);
		mSplitModel[1].SetColor(0xffff0000);

        mSplitModel[2].LoadModelByTexture(3);
        mSplitModel[2].SetVerticalScale(0.5f);
        mSplitModel[2].TranslateVertices(0, 0, 0.5);
		mSplitModel[2].SetColor(0xffffff00);

        mSplitModel[3].LoadModelByTexture(3);
        mSplitModel[3].SetVerticalScale(0.5f);
        mSplitModel[3].TranslateVertices(0.5, 0, 0.5);
		mSplitModel[3].SetColor(0x0000ffff);

		mSplitModel[4].LoadModelByTexture(3);
		mSplitModel[4].SetVerticalScale(0.5f);
		mSplitModel[4].TranslateVertices(0, 0.5, 0);
		mSplitModel[4].SetColor(0xffff0000);

		mSplitModel[5].LoadModelByTexture(3);
		mSplitModel[5].SetVerticalScale(0.5f);
		mSplitModel[5].TranslateVertices(0.5, 0.5, 0);
		mSplitModel[5].SetColor(0xffffff00);

		mSplitModel[6].LoadModelByTexture(3);
		mSplitModel[6].SetVerticalScale(0.5f);
		mSplitModel[6].TranslateVertices(0, 0.5, 0.5);
		mSplitModel[6].SetColor(0x0000ffff);

		mSplitModel[7].LoadModelByTexture(3);
		mSplitModel[7].SetVerticalScale(0.5f);
		mSplitModel[7].TranslateVertices(0.5, 0.5, 0.5);
		mSplitModel[7].SetColor(0x000000ff);
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
    int CSplitModelProvider::getComFaceCount(Block * src) const 
    {
		assert(src);
		int cnt = 0;
		if (!src->getExtData())
		{
			SplitBlock * temp = new SplitBlock();
			temp->add(0);
			temp->add(1);
			temp->add(2);
			src->setExtData(temp);
		}
		if(src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
			if(stemp->childs[0])
				++cnt;
			if (stemp->childs[1])
				++cnt;
			if (stemp->childs[2])
				++cnt;
			if (stemp->childs[3])
				++cnt;
			if (stemp->childs[4])
				++cnt;
			if (stemp->childs[5])
				++cnt;
			if (stemp->childs[6])
				++cnt;
			if (stemp->childs[7])
				++cnt;
		}
		return cnt * 6;
    }
    //-----------------------------------------------------
    int CSplitModelProvider::getComModelList(Block * src, BlockModelList & out) const
    {
		assert(src);
		if (!src->getExtData())
		{
			SplitBlock * temp = new SplitBlock();
			temp->add(0);
			temp->add(1);
			temp->add(2);
			src->setExtData(temp);
		}
		int cnt = 0;
		if (src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock *>(src->getExtData());
			if (stemp->childs[0])
			{
				out.push_back(mSplitModel[0]);
				++cnt;
			}
			if (stemp->childs[1])
			{
				out.push_back(mSplitModel[1]);
				++cnt;
			}
			if (stemp->childs[2])
			{
				out.push_back(mSplitModel[2]);
				++cnt;
			}
			if (stemp->childs[3])
			{
				out.push_back(mSplitModel[3]);
				++cnt;
			}
			if (stemp->childs[4])
			{
				out.push_back(mSplitModel[4]);
				++cnt;
			}
			if (stemp->childs[5])
			{
				out.push_back(mSplitModel[5]);
				++cnt;
			}
			if (stemp->childs[6])
			{
				out.push_back(mSplitModel[6]);
				++cnt;
			}
			if (stemp->childs[7])
			{
				out.push_back(mSplitModel[7]);
				++cnt;
			}
		}
		return cnt;
    }
	//-----------------------------------------------------
	void CSplitModelProvider::freeComBlockData(Block * src) const 
	{
		if (src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock *>(src->getExtData());
			delete stemp;
		}
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