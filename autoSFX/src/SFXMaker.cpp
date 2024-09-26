#include "SFXMaker.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

void SFXMaker::promptUser() {
    int numFiles;
    std::cout << "Enter amount of files: ";
    std::cin >> numFiles;

    std::vector<std::string> filePaths(numFiles);
    for (int i = 0; i < numFiles; ++i) {
        std::cout << "Enter file path: ";
        std::cin >> filePaths[i];
    }

    std::string outputPath;
    std::cout << "Enter output path for SFX (include .exe): ";
    std::cin >> outputPath;

    createSFX(filePaths, outputPath);
}

void SFXMaker::createSFX(const std::vector<std::string>& filePaths, const std::string& outputPath) {
    embedFiles(filePaths, outputPath);
    std::cout << "SFX created and saved at " << outputPath << std::endl;
}

void SFXMaker::embedFiles(const std::vector<std::string>& filePaths, const std::string& outputPath) {
    std::ofstream ofs(outputPath, std::ios::binary);

    //Simple header
    uint32_t numFiles = filePaths.size();
    ofs.write(reinterpret_cast<char*>(&numFiles), sizeof(numFiles));

    //Write the file paths and their sizes
    for (const auto& filePath : filePaths) {
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return;
        }

        //Get file size
        ifs.seekg(0, std::ios::end);
        std::streamsize size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        //Write file path length
        uint32_t pathLength = filePath.size();
        ofs.write(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
        ofs.write(filePath.c_str(), pathLength);

        //Write file size
        ofs.write(reinterpret_cast<char*>(&size), sizeof(size));

        //Write file content
        ofs << ifs.rdbuf();
    }

    ofs.close();
}