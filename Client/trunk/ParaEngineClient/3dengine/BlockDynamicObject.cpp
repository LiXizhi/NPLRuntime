//////////////////////////////////////////////////////////////////////////////-
// Class:	block piece particle
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.1.17
//////////////////////////////////////////////////////////////////////////////-
#include "ParaEngine.h"
#include "BaseCamera.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockDynamicObject.h"

using namespace ParaEngine;

namespace ParaEngine
{
	float CBlockDynamicObject::default_min_speed = 0.3f;
	float CBlockDynamicObject::default_surface_decay = 0.5f;
	float CBlockDynamicObject::default_air_decay = 0.02f;
	float CBlockDynamicObject::default_gravity = 9.18f * 2.f;
	float CBlockDynamicObject::default_speedlost_perbounce = 0.5f;
}

ParaEngine::CBlockDynamicObject::CBlockDynamicObject()
	:m_can_bounce(true), m_is_stopped(false), m_isOnGround(false), m_lifetime(2.f), m_fade_out_time(1.0f), m_vBlockPos(0, 0, 0), m_vSpeed(0, 0, 0), m_vOffsetFromCenter(0, 0, 0), m_bIsDead(false)
{

}

ParaEngine::CBlockDynamicObject::~CBlockDynamicObject()
{

}

bool ParaEngine::CBlockDynamicObject::TestCollision(CBaseCamera* pCamera)
{
	Vector3 vPos = GetRenderOffset();
	return pCamera->GetObjectFrustum()->CullPointsWithFrustum(&vPos, 1);
}

void ParaEngine::CBlockDynamicObject::SetParent(CBaseObject* pParent)
{
	m_parent = pParent;
}

CBaseObject* ParaEngine::CBlockDynamicObject::GetParent()
{
	return m_parent;
}

bool ParaEngine::CBlockDynamicObject::IsDead()
{
	return m_bIsDead;
}

void ParaEngine::CBlockDynamicObject::SetDead()
{
	m_bIsDead = true;
	AddToDeadObjectPool();
}

void ParaEngine::CBlockDynamicObject::SetOnGround(bool val)
{
	m_isOnGround = val;
	if (m_isOnGround)
	{
		if (!CanBounce())
			m_vSpeed.y = 0;
	}
}

bool ParaEngine::CBlockDynamicObject::IsOnGround() const
{
	return m_isOnGround;
}

void ParaEngine::CBlockDynamicObject::UpdateParams()
{
	BlockCommon::ConvertToBlockIndex((float)m_vPos.x, (float)(m_vPos.y + m_fRadius), (float)(m_vPos.z), m_vBlockPos.x, m_vBlockPos.y, m_vBlockPos.z);
	m_vBlockCenter = BlockCommon::ConvertToRealPosition(m_vBlockPos.x, m_vBlockPos.y, m_vBlockPos.z);
	m_vOffsetFromCenter = m_vPos - m_vBlockCenter;
	m_vOffsetFromCenter.y += m_fRadius;
}


void ParaEngine::CBlockDynamicObject::SetPosition(const DVector3& v)
{
	if (m_vPos != v)
	{
		CTileObject::SetPosition(v);
		UpdateParams();
	}
}

