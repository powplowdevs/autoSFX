#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "logger/logger.h"
#include <json.hpp>
#include <windows.h>
#include <shellapi.h>
#include "extractor.h"

// Json
using json = nlohmann::json;

// Helpers
bool isExe(const std::wstring& str){
    size_t pos = str.find_last_of(L".");
    if (pos == std::wstring::npos) return false;
    if (str.substr(pos).compare(L".exe") == 0 || str.substr(pos).compare(L".com") == 0) return true;
    return false;
}

// Function to extract files to disk
void extractFiles(json& fileTable, std::ifstream& exeFile){
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
        bool compressed = entry["compressed"];
        bool runHidden = entry["runHidden"];
        uint32_t runCount = entry["runCount"];
        uint32_t runIndex = entry["runIndex"];

        logger.info("Found file: " + name);

        // Save file to run dict
        extractedFile file {name, path, offset, size, compressed, runHidden, runCount, runIndex};
        extractedFiles.push_back(file); 

        // Grab file bytes
        // TODO: handle compression
        exeFile.seekg(offset, std::ios::beg);
        std::string fileContentBuffer;
        fileContentBuffer.resize(size);
        exeFile.read(fileContentBuffer.data(), size);

        logger.info("Found file bytes: " + name);

        // Write file bytes to file
        std::ofstream outFile(path, std::ios::binary);
        outFile.write(fileContentBuffer.data(), size);
        outFile.close();

        logger.info("Extracted file: " + name);
    }

    // Sort run rict
    std::sort(extractedFiles.begin(), extractedFiles.end(), [](const extractedFile& a, const extractedFile& b) {return a.runIndex < b.runIndex;});

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
                result = CreateProcessW(entry.path.c_str(), NULL, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
                // Error handle
                if(result){
                    logger.info("Executed file: " + entry.name);
                    WaitForSingleObject(pi.hProcess, INFINITE);
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
            for(int i=0; i<entry.runCount; i++){
                result = ShellExecuteExW(&sei);
                if (sei.hProcess){
                    WaitForSingleObject(sei.hProcess, INFINITE);
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

