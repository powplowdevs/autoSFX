#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>

std::vector<char> readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return buffer;
}

int main() {
    int numFiles;
    std::cout << "Enter the number of files to be in the SFX: ";
    std::cin >> numFiles;

    std::vector<std::string> filePaths(numFiles);
    for (int i = 0; i < numFiles; ++i) {
        std::cout << "Enter file path " << i + 1 << ": ";
        std::cin >> filePaths[i];
    }

    std::vector<std::vector<char>> fileData;
    for (const auto& path : filePaths) {
        fileData.push_back(readFile(path));
    }

    std::ofstream sfxTemplate("SFXtemplate.cpp");
    sfxTemplate << "#include <iostream>\n#include <fstream>\n#include <vector>\n\n";
    sfxTemplate << "int main() {\n";

    for (size_t i = 0; i < fileData.size(); ++i) {
        sfxTemplate << "    // File " << i + 1 << "\n";
        sfxTemplate << "    const char file" << i + 1 << "[] = {";
        for (size_t j = 0; j < fileData[i].size(); ++j) {
            sfxTemplate << static_cast<int>(fileData[i][j]) << ",";
        }
        sfxTemplate << "};\n";
        sfxTemplate << "    std::ofstream outFile" << i + 1 << "(\"output_file_" << i + 1 << "\", std::ios::binary);\n";
        sfxTemplate << "    outFile" << i + 1 << ".write(file" << i + 1 << ", sizeof(file" << i + 1 << "));\n";
        sfxTemplate << "    outFile" << i + 1 << ".close();\n\n";
    }

    sfxTemplate << "    return 0;\n}\n";
    sfxTemplate.close();

    std::string compileCommand = "g++ -o generated_sfx.exe SFXtemplate.cpp";
    system(compileCommand.c_str());

    std::cout << "SFX executable generated: generated_sfx.exe\n";
    return 0;
}
