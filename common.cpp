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

int countOccurences(std::string Input, std::string SubString) {
    int count = 0;
    size_t nPos = Input.find(SubString, 0); // fist occurrence
    while(nPos != std::string::npos)
    {
        count++;
        nPos = Input.find(SubString, nPos+1);
    }

    return count;
}

ConfigManager* GlobalConfig() {
    return Common::GlobalConfig;
}
