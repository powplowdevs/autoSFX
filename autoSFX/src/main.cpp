#include "AutoSFXHeader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <filesystem>
#include <csignal>

using namespace std;

//~~CLI colors~~
const string RESET = "\033[0m";
//Normal colors
const string BLACK = "\033[30m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";
//Bold colors
const string BOLD_BLACK = "\033[1;30m";
const string BOLD_RED = "\033[1;31m";
const string BOLD_GREEN = "\033[1;32m";
const string BOLD_YELLOW = "\033[1;33m";
const string BOLD_BLUE = "\033[1;34m";
const string BOLD_MAGENTA = "\033[1;35m";
const string BOLD_CYAN = "\033[1;36m";
const string BOLD_WHITE = "\033[1;37m";

//~~CLI art helper functions~~
//Display the banner
void displayBanner(){
    cout << BLUE << "=========================================" << RESET << endl;
    cout << BLUE << "       AUTO SFX WIZARD - BETA VERSION    " << RESET << endl;
    cout << BLUE << "=========================================" << RESET << endl;
    cout << endl;
}

//Reset terminal
void handleInterrupt(int signal){
    cout << "\033[37m";
    cout << "Program interrupted. Exiting gracefully..." << endl;
    exit(0);
}

//~~SFX helper functions~~
//Read file data as bytes
vector<char> readFile(const string& filePath){
    ifstream file(filePath, ios::binary);
    if (!file){
        cerr << RED << "Error: Could not open file " << filePath << RESET << endl;
        exit(1);
    }
    return vector<char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

//Escape paths for Windows
string escapePath(const string& path){
    string escapedPath = path;
    size_t pos = 0;
    while ((pos = escapedPath.find("\\", pos)) != string::npos){
        escapedPath.replace(pos, 1, "\\\\");
        pos += 2;
    }
    return escapedPath;
}

//Create an SFX 
void createSFX(const string& fileName, const vector<string>& filePaths, const vector<string>& fileExtensions){
    cout << "Creating SFX " << fileName << " with " << filePaths.size() << " files...";

    //Collect byte data
    vector<vector<char>> fileData;
    for (const auto& path : filePaths){
        fileData.push_back(readFile(path));
    }

    //Generate SFX template
    ofstream sfxTemplate("SFXtemplate.cpp");
    sfxTemplate << "#include <iostream>\n";
    sfxTemplate << "#include <fstream>\n";
    sfxTemplate << "#include <vector>\n";
    sfxTemplate << "#include <windows.h>\n\n";
    sfxTemplate << "using namespace std;\n\n";

    //Function to create hidden temp dir
    sfxTemplate << "string createHiddenTempDir(){\n";
    sfxTemplate << "    char tempPath[MAX_PATH];\n";
    sfxTemplate << "    GetTempPathA(MAX_PATH, tempPath);\n";
    sfxTemplate << "    string tempDir = string(tempPath) + \"autoSFX\\\\\";\n";
    sfxTemplate << "    if (!CreateDirectoryA(tempDir.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS){\n";
    sfxTemplate << "        cerr << \"Error: Could not create temporary directory.\" << endl;\n";
    sfxTemplate << "        exit(1);\n";
    sfxTemplate << "    }\n";
    sfxTemplate << "    SetFileAttributesA(tempDir.c_str(), FILE_ATTRIBUTE_HIDDEN);\n";
    sfxTemplate << "    return tempDir;\n";
    sfxTemplate << "}\n\n";

    //Start main function
    sfxTemplate << "int main(){\n";
    sfxTemplate << "    string tempDir = createHiddenTempDir();\n\n";

    //Write file data
    for (size_t i = 0; i < fileData.size(); ++i){
        sfxTemplate << "    const char file" << i + 1 << "[] ={";
        for (size_t j = 0; j < fileData[i].size(); ++j){
            sfxTemplate << static_cast<int>(fileData[i][j]) << ",";
        }
        sfxTemplate << "};\n";
        sfxTemplate << "    ofstream outFile" << i + 1 << "(tempDir + \"output_file_" << i + 1 << fileExtensions[i] << "\", ios::binary);\n";
        sfxTemplate << "    outFile" << i + 1 << ".write(file" << i + 1 << ", sizeof(file" << i + 1 << "));\n";
        sfxTemplate << "    outFile" << i + 1 << ".close();\n\n";
    }

    for (size_t i = 0; i < fileData.size(); ++i){
        sfxTemplate << "    ShellExecute(NULL, \"open\", (tempDir + \"output_file_" << i + 1 << fileExtensions[i] << "\").c_str(), NULL, NULL, SW_SHOWNORMAL);\n";
    }

    sfxTemplate << "    return 0;\n";
    sfxTemplate << "}\n";
    sfxTemplate.close();

    //Compile command
    string buildCommand = "g++ -static -static-libgcc -static-libstdc++ SFXtemplate.cpp -o " + fileName + " -lpthread -lShell32";
    system(buildCommand.c_str());

    cout << GREEN << "SFX executable generated: " << fileName << RESET << endl;
}

//Main
int main(){
    signal(SIGINT, handleInterrupt); //Handle keybord interrupt
    displayBanner();
    cout << YELLOW << "Welcome to the Auto SFX Wizard! Created by Powplowdevs." << RESET << endl;

    int choice = -1;
    
    while (choice != 0)
   {
        cout << YELLOW << "Type '1' to start creating an SFX or '0' to exit." << RESET << endl;
        cin >> choice;

        if (choice == 1){
            int numFiles;
            cout << YELLOW << "Enter the number of files to be in the SFX: " << RESET;
            cin >> numFiles;

            vector<string> filePaths(numFiles);
            vector<string> fileExtensions(numFiles);

            for (int i = 0; i < numFiles; ++i){
                cout << YELLOW << "Enter file path " << i + 1 << ": " << RESET;
                cin >> filePaths[i];
                fileExtensions[i] = filesystem::path(filePaths[i]).extension().string();
            }

            string fileName;
            cout << YELLOW << "Enter output file name (e.g., final_sfx.exe): " << RESET;
            cin >> fileName;

            createSFX(fileName, filePaths, fileExtensions);
        } 
        else if (choice == 0){
            cout << "Quiting..., goodbye!";
        }
        else{
            cout << WHITE << "Other features not yet implemented..." << RESET << endl;
        }
    }

    return 0;
}
