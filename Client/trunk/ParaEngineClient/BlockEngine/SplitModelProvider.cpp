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
		splitLevel(stemp);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::destroyCom(Block * src, const std::string & level)
	{
		assert(src);
        SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
	}
	//-----------------------------------------------------
	void CSplitModelProvider::setComColour(Block * src, const std::string & level, DWORD colour)
	{
        assert(src);
        SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
	}
	//-----------------------------------------------------
	void CSplitModelProvider::setComTexture(Block * src, const std::string & level, const char* texture)
	{
		assert(src);
        SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
	}
    //-----------------------------------------------------
    std::string CSplitModelProvider::getComByCursor(const Block * src) const 
    {
		assert(src);
        SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
        
		Vector3 vPickRayOrig, vPickRayDir;
		POINT ptCursor;
		Matrix4 matWorld = Matrix4::IDENTITY;
		int cursorpx, cursorpy;
		CGlobals::GetGUI()->GetMousePosition(&cursorpx, &cursorpy);
		float fScaleX = 1.f, fScaleY = 1.f;
		CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
		ptCursor.x = (fScaleX == 1.f) ? cursorpx : (int)(cursorpx*fScaleX);
		ptCursor.y = (fScaleY == 1.f) ? cursorpy : (int)(cursorpy*fScaleY);
		cursorpx = ptCursor.x;
		cursorpy = ptCursor.y;
		int nWidth, nHeight;
		CGlobals::GetViewportManager()->GetPointOnViewport(cursorpx, cursorpy, &nWidth, &nHeight);
		ptCursor.x = cursorpx;
		ptCursor.y = cursorpy;
		CGlobals::GetScene()->GetCurrentCamera()->GetMouseRay(vPickRayOrig, vPickRayDir, ptCursor, nWidth, nHeight, &matWorld);

		CShapeRay(vPickRayOrig + CGlobals::GetScene()->GetRenderOrigin(), vPickRayDir);
		return std::string();
    };
    //-----------------------------------------------------
    int CSplitModelProvider::getComFaceCount(Block * src) const 
    {
		assert(src);
		int cnt = 0;
		if(src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock * >(src->getExtData());
			int i, iend = 8;
			for (i = 0; i < iend; ++i)
			{
				if(stemp->childs[i])
					++cnt;
			}
			if(cnt == 0)
			{
				++cnt;
			}
		}
		return cnt * 6;
    }
    //-----------------------------------------------------
    int CSplitModelProvider::getComModelList(Block * src, BlockModelList & out) const
    {
		assert(src);
		int cnt = 0;
		if (src->getExtData())
		{
			SplitBlock * stemp = static_cast<SplitBlock *>(src->getExtData());
			int i, iend = 8;
			for (i = 0; i < iend; ++i)
			{
				if (stemp->childs[i])
				{
					out.push_back(mSplitModel[i]);
					++cnt;
				}
			}
			if (cnt == 0)
			{
				out.push_back(mFrameModel);
				++cnt;
			}
		}
		return cnt;
    }
	//-----------------------------------------------------
	void CSplitModelProvider::initBlockData(Block * src) const
	{
		assert(src);
        assert(!src->getExtData());
        
		SplitBlock * stemp = new SplitBlock();
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
		parent->add(0);
		parent->add(1);
		parent->add(2);
		parent->add(3);
		parent->add(4);
		parent->add(5);
		parent->add(6);
		parent->add(7);
	}
    //-----------------------------------------------------
	void CSplitModelProvider::splitLevel(BlockModelList & out, const BlockModel * parent)
    {
		mSplitModel[0].Clone(*parent);
		mSplitModel[0].SetVerticalScale(0.5f);
		mSplitModel[0].TranslateVertices(0, 0, 0);
		mSplitModel[0].SetColor(0x00000000);
		out.push_back(mSplitModel[0]);

		mSplitModel[1].Clone(*parent);
		mSplitModel[1].SetVerticalScale(0.5f);
		mSplitModel[1].TranslateVertices(0.5, 0, 0);
		mSplitModel[1].SetColor(0xffffffff);
		out.push_back(mSplitModel[1]);

		mSplitModel[2].Clone(*parent);
		mSplitModel[2].SetVerticalScale(0.5f);
		mSplitModel[2].TranslateVertices(0, 0, 0.5);
		mSplitModel[2].SetColor(0xffff0000);
		out.push_back(mSplitModel[2]);

		mSplitModel[3].Clone(*parent);
		mSplitModel[3].SetVerticalScale(0.5f);
		mSplitModel[3].TranslateVertices(0.5, 0, 0.5);
		mSplitModel[3].SetColor(0x0000ffff);
		out.push_back(mSplitModel[3]);

		mSplitModel[4].Clone(*parent);
		mSplitModel[4].SetVerticalScale(0.5f);
		mSplitModel[4].TranslateVertices(0, 0.5, 0);
		mSplitModel[4].SetColor(0xffff0000);
		out.push_back(mSplitModel[4]);

		mSplitModel[5].Clone(*parent);
		mSplitModel[5].SetVerticalScale(0.5f);
		mSplitModel[5].TranslateVertices(0.5, 0.5, 0);
		mSplitModel[5].SetColor(0xffffff00);
		out.push_back(mSplitModel[5]);

		mSplitModel[6].Clone(*parent);
		mSplitModel[6].SetVerticalScale(0.5f);
		mSplitModel[6].TranslateVertices(0, 0.5, 0.5);
		mSplitModel[6].SetColor(0x0000ffff);
		out.push_back(mSplitModel[6]);

		mSplitModel[7].Clone(*parent);
		mSplitModel[7].SetVerticalScale(0.5f);
		mSplitModel[7].TranslateVertices(0.5, 0.5, 0.5);
		mSplitModel[7].SetColor(0x000000ff);
		out.push_back(mSplitModel[7]);
    }
	//-----------------------------------------------------

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