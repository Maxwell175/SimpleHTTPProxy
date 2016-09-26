#include "configmanager.h"

ConfigManager::ConfigManager()
{
    DomainRoutes = new std::map<std::string,RouteInfo>();

    DomainRoutes->insert(std::pair<std::string,RouteInfo>("abc.com", RouteInfo("github.com", 80, false)));
}
