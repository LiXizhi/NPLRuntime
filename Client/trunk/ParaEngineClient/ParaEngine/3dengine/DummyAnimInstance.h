#pragma once
#include "AnimInstanceBase.h"

namespace ParaEngine
{
	/** this is only used when the anim x file does not exist. 
	it is usually used as a singleton with CDummyAnimInstance::GetInstance(). 
	the walk animation has a default speed of 4m/s
	*/
	class CDummyAnimInstance : public CAnimInstanceBase
	{
	public:
		CDummyAnimInstance();
		virtual ~CDummyAnimInstance(){};

		ATTRIBUTE_DEFINE_CLASS(CDummyAnimInstance);

		static CDummyAnimInstance* GetInstance();
	public:
		virtual void LoadAnimation(int nAnimID, float * fSpeed, bool bAppend = false);
		virtual bool HasAnimId(int nAnimID);
		/// normally this will read the move speed of the specified animation
		/// and calculate the correct(scaled) speed
		virtual void GetSpeedOf(const char * sName, float * fSpeed);
	};
}

