//-----------------------------------------------------------------------------
// Class:	base class for light calculation. 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.7
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockWorld.h"
#include "BlockLightGridBase.h"

namespace ParaEngine
{
	CBlockLightGridBase::CBlockLightGridBase(CBlockWorld* pBlockWorld)
		: m_nLightGridChunkSize(0), m_pBlockWorld(pBlockWorld), m_suspendLightUpdate(false), m_nLightCalculationStep(0), m_bIsAsyncLightCalculation(true), m_nSkyHeight(255)
	{
	}

	CBlockLightGridBase::~CBlockLightGridBase()
	{
	}

	void CBlockLightGridBase::OnEnterWorld()
	{
		m_nSkyHeight = 255;
	}

	void CBlockLightGridBase::OnLeaveWorld()
	{
	}

	void CBlockLightGridBase::OnWorldMove(uint16_t centerChunkX, uint16_t centerChunkZ)
	{
	}

	bool CBlockLightGridBase::GetBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nSize, int nLightType)
	{
		return false;
	}

	void CBlockLightGridBase::SetLightDirty(Uint16x3& blockId_ws, bool isSunLight, int8 nUpdateRange)
	{
	}

	void CBlockLightGridBase::NotifyBlockHeightChanged(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, ChunkMaxHeight& prevBlockHeight)
	{
	}

	void CBlockLightGridBase::UpdateLighting()
	{
	}

	void CBlockLightGridBase::AddDirtyColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
	}

	void CBlockLightGridBase::SetLightGridSize(int nSize)
	{
		m_nLightGridChunkSize = nSize;
	}

	int CBlockLightGridBase::GetLightGridSize()
	{
		return m_nLightGridChunkSize;
	}

	int CBlockLightGridBase::GetDirtyColumnCount()
	{
		return 0;
	}

	void CBlockLightGridBase::SuspendLightUpdate()
	{
		m_suspendLightUpdate = true;
	}

	void CBlockLightGridBase::ResumeLightUpdate()
	{
		m_suspendLightUpdate = false;
	}

	bool CBlockLightGridBase::IsLightUpdateSuspended()
	{
		return m_suspendLightUpdate;
	}

	BlockIndex CBlockLightGridBase::CalcLightDataIndex(const Uint16x3& blockId_ws, bool bCreateIfNotExist)
	{
		return m_pBlockWorld->GetBlockIndex(blockId_ws.x, blockId_ws.y, blockId_ws.z, bCreateIfNotExist);
	}

	LightData* CBlockLightGridBase::GetLightData(const BlockIndex& index)
	{
		if (index.m_pChunk != NULL)
			return index.m_pChunk->GetLightData(index.m_nChunkBlockIndex);
		return NULL;
	}

	LightData* CBlockLightGridBase::GetLightData(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist)
	{
		return m_pBlockWorld->GetLightData(x, y, z, bCreateIfNotExist);
	}

	BlockChunk* CBlockLightGridBase::GetChunk(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist)
	{
		return m_pBlockWorld->GetChunk(x, y, z, bCreateIfNotExist);
	}

	void CBlockLightGridBase::SetColumnPreloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{

	}

	void CBlockLightGridBase::SetLightCalculationStep(uint32 nTicks)
	{
		m_nLightCalculationStep = nTicks;
	}

	uint32 CBlockLightGridBase::GetLightCalculationStep() const
	{
		return m_nLightCalculationStep;
	}

	int CBlockLightGridBase::ForceAddChunkColumn(int nChunkWX, int nChunkWZ)
	{
		return 0;
	}

	int CBlockLightGridBase::GetForcedChunkColumnCount()
	{
		return 0;
	}

	bool CBlockLightGridBase::IsChunkColumnLoaded(int nChunkX, int nChunkZ)
	{
		return true;
	}

	int CBlockLightGridBase::GetDirtyBlockCount()
	{
		return 0;
	}

	void CBlockLightGridBase::SetColumnUnloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{

	}

	bool CBlockLightGridBase::IsAsyncLightCalculation() const
	{
		return m_bIsAsyncLightCalculation;
	}

	void CBlockLightGridBase::SetAsyncLightCalculation(bool val)
	{
		m_bIsAsyncLightCalculation = val;
	}
	void CBlockLightGridBase::SetSkyHeight(int nHeight)
	{
		if (m_nSkyHeight != nHeight) {
			m_nSkyHeight = nHeight;
			m_pBlockWorld->ClearBlockRenderCache();
		}
	}
	int CBlockLightGridBase::GetSkyHeight() const
	{
		return m_nSkyHeight;
	}

	int CBlockLightGridBase::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		IAttributeFields::InstallFields(pClass, bOverride);
		pClass->AddField("DirtyColumnCount", FieldType_Int, (void*)0, (void*)GetDirtyColumnCount_s, NULL, NULL, bOverride);
		pClass->AddField("DirtyBlockCount", FieldType_Int, (void*)0, (void*)GetDirtyBlockCount_s, NULL, NULL, bOverride);
		pClass->AddField("LightGridSize", FieldType_Int, (void*)0, (void*)GetLightGridSize_s, NULL, NULL, bOverride);
		pClass->AddField("LightCalculationStep", FieldType_Int, (void*)SetLightCalculationStep_s, (void*)GetLightCalculationStep_s, NULL, NULL, bOverride);
		pClass->AddField("SkyHeight", FieldType_Int, (void*)SetSkyHeight_s, (void*)GetSkyHeight_s, NULL, NULL, bOverride);

		return S_OK;
	}

}



