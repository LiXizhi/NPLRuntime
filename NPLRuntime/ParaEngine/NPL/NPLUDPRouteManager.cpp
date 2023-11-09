//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <algorithm>
#include <list>
#include <boost/bind.hpp>
#include "NPLUDPRouteManager.h"

namespace NPL {

	CNPLUDPRouteManager::CNPLUDPRouteManager()
	{

	}

	CNPLUDPRouteManager::~CNPLUDPRouteManager()
	{

	}

	void CNPLUDPRouteManager::start(NPLUDPRoute_ptr route)
	{
		{
			ParaEngine::Lock lock_(m_mutex);
			m_connections[route->getHash()] = route;
		}
		route->start();
	}

	void CNPLUDPRouteManager::stop(NPLUDPRoute_ptr route)
	{
		{
			ParaEngine::Lock lock_(m_mutex);
			m_connections.erase(route->getHash());
		}
		route->stop(false);
	}

	void CNPLUDPRouteManager::stop_all()
	{
	
		ParaEngine::Lock lock_(m_mutex);

		for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
		{
			auto& route = it->second;
			route->stop(false);
		}

		m_connections.clear();
	}

	int CNPLUDPRouteManager::CheckIdleTimeout()
	{
		std::list<NPLUDPRoute_ptr> dead_route_pool;

		int nCount = 0;
		{
			ParaEngine::Lock lock_(m_mutex);
			unsigned int nCurTime = GetTickCount();


			for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
			{
				auto& route = it->second;

				int nRes = route->CheckIdleTimeout(nCurTime);
				if (nRes != 1)
				{
					++nCount;
					if (nRes == 0)
					{
						// add to remove queue;
						dead_route_pool.push_back(route);
					}
				}
			}
		}
		if (!dead_route_pool.empty())
		{
			for (auto itCur = dead_route_pool.begin(); itCur != dead_route_pool.end(); ++itCur)
			{
				stop(*itCur);
			}
			dead_route_pool.clear();
		}

		return nCount;
	}

	int  CNPLUDPRouteManager::ForEachRoute(std::function<bool(const NPLUDPRoute_ptr& route)> cb)
	{
		ParaEngine::Lock lock_(m_mutex);

		int nCount = 0;
		for (auto itCur = m_connections.begin(); itCur != m_connections.end(); ++itCur)
		{
			++nCount;
			if (cb(itCur->second) != 0)
				break;
		}
		return nCount;
	}

	void CNPLUDPRouteManager::add(NPLUDPRoute_ptr route)
	{
		ParaEngine::Lock lock_(m_mutex);
		m_connections[route->getHash()] = route;
	}


	int CNPLUDPRouteManager::get_route_count()
	{
		ParaEngine::Lock lock_(m_mutex);
		return (int)(m_connections.size());
	}


#ifndef EMSCRIPTEN_SINGLE_THREAD
	bool CNPLUDPRouteManager::HasRoute(const boost::asio::ip::udp::endpoint& ep)
	{
		return HasRoute(NPLUDPAddress::ComputerHash(ep));
	}
#endif
	bool CNPLUDPRouteManager::HasRoute(const char* ip, unsigned short port)
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		boost::asio::ip::udp::endpoint ep(boost::asio::ip::make_address_v4(ip), port);
		return HasRoute(ep);
#else
		return false;
#endif
	}

	bool CNPLUDPRouteManager::HasRoute(unsigned long long hash)
	{
		ParaEngine::Lock lock_(m_mutex);
		return m_connections.find(hash) != m_connections.end();
	}

	///
#ifndef EMSCRIPTEN_SINGLE_THREAD
	NPLUDPRoute_ptr CNPLUDPRouteManager::GetRoute(const boost::asio::ip::udp::endpoint& ep)
	{
		return GetRoute(NPLUDPAddress::ComputerHash(ep));
	}
#endif
	NPLUDPRoute_ptr CNPLUDPRouteManager::GetRoute(const char* ip, unsigned short port)
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		boost::asio::ip::udp::endpoint ep(boost::asio::ip::make_address_v4(ip), port);
		return GetRoute(ep);
#else
		return GetRoute(0);
#endif
	}

	NPLUDPRoute_ptr CNPLUDPRouteManager::GetRoute(unsigned long long hash)
	{
		ParaEngine::Lock lock_(m_mutex);
		auto it = m_connections.find(hash);
		if (it == m_connections.end())
			return nullptr;
		else
			return it->second;
	}


} // namespace NPL