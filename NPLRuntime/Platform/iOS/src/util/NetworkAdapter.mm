#import <UIKit/UIKit.h>

#include "ParaEngine.h"
#include "util/NetworkAdapter.h"

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>


namespace ParaEngine {
	void NetworkAdapter::Init()
	{
		if (m_bInitialized)
			return;
		m_bInitialized = true;
        
        // get mac address
        do
        {
            int mib[6] = {CTL_NET, AF_ROUTE, 0, AF_LINK, NET_RT_IFLIST};
            size_t len;
            char *buf;
            unsigned char *ptr;
            if_msghdr *ifm;
            sockaddr_dl *sdl;
            
            if ((mib[5] = if_nametoindex("en0")) == 0)
                break;
            if (sysctl(mib, 6, nullptr, &len, nullptr, 0) < 0)
                break;
            
            buf = (char*)malloc(len);
            
            if (sysctl(mib, 6, buf, &len, nullptr, 0) < 0)
            {
                free(buf);
                break;
            }
            
            ifm = (if_msghdr*)buf;
            sdl = (sockaddr_dl*)(ifm + 1);
            ptr = (unsigned char*)LLADDR(sdl);
            
            NSString *outstring = [NSString stringWithFormat:@"%02X%02X%02X%02X%02X%02X", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5)];
            
            m_sMacAddress = [outstring UTF8String];
            
            free(buf);
            
        }while(false);
        
	}

    const std::string& NetworkAdapter::GetMaxMacAddress()
    {
        return m_sMacAddress;
    }

    const std::string& NetworkAdapter::GetMaxIPAddress()
    {
        return m_sIpAddress;
    }
} // namespace ParaEngine

