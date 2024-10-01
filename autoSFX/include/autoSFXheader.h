#ifndef AUTOSFXHEADER_H
#define AUTOSFXHEADER_H

#include <vector>
#include <string>

std::vector<char> readFile(const std::string& filepath);
std::string createHiddenTempDir();
std::string escapePath(const std::string& path);

#endif
