#include "AutoSFXHeader.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <filesystem>

//Read file data as bytes
std::vector<char> ReadFile(const std::string& FilePath) {
    std::ifstream File(FilePath, std::ios::binary);
    if (!File) {
        std::cerr << "Error: Could not open file " << FilePath << std::endl;
        exit(1);
    }
    return std::vector<char>((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
}

//Create a hidden temp dir
std::string CreateHiddenTempDir() {
    char TempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, TempPath);
    std::string TempDir = std::string(TempPath) + "HiddenSFX\\";

    if (!CreateDirectoryA(TempDir.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        std::cerr << "Error: Could not create temporary directory." << std::endl;
        exit(1);
    }

    SetFileAttributesA(TempDir.c_str(), FILE_ATTRIBUTE_HIDDEN);
    
    std::cout << "Hidden temporary directory created at: " << TempDir << std::endl;

    return TempDir;
}

//Edit escape paths
std::string EscapePath(const std::string& Path) {
    std::string EscapedPath = Path;
    size_t Pos = 0;
    while ((Pos = EscapedPath.find("\\", Pos)) != std::string::npos) {
        EscapedPath.replace(Pos, 1, "\\\\");
        Pos += 2;
    }
    return EscapedPath;
}

int main() {
    int NumFiles;
    std::cout << "Enter the number of files to be in the SFX: ";
    std::cin >> NumFiles;

    std::vector<std::string> FilePaths(NumFiles);
    std::vector<std::string> FileExtensions(NumFiles);

    //Get file paths
    for (int i = 0; i < NumFiles; ++i) {
        std::cout << "Enter file path " << i + 1 << ": ";
        std::cin >> FilePaths[i];
        FileExtensions[i] = std::filesystem::path(FilePaths[i]).extension().string();
    }

    //Collect byte data
    std::vector<std::vector<char>> FileData;
    for (const auto& Path : FilePaths) {
        FileData.push_back(ReadFile(Path));
    }

    //Create hidden temp dir
    std::string TempDir = CreateHiddenTempDir();

    //Generate SFXtemplate
    std::ofstream SfxTemplate("SFXtemplate.cpp");
    SfxTemplate << "#include <iostream>\n#include <fstream>\n#include <vector>\n#include <windows.h>\n\n";
    SfxTemplate << "int main() {\n";

    for (size_t i = 0; i < FileData.size(); ++i) {
        SfxTemplate << "    // File " << i + 1 << "\n";
        SfxTemplate << "    const char File" << i + 1 << "[] = {";
        for (size_t j = 0; j < FileData[i].size(); ++j) {
            SfxTemplate << static_cast<int>(FileData[i][j]) << ",";
        }
        SfxTemplate << "};\n";
        SfxTemplate << "    std::ofstream OutFile" << i + 1 << "(\"" << EscapePath(TempDir + "output_file_" + std::to_string(i + 1) + FileExtensions[i]) << "\", std::ios::binary);\n";
        SfxTemplate << "    OutFile" << i + 1 << ".write(File" << i + 1 << ", sizeof(File" << i + 1 << "));\n";
        SfxTemplate << "    OutFile" << i + 1 << ".close();\n\n";
    }

    for (size_t i = 0; i < FileData.size(); ++i) {
        SfxTemplate << "    ShellExecute(NULL, \"open\", \"" << EscapePath(TempDir + "output_file_" + std::to_string(i + 1) + FileExtensions[i]) << "\", NULL, NULL, SW_SHOWNORMAL);\n";
    }

    SfxTemplate << "    return 0;\n";
    SfxTemplate << "}\n";
    SfxTemplate.close();

    //Generate CMakeLists
    std::ofstream CMakeFile("CMakeLists.txt");
    CMakeFile << "cmake_minimum_required(VERSION 3.10)\n";
    CMakeFile << "project(GeneratedSFX)\n";
    CMakeFile << "add_executable(generated_sfx SFXtemplate.cpp)\n";
    CMakeFile.close();

    //Compile
    std::string BuildCommand = "cmake . && cmake --build .";
    system(BuildCommand.c_str());

    std::cout << "SFX executable generated: generated_sfx.exe\n";
    return 0;
}
