#ifndef SFXMAKER_H
#define SFXMAKER_H

#include <string>
#include <vector>

struct FileEntry {
    std::string filename;
    std::vector<char> data;
};

class SFXMaker {
public:
    void createSFX(const std::vector<FileEntry>& files, const std::string& outputExe);
    void extractAndRun();
    
private:
    void writeFiles(std::ostream& out, const std::vector<FileEntry>& files);
    void readFiles(std::istream& in);
};

#endif 