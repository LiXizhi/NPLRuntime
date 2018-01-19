#pragma once

namespace ParaEngine
{
	class CBipedObject;
	struct ActiveBiped;

	/** base class for all AI objects */
	class CAIBase
	{
	public:
		enum AIObjectType
		{
			BASE = 0,	/// base class never used directly
			NPC,		/// NPC
			UNDEFINED	/// undefined
		};
		virtual AIObjectType GetType(){return BASE;};
		CAIBase(CBipedObject* pBiped);
		CAIBase(void);
		virtual ~CAIBase(void);
	private:
		/** to which this biped controller is associated.*/
		CBipedObject*		m_pBiped;
	public:
		/** get the biped, to which this biped controller is associated. */
		CBipedObject* GetBiped();
		/** Set the biped, to which this biped controller is associated. */
		void SetBiped(CBipedObject* pBiped);

		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		* @param pInput: It holds any information that is perceived by a Active Biped object
		*/
		virtual void FrameMove(float fDeltaTime)=0;

	public:

	};

}
