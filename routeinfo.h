#include <string>

#ifndef ROUTEINFO_H
#define ROUTEINFO_H


class RouteInfo
{
public:
    RouteInfo();
    RouteInfo(std::string destinationHost, unsigned short destinationPort);
    RouteInfo(std::string destinationHost, unsigned short destinationPort, bool useSSL);
    RouteInfo(std::string destinationHost, unsigned short destinationPort, bool useSSL, std::string hostHeader);

    std::string DestinationHost;
    unsigned short DestinationPort;
    bool UseSSL;

    std::string HostHeader;
};

#endif // ROUTEINFO_H
