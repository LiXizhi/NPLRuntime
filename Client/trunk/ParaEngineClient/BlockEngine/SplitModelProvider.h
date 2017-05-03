/**
深圳市爱致科技有限公司
@version 1.0.0 
@data 2017.4.1
*/
#ifndef _PARAENGINE_SplitModelProvider_H_
#define _PARAENGINE_SplitModelProvider_H_

#include "BlockModelProvider.h"

namespace ParaEngine
{
	class SplitBlock;
    /** 用于描述可分割正方形
    @remark 
        描述可分割正方形,把自身平均成四份同等面积的正方形,这四份也
        有分割能力
    @note ParaEngine 专用类库
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
        /** 生成指定等级位置的方块模型
        @param[in] out
        @param[in] level
        @param[in] posmark
        @data 2017.5.2
        */
		void splitLevel(BlockModelList & out, const BlockModel * parent);
    protected:
/*
        /// 坐标系统
        enum CoordSystem
        {
            CS_LeftHand = 0,
            CS_RightHand = 1
        };
        
        /// 距离类型
        enum BaseUnit
        {
            BU_NM = 0,
            BU_MM = 1,
            BU_CM = 2,
            BU_M  = 3,
            BU_KM = 4
        };
        
        /// 分割类型
        enum BlockState
        {
            BS_Unsplit = 0,
            BS_Split   = 1
        };

        /// 子方块结构
        struct SubBlock
        {
            unsigned int id;            ///< 唯一标识
            unsigned int parentid;      ///< 父方块id
            unsigned char coord;        ///< 父方块中的相对坐标
            unsigned char texture;      ///< 材质序号
            BlockState state;           ///< 方块类型
            unsigned char subBlockData; ///< 子方块状态
            SubBlock * subBlocks;       ///< 子方块链
        };

        /// 基础块结构
        struct BaseBlock
        {
            unsigned int id;            ///< 唯一标识
            unsigned short x;           ///< x坐标值
            unsigned short y;           ///< y坐标值
            unsigned short z;           ///< z坐标值
            unsigned char texture;      ///< 材质引用
            BlockState state;           ///< 方块类型
            unsigned char subBlockData; ///< 子方块状态
            SubBlock * subBlocks;       ///< 子方块链
        };
   
		struct SplitBlock
		{
			unsigned char id;
			SplitBlock * sub[8];
			SplitBlock * parent;
		}; 
      
        /// 块链结构
        struct VariableBlockModel
        {
            CoordSystem coordSystem;    ///< 使用的坐标系
            BaseUnit baseUnit;          ///< 基本单位
            unsigned short baseLength;  ///< 单位长度
            SplitBlock * baseBlocks;     ///< 所有基础块
        };
        */          
    public:
        /** 导出到文件中(like BMax)
        @param[in] out 输出到文件
        @note 一种很像BMax的格式
        @version 1.0.0
        */
//        static void ExportXML(const std::string & out, VariableBlockModel * in);
        
        /** 导出到数据流中(like BMax)
        @note 一种很像BMax的格式
        @version 1.0.0
        */
//        static void ExportXML(const char * out, int32 & size, VariableBlockModel * in);
        
        /** 从文件中导入(like BMax)
        @note 一种很像BMax的格式
        @version 1.0.0
        */
//        static void InportXML(const std::string & in, VariableBlockModel * out);   
        
        /** 从数据流中导入(like BMax)
        @note 一种很像BMax的格式
        @version 1.0.0
        */
//        static void InportXML(const char * in, int32 size, VariableBlockModel * out);
        
        /** 导出到文件中(Bin)
        @note 二进制结构
        @version 1.0.0
        */
//        static void ExportBinary(const std::string & out, VariableBlockModel * in);
        
        /** 导出到数据流中(Bin)
        @note 二进制结构
        @version 1.0.0
        */
//        static void ExportBinary(const char * out, int32 & size, VariableBlockModel * in);
        
        /** 从文件中导入(Bin)
        @note 二进制结构
        @version 1.0.0
        */
//        static void InportBinary(const std::string & in, VariableBlockModel * out);

        /** 从数据流中导入(Bin)
        @note 二进制结构
        @version 1.0.0
        */
//        static void InportBinary(const char * in, int32 size, VariableBlockModel * out);
	protected:

    private:
		BlockModel mFrameModel;
        BlockModel mSplitModel[8]; // 测试
    };
}

#endif