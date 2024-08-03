#pragma once
#include "util/mutex.h"
#include <set>
#include <unordered_map>
#include <boost/core/noncopyable.hpp>
#include "NPLUDPRoute.h"

namespace NPL
{
	class CNPLUDPRouteManager : private boost::noncopyable
	{
	public:
		CNPLUDPRouteManager();
		~CNPLUDPRouteManager();

		/// Add the specified route to the manager and start it.
		void start(NPLUDPRoute_ptr route);

		/* Add the specified route to the manager without starting it.
		*/
		void add(NPLUDPRoute_ptr route);

		/** Stop the specified route.
		* send a message to tell the client about this.
		*/
		void stop(NPLUDPRoute_ptr route);

		/// Stop all route.
		void stop_all();

		/** get the total number of routes. */
		int get_route_count();

		///
		int ForEachRoute(std::function<bool(const NPLUDPRoute_ptr& route)>);

		///
		bool HasRoute(const boost::asio::ip::udp::endpoint& ep);
		bool HasRoute(const char* ip, unsigned short port);
		bool HasRoute(unsigned long long hash);

		///
		NPLUDPRoute_ptr GetRoute(const boost::asio::ip::udp::endpoint& ep);
		NPLUDPRoute_ptr GetRoute(const char* ip, unsigned short port);
		NPLUDPRoute_ptr GetRoute(unsigned long long hash);

		/** for all active routes, check if any of them should be timed out. If so, close it or send keep alive message.
		* This function is called by the NPLNetUDPServer from an io service timer that periodically does the checking.
		* @return the number of timed out routes.
		*/
		int CheckIdleTimeout();

	private:
		/*
		typedef std::set < boost::shared_ptr<CNPLUDPRoute>, NPLUDPRoute_PtrOps> NPLRoutePool_Type;

		/// The managed routes. It only keeps established routes, but it does not mean that the route here is authenticated. 
		NPLRoutePool_Type m_connections;
		*/

		std::unordered_map<unsigned long long, NPLUDPRoute_ptr> m_connections;


		ParaEngine::mutex m_mutex;
	};

} // namespace NPL