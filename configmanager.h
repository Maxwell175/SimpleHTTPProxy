#include <string>
#include <map>
#include "routeinfo.h"

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

class ConfigManager
{
public:
    ConfigManager();

    std::map<std::string,RouteInfo>* DomainRoutes;
};

#endif // CONFIGMANAGER_H
