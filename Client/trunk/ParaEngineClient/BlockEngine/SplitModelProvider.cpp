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
		/*int num = -1;
		std::string::const_iterator i, iend = level.end();
		for (i = level.begin(); i != iend; ++i)
		{
			num = getLevelNum(*i);
			if(stemp->childs[num])
				stemp = stemp->childs[num];
			else
				return;
		}
		assert(stemp);*/
		splitLevel(stemp);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::destroyCom(Block * src, const std::string & level)
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
				return;
		}
		assert(stemp);
		SplitBlock * stemp2 = stemp->parent;
		if(stemp2)
		{
			assert(lnum != -1);
			stemp2->remove(lnum);
		}
		else
		{
			comLevel(stemp);
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
	void CSplitModelProvider::setComTexture(Block * src, const std::string & level, const char* texture)
	{
		// 保留到后期
		/*
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
		splitLevel(stemp); 
		*/
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
		}
		if (nochild)
			out.push_back(temp);
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
		SplitBlock *sp0 = new SplitBlock();
		sp0->set(0x00000000);
		parent->add(0, sp0);

		SplitBlock *sp1 = new SplitBlock();
		sp1->set(0xffffffff);
		parent->add(1, sp1);

		SplitBlock *sp2 = new SplitBlock();
		sp2->set(0xffff0000);
		parent->add(2, sp2);

		SplitBlock *sp3 = new SplitBlock();
		sp3->set(0x0000ffff);
		parent->add(3, sp3);

		SplitBlock *sp4 = new SplitBlock();
		sp4->set(0xffff0000);
		parent->add(4, sp4);

		SplitBlock *sp5 = new SplitBlock();
		sp5->set(0xffffff00);
		parent->add(5, sp5);

		SplitBlock *sp6 = new SplitBlock();
		sp6->set(0x0000ffff);
		parent->add(6, sp6);

		//
/*		SplitBlock *sp60 = new SplitBlock();
		sp60->set(0x0000ffff);
		sp6->add(0, sp60);

		//
		SplitBlock *sp61 = new SplitBlock();
		sp61->set(0x0000ffff);
		sp6->add(1, sp61);
		*/

		SplitBlock *sp7 = new SplitBlock();
		sp7->set(0x000000ff);
		parent->add(7, sp7);
	}
	//-----------------------------------------------------
	void CSplitModelProvider::comLevel(SplitBlock * parent)
	{
		assert(parent);
		parent->remove(0);
		parent->remove(1);
		parent->remove(2);
		parent->remove(3);
		parent->remove(4);
		parent->remove(5);
		parent->remove(6);
		parent->remove(7);
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
	int CSplitModelProvider::getSplitLevel(BlockModelList & out, const SplitBlock * sparent, const BlockModel * bparent, int level, int i) const
    {
        int cnt = 0;
		bool nochild = true;
		switch (i)
		{
		case 0:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(0, 0, 0);
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 1:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(1.0f / (level * 2), 0, 0);
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 2:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(0, 0, 1.0f / (level * 2));
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 3:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(1.0f / (level * 2), 0, 1.0f / (level * 2));
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 4:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(0, 1.0f / (level * 2), 0);
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 5:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(1.0f / (level * 2), 1.0f / (level * 2), 0);
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 6:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(0, 1.0f / (level * 2), 1.0f / (level * 2));
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);

			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
		}
			break;
		case 7:
		{
			BlockModel temp;
			temp.Clone(*bparent);
			temp.SetVerticalScale(0.5f);
			temp.TranslateVertices(1.0f / (level * 2), 1.0f / (level * 2), 1.0f / (level * 2));
			cnt += getSplitLevel(out, sparent, &temp, level, nochild);
			temp.SetColor(sparent->color);
			if (nochild)
				out.push_back(temp);
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