void ParaEngine::CBlockDynamicObject::Animate(double dTimeDelta, int nRenderNumber /*= 0*/)
{
	if (IsDead())
		return;
	float deltaTime = (float)dTimeDelta;
	if (m_lifetime >= 0)
		m_lifetime -= deltaTime;
	if (m_lifetime < 0){
		SetDead();
		return;
	}
	if (m_is_stopped) {
		return;
	}

	float half_blocksize = BlockConfig::g_half_blockSize;
	float radius = GetRadius();
	int bx = m_vBlockPos.x, by = m_vBlockPos.y, bz = m_vBlockPos.z;
	bool is_stopped = true;

	m_isOnGround = false;

	auto pBlockWorld = BlockWorldClient::GetInstance();

	float min_speed = default_min_speed;
	float gravity = -default_gravity;
	// apply gravity in y direction. and compute new position.  
	{
		m_vSpeed.y = m_vSpeed.y + gravity * deltaTime;
		float  offset_y = m_vOffsetFromCenter.y + m_vSpeed.y * deltaTime;
		if (m_vSpeed.y < 0) {
			if ((offset_y - radius) < -half_blocksize && pBlockWorld->IsObstructionBlock(bx, by - 1, bz))
			{
				m_vSpeed.y = -m_vSpeed.y*default_speedlost_perbounce;
				if ((Math::Abs(m_vSpeed.y) < (min_speed + Math::Abs(gravity*deltaTime))))
				{
					m_vSpeed.y = 0;
					offset_y = radius - half_blocksize;
					m_vPos.y = m_vBlockCenter.y + offset_y - radius;
				}
				else
					is_stopped = false;
				SetOnGround(true);
			}
			else
			{
				m_vOffsetFromCenter.y = offset_y;
				m_vPos.y = m_vBlockCenter.y + offset_y - radius;
				is_stopped = false;
			}
		}
		else
		{
			if ((offset_y + radius) > half_blocksize && pBlockWorld->IsObstructionBlock(bx, by + 1, bz))
				m_vSpeed.y = -m_vSpeed.y*default_speedlost_perbounce;
			else
			{
				m_vOffsetFromCenter.y = offset_y;
				m_vPos.y = m_vBlockCenter.y + offset_y - radius;
			}
			is_stopped = false;
		}
	}

	// apply acceleration in x,z plane if y speed is 0. 
	float speed_decay = (m_vSpeed.y == 0) ? default_surface_decay : default_air_decay;

	if (m_vSpeed.x < 0) {
		m_vSpeed.x = m_vSpeed.x + speed_decay * deltaTime;
		if (m_vSpeed.x > 0)
			m_vSpeed.x = 0;
	}
	else if (m_vSpeed.x > 0) {
		m_vSpeed.x = m_vSpeed.x - speed_decay * deltaTime;
		if (m_vSpeed.x < 0)
			m_vSpeed.x = 0;
	}
	if (m_vSpeed.z < 0) {
		m_vSpeed.z = m_vSpeed.z + speed_decay * deltaTime;
		if (m_vSpeed.z > 0)
			m_vSpeed.z = 0;
	}
	else if (m_vSpeed.z > 0) {
		m_vSpeed.z = m_vSpeed.z - speed_decay * deltaTime;
		if (m_vSpeed.z < 0)
			m_vSpeed.z = 0;
	}

	// x direction
	{
		float offset_x = m_vOffsetFromCenter.x + m_vSpeed.x*deltaTime;
		if (m_vSpeed.x < 0) {
			if ((offset_x - radius) < -half_blocksize && pBlockWorld->IsObstructionBlock(bx - 1, by, bz))
				m_vSpeed.x = -m_vSpeed.x*default_speedlost_perbounce;
			else{
				m_vOffsetFromCenter.x = offset_x;
				m_vPos.x = m_vBlockCenter.x + offset_x;
			}
		}
		else
		{
			if ((offset_x + radius) > half_blocksize && pBlockWorld->IsObstructionBlock(bx + 1, by, bz))
				m_vSpeed.x = -m_vSpeed.x*default_speedlost_perbounce;
			else{
				m_vOffsetFromCenter.x = offset_x;
				m_vPos.x = m_vBlockCenter.x + offset_x;
			}
		}

		if (Math::Abs(m_vSpeed.x) < min_speed)
			m_vSpeed.x = 0;
		else
			is_stopped = false;
	}

	// z direction
	{
		float offset_z = m_vOffsetFromCenter.z + m_vSpeed.z*deltaTime;
		if (m_vSpeed.z < 0) {
			if ((offset_z - radius) < -half_blocksize && pBlockWorld->IsObstructionBlock(bx, by, bz - 1))
				m_vSpeed.z = -m_vSpeed.z*default_speedlost_perbounce;
			else
			{
				m_vOffsetFromCenter.z = offset_z;
				m_vPos.z = m_vBlockCenter.z + offset_z;
			}
		}
		else
		{
			if ((offset_z + radius) > half_blocksize && pBlockWorld->IsObstructionBlock(bx, by, bz + 1))
				m_vSpeed.z = -m_vSpeed.z*default_speedlost_perbounce;
			else
			{
				m_vOffsetFromCenter.z = offset_z;
				m_vPos.z = m_vBlockCenter.z + offset_z;
			}
		}

		if (Math::Abs(m_vSpeed.z) < min_speed)
			m_vSpeed.z = 0;
		else
			is_stopped = false;
	}
	if (!is_stopped)
	{
		if (!((Math::Abs(m_vOffsetFromCenter.x) < half_blocksize && Math::Abs(m_vOffsetFromCenter.y) < half_blocksize && Math::Abs(m_vOffsetFromCenter.z) < half_blocksize)))
		{
			UpdateParams();
			if (pBlockWorld->IsObstructionBlock(m_vBlockPos.x, m_vBlockPos.y, m_vBlockPos.z))
			{
				m_vSpeed.x = 0;
				m_vSpeed.y = 0;
				m_vSpeed.z = 0;
				m_is_stopped = true;
				SetOnGround(true);
			}
		}
	}
	m_is_stopped = is_stopped;
}
int ParaEngine::CBlockDynamicObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CTileObject::InstallFields(pClass, bOverride);

	pClass->AddField("lifetime", FieldType_Float, (void*)SetLifetime_s, (void*)GetLifetime_s, NULL, NULL, bOverride);
	pClass->AddField("FadeOutTime", FieldType_Float, (void*)SetFadeOutTime_s, (void*)GetFadeOutTime_s, NULL, NULL, bOverride);
	pClass->AddField("CanBounce", FieldType_Bool, (void*)SetCanBounce_s, (void*)CanBounce_s, NULL, NULL, bOverride);
	pClass->AddField("Speed", FieldType_Vector3, (void*)SetSpeed_s, (void*)GetSpeed_s, NULL, "global position of the character", bOverride);
	return S_OK;
}
