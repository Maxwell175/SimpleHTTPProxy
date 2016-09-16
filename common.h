#include <string>
#include <vector>

#include "configmanager.h"

#ifndef COMMON_H
#define COMMON_H

class ConfigManager;
class HTTPServer;
class HTTPThread;

class Common {

public:
    static ConfigManager* GlobalConfig;
};


std::vector<std::string>* Split(std::string Input, std::string Delimiter);
ConfigManager* GlobalConfig();


#endif // COMMON_H
