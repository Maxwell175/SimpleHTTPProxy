#include "routeinfo.h"

RouteInfo::RouteInfo()
{
    DestinationHost = "";
    DestinationPort = 0;
    UseSSL = false;
    HostHeader = "";
}

RouteInfo::RouteInfo(std::string destinationHost, unsigned short destinationPort)
{
    DestinationHost = destinationHost;
    DestinationPort = destinationPort;
    UseSSL = false;

    if (DestinationPort == 80)
        HostHeader = DestinationHost;
    else {
        HostHeader = DestinationHost;
        HostHeader += ":";
        HostHeader += DestinationPort;
    }
}

RouteInfo::RouteInfo(std::string destinationHost, unsigned short destinationPort, bool useSSL)
{
    DestinationHost = destinationHost;
    DestinationPort = destinationPort;
    UseSSL = useSSL;

    if (DestinationPort == 80)
        HostHeader = DestinationHost;
    else {
        HostHeader = DestinationHost;
        HostHeader += ":";
        HostHeader += DestinationPort;
    }
}

RouteInfo::RouteInfo(std::string destinationHost, unsigned short destinationPort,
                     bool useSSL, std::string hostHeader)
{
    DestinationHost = destinationHost;
    DestinationPort = destinationPort;
    UseSSL = useSSL;
    HostHeader = hostHeader;
}
