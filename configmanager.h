#include <string>
#include <map>

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

class ConfigManager
{
public:
    ConfigManager();

    std::map<std::string,std::string>* DomainRoutes;
};

#endif // CONFIGMANAGER_H
