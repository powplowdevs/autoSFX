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

//Escape paths for Windows
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

    //Generate SFXtemplate with dynamic temp dir creation
    std::ofstream SfxTemplate("SFXtemplate.cpp");
    SfxTemplate << "#include <iostream>\n";
    SfxTemplate << "#include <fstream>\n";
    SfxTemplate << "#include <vector>\n";
    SfxTemplate << "#include <windows.h>\n\n";

    SfxTemplate << "//Func to dynamically create hidden temp dir\n";
    SfxTemplate << "std::string CreateHiddenTempDir() {\n";
    SfxTemplate << "    char TempPath[MAX_PATH];\n";
    SfxTemplate << "    GetTempPathA(MAX_PATH, TempPath);\n";
    SfxTemplate << "    std::string TempDir = std::string(TempPath) + \"autoSFX\\\\\";\n";
    SfxTemplate << "    if (!CreateDirectoryA(TempDir.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {\n";
    SfxTemplate << "        std::cerr << \"Error: Could not create temporary directory.\" << std::endl;\n";
    SfxTemplate << "        exit(1);\n";
    SfxTemplate << "    }\n";
    SfxTemplate << "    SetFileAttributesA(TempDir.c_str(), FILE_ATTRIBUTE_HIDDEN);\n";
    SfxTemplate << "    return TempDir;\n";
    SfxTemplate << "}\n\n";

    SfxTemplate << "int main() {\n";
    SfxTemplate << "    //Create hidden temp dir\n";
    SfxTemplate << "    std::string TempDir = CreateHiddenTempDir();\n\n";

    for (size_t i = 0; i < FileData.size(); ++i) {
        SfxTemplate << "    //File" << i + 1 << "\n";
        SfxTemplate << "    const char File" << i + 1 << "[] = {";
        for (size_t j = 0; j < FileData[i].size(); ++j) {
            SfxTemplate << static_cast<int>(FileData[i][j]) << ",";
        }
        SfxTemplate << "};\n";
        SfxTemplate << "    std::ofstream OutFile" << i + 1 << "(TempDir + \"output_file_" << i + 1 << FileExtensions[i] << "\", std::ios::binary);\n";
        SfxTemplate << "    OutFile" << i + 1 << ".write(File" << i + 1 << ", sizeof(File" << i + 1 << "));\n";
        SfxTemplate << "    OutFile" << i + 1 << ".close();\n\n";
    }

    for (size_t i = 0; i < FileData.size(); ++i) {
        SfxTemplate << "    ShellExecute(NULL, \"open\", (TempDir + \"output_file_" << i + 1 << FileExtensions[i] << "\").c_str(), NULL, NULL, SW_SHOWNORMAL);\n";
    }

    SfxTemplate << "    return 0;\n";
    SfxTemplate << "}\n";
    SfxTemplate.close();

    //Compile
    std::string BuildCommand = "g++ -static -static-libgcc -static-libstdc++ SFXtemplate.cpp -o final_sfx.exe -lpthread -lShell32";
    system(BuildCommand.c_str());

    std::cout << "SFX executable generated: final_sfx.exe\n";
    return 0;
}
