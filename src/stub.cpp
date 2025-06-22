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


// Custom logger
Logger logger;

// Json
using json = nlohmann::json;

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

// Helpers
bool isExe(const std::wstring& str){
    size_t pos = str.find_last_of(L".");
    if (pos == std::wstring::npos) return false;
    if (str.substr(pos).compare(L".exe") == 0 || str.substr(pos).compare(L".com") == 0) return true;
    return false;
}

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
            for(int i=0; i<entry.runCount; i++){
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
