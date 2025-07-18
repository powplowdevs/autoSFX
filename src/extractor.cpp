#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "logger/logger.h"
#include <json.hpp>
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include "extractor.h"
#include <zlib.h>
#include <filesystem>
#include <fstream>
#include <sstream>


// Json
using json = nlohmann::json;

// Helpers
bool isExe(const std::wstring& str){
    size_t pos = str.find_last_of(L".");
    if (pos == std::wstring::npos) return false;
    if (str.substr(pos).compare(L".exe") == 0 || str.substr(pos).compare(L".com") == 0) return true;
    return false;
}

bool decompressStreamed(std::istream& in, std::ostream& out, size_t compressedSize) {
    const size_t chunkSize = 16384;

    std::vector<uint8_t> inBuffer(chunkSize);
    std::vector<uint8_t> outBuffer(chunkSize);

    z_stream strm = {};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    if (inflateInit(&strm) != Z_OK) return false;

    size_t remaining = compressedSize;
    while (remaining > 0){
        size_t toRead = std::min(chunkSize, remaining);
        in.read(reinterpret_cast<char*>(inBuffer.data()), toRead);
        strm.avail_in = static_cast<uInt>(toRead);
        strm.next_in = inBuffer.data();

        do{
            strm.avail_out = chunkSize;
            strm.next_out = outBuffer.data();

            int ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                return false;
            }

            size_t have = chunkSize - strm.avail_out;
            out.write(reinterpret_cast<char*>(outBuffer.data()), have);
        } while (strm.avail_out == 0);

        remaining -= toRead;
    }

    inflateEnd(&strm);
    return true;
}


// Function to extract files to disk
std::vector<extractedFile> extractFiles(json& fileTable, std::ifstream& exeFile){
    if (!exeFile) logger.error("File read failed!");

    // Extracted file list
    std::vector<extractedFile> extractedFiles;

    // Loop json
    for (auto& entry : fileTable){
        // Grab file meta data
        std::string name = entry["name"];
        std::string path_str = entry["relativePath"];
        std::wstring path(path_str.begin(), path_str.end());
        uint64_t offset = entry["offset"];
        uint64_t size = entry["size"];
        uint64_t sizeCompressed = entry["sizeCompressed"];
        bool compressed = entry["compressed"].get<bool>();;
        bool runHidden = entry["runHidden"];
        uint32_t runCount = entry["runCount"];
        uint32_t runIndex = entry["runIndex"];

        logger.info("Found file: " + name);

        // Save file to run dict
        extractedFile file {name, path, offset, size, sizeCompressed, compressed, runHidden, runCount, runIndex};
        extractedFiles.push_back(file); 

        // Grab file bytes
        exeFile.seekg(offset, std::ios::beg);
        std::string fileContentBuffer;
        // handle compression
       if (compressed){
            std::ostringstream outStream;
            bool res = decompressStreamed(exeFile, outStream, sizeCompressed);

            if (!res) logger.error("Decompression failed: " + name);
            else {
                fileContentBuffer = outStream.str();
                logger.info("Decompressed: " + name);
            }
        }

        else{
            fileContentBuffer.resize(size);
            exeFile.read(fileContentBuffer.data(), size);
        }
        

        logger.info("Found file bytes: " + name);

        // Write file bytes to file
        std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "ASFX_EXTRC";
        std::filesystem::create_directories(tempDir);

        std::filesystem::path baseName = std::filesystem::path(path).filename();
        std::filesystem::path outputPath = tempDir / baseName;

        std::ofstream outFile(outputPath, std::ios::binary);
        outFile.write(fileContentBuffer.data(), fileContentBuffer.size());
        outFile.close();

        file.path = outputPath.wstring();

        logger.info("Extracted file: " + name);
    }

    // Sort run rict
    std::sort(extractedFiles.begin(), extractedFiles.end(), [](const extractedFile& a, const extractedFile& b) {return a.runIndex < b.runIndex;});

    return extractedFiles;
}

