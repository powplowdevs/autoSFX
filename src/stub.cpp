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

    // Find file tabel JSON (not up to date, see ectractor.cpp function extractFiles)
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

void executeFiles(const std::vector<extractedFile>&  extractedFiles){
    // Exectue files
    for (auto& entry : extractedFiles){
        BOOL result = NULL;
        // Check if is exe file
        if(isExe(entry.path)){
            // Create process
            // Startup info
            STARTUPINFOW si = {0};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = entry.runHidden ? SW_HIDE : SW_SHOW;
            // Process info
            PROCESS_INFORMATION pi = { 0 };

            //Run
            for(uint32_t i=0; i<entry.runCount; i++){
                result = CreateProcessW(entry.path.c_str(), NULL, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
                // Error handle
                if(result){
                    logger.info("Executed file: " + entry.name);
                    // WaitForSingleObject(pi.hProcess, INFINITE);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                else logger.error("Error executing file " + entry.name + " " + std::to_string(GetLastError()));
            }
        }
        else{
            // Shell Execute info
            SHELLEXECUTEINFOW sei = {0};
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.lpVerb = L"open";
            sei.lpFile = entry.path.c_str();
            sei.nShow = entry.runHidden ? SW_HIDE : SW_SHOW;

            // Shell Execute    
            for(uint32_t i=0; i<entry.runCount; i++){
                result = ShellExecuteExW(&sei);
                if (sei.hProcess){
                    // WaitForSingleObject(sei.hProcess, INFINITE);
                    CloseHandle(sei.hProcess);
                }
                // Error handle
                if(result) logger.info("Executed file: " + entry.name);
                else logger.error("Error executing file " + entry.name + " " + std::to_string(GetLastError()));
            }
        }
    }

    return;
}


int main(int argc, char* argv[])
{
    // Open self-exe in binary mode
    std::ifstream exeFile(argv[0], std::ios::binary);
    if (!exeFile)
    {
        logger.error("Failed to open self executabel.");
        return 1;
    }

    // Locate packed data
    json fileTable = locatefileTable(exeFile);

    try {
        // Read file tabel and extract files and run files
        executeFiles(extractFiles(fileTable, exeFile));
    }
    catch (const std::exception& e) {
        logger.error(std::string("Exception: ") + e.what());
        return 1;
    }

    exeFile.close();
    return 0;
}