#pragma once
#include "BlockModel.h"

namespace ParaEngine
{				
	class CBlockWorld;
	class BlockTemplate;
	class IBlockModelProvider;
	class Block;

	/** interface class for filtering block models. */
	class IBlockModelProvider
	{
	public:
		IBlockModelProvider(BlockTemplate* pBlockTemplate):m_pBlockTemplate(pBlockTemplate){};
		virtual ~IBlockModelProvider(){};

		/** get the default block model. 
		* This function must be implemented if one provide its own model
		* @param nIndex: model index default to 0
		*/
		virtual BlockModel& GetBlockModel(int nIndex = 0);
		
		virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
		virtual BlockModel& GetBlockModelByData(uint32 nData);

		/** get block model index 
		*/
		virtual int GetModelIndex(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks=NULL){return 0;};
        
        /** 是否是复合结构体
        @data 2017.4.18
        */
        virtual bool isComBlock() const {return false;}
        
		/** 通过当前鼠标分裂方块
		@note 仅在 isComBlock 为真时使用
		@data 2017.5.2
		*/
		virtual void splitCom(Block *, const std::string &){}

		/** 通过当前鼠标删除分裂出的方块
		@note 仅在 isComBlock 为真时使用
		@data 2017.5.2
		*/
		virtual bool destroyCom(Block *, const std::string &) { return true; }

		/** 通过当前制定层次设置颜色
		@note 仅在 isComBlock 为真时使用
		@data 2017.5.2
		*/
		virtual void setComColour(Block *, const std::string &, DWORD){}

		/** 通过当前制定层次获取颜色
		@note 仅在 isComBlock 为真时使用
		@data 2017.5.2
		*/
		virtual DWORD getComColour(const Block *, const std::string &) const { return 0; }

		/** 通过当前制定层次设置颜色
		@note 仅在 isComBlock 为真时使用
		@data 2017.5.2
		*/
		virtual void setComTexture(Block *, const std::string &, const std::string &){}
        
		/** 通过当前制定层次获取颜色
		@note 仅在 isComBlock 为真时使用
		@data 2017.5.2
		*/
		virtual std::string getComTexture(const Block *, const std::string &) const { return std::string(); }

		/** 通过当前鼠标获取当前选中的分裂方块
		@note 可分裂方块专用
		@data 2017.4.25
		*/
        virtual std::string getComByCursor(const Block *) const { return std::string();};
        
        /** 获取复合结构的数量
        @note 仅在 isComBlock 为真时使用
        @data 2017.4.18
        */
        virtual int getComFaceCount(Block *) const {return 0;}
        
        /** 获取复合结构模型数量
        @note 仅在 isComBlock 为真时使用
        @data 2017.4.18
        */
		virtual int getComModelList(Block *, BlockModelList &) const { return 0; }

		/** 初始化结构数据
		@data 2017.5.2
		*/
		virtual void initBlockData(Block *) const {}

		/** 释放复合结构数据
		@note 仅在 isComBlock 为真时使用
		@data 2017.4.18
		*/
		virtual void destroyBlockData(Block *) const {}
    protected:
		BlockTemplate * m_pBlockTemplate;
	};

	/** for grass cross model only. */
	class CGrassModelProvider: public IBlockModelProvider
	{
	public:
		CGrassModelProvider(BlockTemplate* pBlockTemplate): IBlockModelProvider(pBlockTemplate){};
		virtual ~CGrassModelProvider(){};

		
		/** get block model index 
		*/
		virtual int GetModelIndex(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks=NULL);
	};

	/** just return model linearly. */
	class CLinearModelProvider: public IBlockModelProvider
	{
	public:
		CLinearModelProvider(BlockTemplate* pBlockTemplate,int nCount = 0) : m_nModelCount(nCount), IBlockModelProvider(pBlockTemplate){};
		virtual ~CLinearModelProvider(){};

		virtual BlockModel& GetBlockModelByData(uint32 nData);

		/** get block model index 
		*/
		virtual int GetModelIndex(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks=NULL);

		int GetModelCount() const;
	protected:
		int m_nModelCount;
	};

}