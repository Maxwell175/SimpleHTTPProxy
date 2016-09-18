#include "configmanager.h"

ConfigManager::ConfigManager()
{
    DomainRoutes = new std::map<std::string,std::string>();

    DomainRoutes->insert(std::pair<std::string,std::string>("abc.com", "stackoverflow.com:80"));
}
