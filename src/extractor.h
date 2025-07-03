#pragma once

#include <zlib.h>
#include <json.hpp>

extern Logger logger;
using json = nlohmann::json;

// Struct for running files
struct extractedFile {
    std::string name;
    std::wstring path;
    uint64_t offset;
    uint64_t size;
    uint64_t sizeCompressed;
    bool compressed;
    bool runHidden;
    uint32_t runCount;
    uint32_t runIndex;
};


bool isExe(const std::wstring& str);
bool decompressStreamed(std::istream& in, std::ostream& out, size_t compressedSize);

std::vector<extractedFile> extractFiles(json& fileTable, std::ifstream& exeFile);

