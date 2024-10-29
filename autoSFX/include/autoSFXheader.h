#ifndef AUTOSFXHEADER_H
#define AUTOSFXHEADER_H

#include <vector>
#include <string>

std::vector<char> readFile(const std::string& filePath);
std::string createHiddenTempDir();
std::string escapePath(const std::string& path);
void createSFX(const std::string& fileName, const std::vector<std::string>& filePaths, const std::vector<std::string>& fileExtensions);

#endif
