#include "common.h"

ConfigManager* Common::GlobalConfig = new ConfigManager();

std::vector<std::string>* Split(std::string Input, std::string Delimiter) {
    std::vector<std::string>* Output = new std::vector<std::string>();

    auto start = 0U;
    auto end = Input.find(Delimiter);
    while (end != std::string::npos)
    {
        Output->push_back(Input.substr(start, end - start));
        start = end + Delimiter.length();
        end = Input.find(Delimiter, start);
    }

    Output->push_back(Input.substr(start, end));

    return Output;
}

ConfigManager* GlobalConfig() {
    return Common::GlobalConfig;
}
