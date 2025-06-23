// Today I ran this script three times...  
// The code worked, then failed, then worked again.  
// Such is the way of the machine, I guess.  
// If it works now, leave it as it is.  
// - Ayoub (aka powplowdevs), 6/22/2025, 3:07 AM  

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "logger/logger.h"
#include <json.hpp>
#include <windows.h>
#include <shellapi.h>
#include "extractor.h"


// Custom logger
Logger logger;

// Json
using json = nlohmann::json;

// Function to locate packed data in self-exe
json locatefileTable(std::ifstream& exeFile){
    const char MARKER[] = "SFXPKGv1";
    const uint32_t MARKER_SIZE = 8;
    const uint32_t FILE_TABEL_SIZE_SIZE = 4;

    // Find file size
    exeFile.seekg(0, std::ios::end);
    std::streampos fileSize = exeFile.tellg();

    // Find marker
    std::streamoff MarkerOffset = static_cast<std::streamoff>(fileSize) - static_cast<std::streamoff>(MARKER_SIZE + 4); // Format [AutoSfxSrouce][packed file data][File tabel][MARKER][file tabel size]
    exeFile.seekg(MarkerOffset, std::ios::beg);
    char markerBuffer[MARKER_SIZE];
    exeFile.read(markerBuffer, MARKER_SIZE);

    // Error handle
    if(memcmp(markerBuffer, MARKER, MARKER_SIZE) != 0){
        logger.error("Marker missing!");
        return json{};
    }
    logger.info("Marker Found: " + std::string(markerBuffer, MARKER_SIZE));

    // Find file tabel size
    uint32_t fileTableSize;
    exeFile.read(reinterpret_cast<char*>(&fileTableSize), FILE_TABEL_SIZE_SIZE);

    // Error handle
    if(fileTableSize <= 0){
        logger.error("File tabel size is 0 or below!");
        return json{};
    }
    logger.info("File tabel of size " + std::to_string(fileTableSize) + " found!");

    // Find file tabel JSON
    // {
    //     "name": "filename.xyz",
    //     "relativePath": "PATH/filename.xyz",
    //     "offset": 123456,
    //     "size": 1234,
    //     "compressed": false
    //     "runCount": x
    //     "runIndex": x
    // },

    std::streamoff fileTableOffset = static_cast<std::streamoff>(fileSize) - static_cast<std::streamoff>(MARKER_SIZE + fileTableSize + FILE_TABEL_SIZE_SIZE);
    exeFile.seekg(fileTableOffset, std::ios::beg);
    std::string fileTablebuffer;
    fileTablebuffer.resize(fileTableSize);
    exeFile.read(fileTablebuffer.data(), fileTableSize);

    // Error handle
    if(fileTablebuffer == "" || fileTablebuffer == "[]"){
        logger.error("File tabel empty or not foudn!");
        return json{};
    }
    logger.info("File tabel found \n" + fileTablebuffer + "\n");

    // Parse string into json
    json fileTable = json::parse(fileTablebuffer);
    return fileTable;
}

int main(int argc, char* argv[])
{
    // Open self-exe in binary mode
    std::ifstream exeFile(argv[0], std::ios::binary);
    if (!exeFile)
    {
        std::cerr << "Failed to open self executabel." << std::endl;
        return 1;
    }

    // Locate packed data
    json fileTable = locatefileTable(exeFile);

    try {
        // Read file tabel and extract files
        extractFiles(fileTable, exeFile);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    exeFile.close();
    return 0;
}