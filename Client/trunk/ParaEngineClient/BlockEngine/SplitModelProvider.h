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

		/// @copydetails IBlockModelProvider::destroyCom
		virtual void destroyCom(Block * src, const std::string & level);

		/// @copydetails IBlockModelProvider::setComColour
		virtual void setComColour(Block * src, const std::string & level, DWORD colour);

		/// @copydetails IBlockModelProvider::setComTexture
		virtual void setComTexture(Block * src, const std::string & level, const char* texture);
        
        /// @copydetails IBlockModelProvider::getComByCursor
        virtual std::string getComByCursor(const Block * src) const;
        
        /// @copydetails IBlockModelProvider::getComCount
        virtual int getComFaceCount(Block * src) const;
        
        /// @copydetails IBlockModelProvider::getComModelList
        virtual int getComModelList(Block * src, BlockModelList & out) const;

		/// @copydetails IBlockModelProvider::intBlockData
		virtual void initBlockData(Block * src) const;

		/// @copydetails IBlockModelProvider::destroyBlockData
		virtual void destroyBlockData(Block * src) const;
    protected:
        /** ����ָ���ȼ�λ�õķ���ģ��
        @param[in] out
        @param[in] level
        @param[in] posmark
        @data 2017.5.2
        */
		void splitLevel(BlockModelList & out, const BlockModel * parent);
    protected:
/*
        /// ����ϵͳ
        enum CoordSystem
        {
            CS_LeftHand = 0,
            CS_RightHand = 1
        };
        
        /// ��������
        enum BaseUnit
        {
            BU_NM = 0,
            BU_MM = 1,
            BU_CM = 2,
            BU_M  = 3,
            BU_KM = 4
        };
        
        /// �ָ�����
        enum BlockState
        {
            BS_Unsplit = 0,
            BS_Split   = 1
        };

        /// �ӷ���ṹ
        struct SubBlock
        {
            unsigned int id;            ///< Ψһ��ʶ
            unsigned int parentid;      ///< ������id
            unsigned char coord;        ///< �������е��������
            unsigned char texture;      ///< �������
            BlockState state;           ///< ��������
            unsigned char subBlockData; ///< �ӷ���״̬
            SubBlock * subBlocks;       ///< �ӷ�����
        };

        /// ������ṹ
        struct BaseBlock
        {
            unsigned int id;            ///< Ψһ��ʶ
            unsigned short x;           ///< x����ֵ
            unsigned short y;           ///< y����ֵ
            unsigned short z;           ///< z����ֵ
            unsigned char texture;      ///< ��������
            BlockState state;           ///< ��������
            unsigned char subBlockData; ///< �ӷ���״̬
            SubBlock * subBlocks;       ///< �ӷ�����
        };
   
		struct SplitBlock
		{
			unsigned char id;
			SplitBlock * sub[8];
			SplitBlock * parent;
		}; 
      
        /// �����ṹ
        struct VariableBlockModel
        {
            CoordSystem coordSystem;    ///< ʹ�õ�����ϵ
            BaseUnit baseUnit;          ///< ������λ
            unsigned short baseLength;  ///< ��λ����
            SplitBlock * baseBlocks;     ///< ���л�����
        };
        */          
    public:
        /** �������ļ���(like BMax)
        @param[in] out ������ļ�
        @note һ�ֺ���BMax�ĸ�ʽ
        @version 1.0.0
        */
//        static void ExportXML(const std::string & out, VariableBlockModel * in);
        
        /** ��������������(like BMax)
        @note һ�ֺ���BMax�ĸ�ʽ
        @version 1.0.0
        */
//        static void ExportXML(const char * out, int32 & size, VariableBlockModel * in);
        
        /** ���ļ��е���(like BMax)
        @note һ�ֺ���BMax�ĸ�ʽ
        @version 1.0.0
        */
//        static void InportXML(const std::string & in, VariableBlockModel * out);   
        
        /** ���������е���(like BMax)
        @note һ�ֺ���BMax�ĸ�ʽ
        @version 1.0.0
        */
//        static void InportXML(const char * in, int32 size, VariableBlockModel * out);
        
        /** �������ļ���(Bin)
        @note �����ƽṹ
        @version 1.0.0
        */
//        static void ExportBinary(const std::string & out, VariableBlockModel * in);
        
        /** ��������������(Bin)
        @note �����ƽṹ
        @version 1.0.0
        */
//        static void ExportBinary(const char * out, int32 & size, VariableBlockModel * in);
        
        /** ���ļ��е���(Bin)
        @note �����ƽṹ
        @version 1.0.0
        */
//        static void InportBinary(const std::string & in, VariableBlockModel * out);

        /** ���������е���(Bin)
        @note �����ƽṹ
        @version 1.0.0
        */
//        static void InportBinary(const char * in, int32 size, VariableBlockModel * out);
	protected:

    private:
		BlockModel mFrameModel;
        BlockModel mSplitModel[8]; // ����
    };
}

#endif