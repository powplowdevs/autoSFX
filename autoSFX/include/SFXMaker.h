#ifndef SFXMAKER_H
#define SFXMAKER_H

#include <string>
#include <vector>

class SFXMaker {
public:
    void promptUser();
    void createSFX(const std::vector<std::string>& filePaths, const std::string& outputPath);
private:
    void embedFiles(const std::vector<std::string>& filePaths, const std::string& outputPath);
};

#endif