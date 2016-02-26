#pragma once
#include "FrameRateController.h"

namespace NPL
{
	struct NPLPacket;
}
namespace ParaEngine
{
	class CEnvSimServer;
	class CEnvSimClient;
	/** emulating a client/server based ParaEngine world editing game, using NPL language. 
	* we might create other network application classes in future for truly distributed network logics. 
	* for simplicity, this class is for C/S only. 
	*/
	class CEmuCSParaEditGame
	{
	public:
		CEmuCSParaEditGame(void);
		~CEmuCSParaEditGame(void);
		/** get the singleton instance. */
		static CEmuCSParaEditGame& GetInstance();

		enum CurrentState
		{
			STATE_STAND_ALONE = 0, 
			STATE_SERVER,
			STATE_CLIENT,
		};
	public:

		/**
		* get the current state
		* @return 
		*/
		CurrentState GetState();

		/** set the current game engine states. 
		* @return: true if successful. */
		bool SetState(CurrentState state);

		/**
		* Whenever an NPL chat packet is received. This function will call other handlers.
		* @return :true if the packet is successfully processed. 
		*/
		bool OnReceiveNPLPacket(NPL::NPLPacket* p);
		/**
		* this function is called automatically by the NPL runtime, when a user is lost by the center
		* @param sUserName 
		* @return 
		*/
		bool OnCenterUserLost(const char* sUserName);
		/**
		* this function is called automatically by the NPL runtime, when a user is lost by the receptor
		* @param sUserName 
		* @return 
		*/
		bool OnReceptorUserLost(const char* sUserName);
		/**
		* Call this function to send a copy of the local terrain to a destination world at a given resolution.
		* @param sDestination a destination name, it could be "all@server" or "@server", etc. This will be used to fill the destination address of the packet to be sent. 
		* @param sSource a source name, it could be "".
		* @param vCenter center of the terrain region to send
		* @param fRadius radius of the terrain region to send
		* @param fResolution if this is 1.f, the local world terrain grid resolution is used. 
		* @return 
		*/
		bool SendTerrainUpdate(const string& sDestination, const D3DXVECTOR3& vCenter, float fRadius, float fResolution=1.f);

		/**
		* called when received a terrain update from the network. It will update the local global terrain data according to the received data.
		* @param p 
		* @return 
		*/
		bool OnReceiveTerrainUpdate(NPL::NPLPacket* p);
		
		/**
		* this should be called every frame. Internally, it may send certain network packages at some given rate. 
		*/
		void FrameMove();
	private:
		CEnvSimServer* GetEnvServer();
		CEnvSimClient* GetEnvClient();

		CFrameRateController m_clientTimer;
		CFrameRateController m_serverTimer;
	};

}
