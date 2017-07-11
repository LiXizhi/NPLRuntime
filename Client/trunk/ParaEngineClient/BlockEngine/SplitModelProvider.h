/**
�����а��¿Ƽ����޹�˾
@version 1.0.0 
@data 2017.4.1
*/
#ifndef _PARAENGINE_SplitModelProvider_H_
#define _PARAENGINE_SplitModelProvider_H_

#include "BlockModelProvider.h"

namespace ParaEngine
{
	class SplitBlock;
    /** ���������ɷָ�������
    @remark 
        �����ɷָ�������,������ƽ�����ķ�ͬ�������������,���ķ�Ҳ
        �зָ�����
    @note ParaEngine ר�����
    @version 1.0.0
    */
    class CSplitModelProvider : public IBlockModelProvider
    {
    public:
		CSplitModelProvider(BlockTemplate* pBlockTemplate);
		virtual ~CSplitModelProvider();
        
		/// @copydetails IBlockModelProvider::GetBlockModel
		virtual BlockModel& GetBlockModel(int nIndex = 0);

        /// @copydetails IBlockModelProvider::GetBlockModel
        virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
		
		/// @copydetails IBlockModelProvider::GetBlockModelByData
		virtual BlockModel& GetBlockModelByData(uint32 nData);

        /// @copydetails IBlockModelProvider::GetModelIndex
		virtual int GetModelIndex(CBlockWorld * pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks=NULL);
        
        /// @copydetails IBlockModelProvider::isComBlock
        virtual bool isComBlock() const;
        
		/// @copydetails IBlockModelProvider::splitCom
		virtual void splitCom(Block * src, const std::string & level);

		/// @copydetails IBlockModelProvider::mergeCom
		virtual void mergeCom(Block * src, const std::string & level, int templateId, DWORD color);

		/// @copydetails IBlockModelProvider::destroyCom
		virtual bool destroyCom(Block * src, const std::string & level);

		/// @copydetails IBlockModelProvider::restoreCom
		virtual void restoreCom(Block * src, const std::string & level);

		/// @copydetails IBlockModelProvider::setComColour
		virtual void setComColour(Block * src, const std::string & level, DWORD colour);

		/// @copydetails IBlockModelProvider::getComColour
		virtual DWORD getComColour(const Block * src, const std::string & level) const;

		/// @copydetails IBlockModelProvider::setComTexture
		virtual void setComTexture(Block * src, const std::string & level, int texture);
        
		/// @copydetails IBlockModelProvider::getComTexture
		virtual int getComTexture(const Block * src, const std::string & level) const;

        /// @copydetails IBlockModelProvider::getComByCursor
        virtual std::string getComByCursor(const Block * src) const;
        
        /// @copydetails IBlockModelProvider::getComCount
        virtual int getComFaceCount(Block * src) const;
        
        /// @copydetails IBlockModelProvider::getComModelList
        virtual int getComModelList(Block * src, BlockModelList & out) const;

		virtual void getComModel(Block * src, BlockModel & out, const std::string level);

		/// @copydetails IBlockModelProvider::intBlockData
		virtual void initBlockData(Block * src) const;

		/// @copydetails IBlockModelProvider::destroyBlockData
		virtual void destroyBlockData(Block * src) const;
    protected:
		/** ����
		@param[in] parent
		@data 2017.5.2
		*/
		void splitLevel(SplitBlock * level);

		/** �ϲ�
		@param[in] parent
		@data 2017.5.2
		*/
		void mergeLevel(SplitBlock * level);

		/** ɾ���ȼ�
		@param[in] parent
		@data 2017.5.2
		*/
		bool destroyLevel(SplitBlock * level);

        /** ��ȡ���ѵȼ�����ģ��
        @param[in] out
        @param[in] parent
        @data 2017.5.2
        */
		int getSplitLevel(BlockModelList & out, const SplitBlock * sparent, const BlockModel * bparent, int level, bool & nochild) const;
        
        /** ��ȡ���ѵȼ�����ģ��
        @param[in] out
        @param[in] parent
        @data 2017.5.2
        */
		int getSplitLevel(BlockModelList & out, const SplitBlock * sparent, const BlockModel * bparent,  int level, int i) const;

		void getSplitLevel(BlockModel & temp,  SplitBlock *pSplitBlock, int level, int i);

		/**
		@data 2017.5.2
		*/
		int getBlockModelCount(SplitBlock * parent) const;

		/**
		@data 2017.5.3
		*/
		int getLevelNum(char num) const;
    private:
		BlockModel mFrameModel;
    };
}
#endif