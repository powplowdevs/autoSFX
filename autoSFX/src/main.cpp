#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <filesystem>
#include <csignal>

// ~~CLI art helper functions~~
// Display the banner
void displayBanner(){
    std::cout << "=========================================" << std::endl;
    std::cout << "       AUTO SFX WIZARD - BETA VERSION    " << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

// Interrupts
void handleInterrupt(int signal){
    std::cout << "Program interrupted. Exiting..." << std::endl;
    exit(0);
}

// ~~SFX helper functions~~
// Read file data as bytes
std::vector<char> readFile(const std::string& filePath){
    std::ifstream file(filePath, std::ios::binary);
    if (!file){
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        exit(1);
    }
    return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Create an SFX 
void createSFX(const std::string& fileName, const std::vector<std::string>& filePaths, const std::vector<std::string>& fileExtensions, const std::vector<int>& runCounts){
    std::cout << "Creating SFX " << fileName << " with " << filePaths.size() << " files..." << std::endl;

    // Collect byte data
    std::vector<std::vector<char>> fileData;
    for (const auto& path : filePaths){
        fileData.push_back(readFile(path));
    }

    // Generate SFX template
    std::ofstream sfxTemplate("SFXtemplate.cpp");
    sfxTemplate << "#include <iostream>\n";
    sfxTemplate << "#include <fstream>\n";
    sfxTemplate << "#include <vector>\n";
    sfxTemplate << "#include <windows.h>\n\n";
    sfxTemplate << "using namespace std;\n\n";

    // Function to create hidden temp dir
    sfxTemplate << "std::wstring createHiddenTempDir(){\n";
    sfxTemplate << "    char tempPath[MAX_PATH];\n";
    sfxTemplate << "    GetTempPathA(MAX_PATH, tempPath);\n";
    sfxTemplate << "    std::wstring tempDir = std::wstring(tempPath, tempPath + strlen(tempPath)) + L\"autoSFX\\\\\";\n";
    sfxTemplate << "    if (!CreateDirectoryW(tempDir.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS){\n";
    sfxTemplate << "        std::wcerr << L\"Error: Could not create temporary directory.\" << std::endl;\n";
    sfxTemplate << "        exit(1);\n";
    sfxTemplate << "    }\n";
    sfxTemplate << "    SetFileAttributesW(tempDir.c_str(), FILE_ATTRIBUTE_HIDDEN);\n";
    sfxTemplate << "    return tempDir;\n";
    sfxTemplate << "}\n\n";

    // Start main function
    sfxTemplate << "int main(){\n";
    sfxTemplate << "    std::wstring tempDir = createHiddenTempDir();\n    std::string filePath;\n\n";

    // Write file data
    for (size_t i = 0; i < fileData.size(); ++i){
        sfxTemplate << "    const char file" << i + 1 << "[] = {";
        for (size_t j = 0; j < fileData[i].size(); ++j){
            sfxTemplate << static_cast<int>(fileData[i][j]);
            if (j != fileData[i].size() - 1) {
                sfxTemplate << ",";
            }
        }
        sfxTemplate << "};\n";
        
        sfxTemplate << "    filePath = std::string(tempDir.begin(), tempDir.end()) + \"output_file_" << i + 1 << fileExtensions[i] << "\";\n";
        sfxTemplate << "    std::ofstream outFile" << i << "(filePath, std::ios::binary);\n";
        sfxTemplate << "    outFile" << i << ".write(file" << i + 1 << ", sizeof(file" << i + 1 << "));\n";
        sfxTemplate << "    outFile" << i << ".close();\n\n";
    }
    
    sfxTemplate << "    HINSTANCE result;\n";
    
    // loops for each file
    for (size_t i = 0; i < fileData.size(); ++i){
        sfxTemplate << "    for(int i = 0; i < " << runCounts[i] << "; ++i) {\n";
        sfxTemplate << "        result = ShellExecuteW(NULL, L\"open\", (tempDir + L\"output_file_" << i + 1 << fileExtensions[i];
        sfxTemplate << "\").c_str(), NULL, NULL, SW_SHOWNORMAL);\n";
        sfxTemplate << "        if ((intptr_t)result <= 32) {\n";
        sfxTemplate << "            std::wcerr << L\"Error opening file: \" << (tempDir + L\"output_file_" << i + 1 << fileExtensions[i];
        sfxTemplate << "\") << std::endl;\n";
        sfxTemplate << "        }\n";
        sfxTemplate << "        else {\n";
        sfxTemplate << "            std::wcout << L\"Successfully opened: \" << (tempDir + L\"output_file_" << i + 1 << fileExtensions[i];
        sfxTemplate << "\") << std::endl;\n";
        sfxTemplate << "        }\n";
        sfxTemplate << "    }\n";
    }

    sfxTemplate << "    return 0;\n";
    sfxTemplate << "}\n";
    sfxTemplate.close();

    // Compile command
    std::string buildCommand = "g++ -static -static-libgcc -static-libstdc++ SFXtemplate.cpp -o " + fileName + ".exe -lpthread -lShell32";
    system(buildCommand.c_str());

    std::cout << "SFX executable generated: " << fileName << std::endl;
}

// Main
int main(){
    signal(SIGINT, handleInterrupt);
    displayBanner();
    std::cout << "Welcome to the Auto SFX Wizard! Created by Powplowdevs." << std::endl;

    int choice = -1;
    
    while (choice != 0)
   {
        std::cout << "Type '1' to start creating an SFX or '0' to exit: ";
        std::cin >> choice;

        if (choice == 1){
            int numFiles;
            std::cout << "Enter the number of files to be in the SFX: ";
            std::cin >> numFiles;

            std::vector<std::string> filePaths(numFiles);
            std::vector<std::string> fileExtensions(numFiles);
            std::vector<int> runCounts(numFiles);

            for (int i = 0; i < numFiles; ++i){
                std::cout << "Enter file path " << i + 1 << ": ";
                std::cin >> filePaths[i];
                fileExtensions[i] = std::filesystem::path(filePaths[i]).extension().string();

                std::cout << "Enter number of times to run file " << i + 1 << ": ";
                std::cin >> runCounts[i];
            }

            std::string fileName;
            std::cout << "Enter output file name, don't add extensions (e.g. final_sfx): ";
            std::cin >> fileName;

            createSFX(fileName, filePaths, fileExtensions, runCounts);
        } 
        else if (choice == 0){
            std::cout << "Quitting..., goodbye!" << std::endl;
        }
        else{
            std::cout << "Other features not yet implemented..." << std::endl;
        }
    }
}
