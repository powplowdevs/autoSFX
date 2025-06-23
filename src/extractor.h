#pragma once

#include <json.hpp>

extern Logger logger;
using json = nlohmann::json;

bool isExe(const std::wstring& str);

void extractFiles(json& fileTable, std::ifstream& exeFile);

// Struct for running files
struct extractedFile {
    std::string name;
    std::wstring path;
    uint64_t offset;
    uint64_t size;
    bool compressed;
    bool runHidden;
    uint32_t runCount;
    uint32_t runIndex;
};
