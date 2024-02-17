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
	protected:
		BlockTemplate* m_pBlockTemplate;
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
	protected:
		Vector3 vec3Rotate(const Vector3 &pt, const Vector3 &angles);
		Vector3 vec3RotateByPoint(const Vector3 &originPt, const Vector3 &pt, const Vector3 &angles);
	};

}