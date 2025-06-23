# For each file
# Ask for file path (get name from path)
# Ask how many times to run
# Ask if it should be run hidden
# Ask for run Index
# Give option to compress file
#      std::string name;
#     std::wstring path;
#     uint64_t offset;
#     uint64_t size;
#     bool compressed;
#     bool runHidden;
#     uint32_t runCount;
#     uint32_t runIndex;

import os
import json

STUB_PATH = "./stub.exe"
STUB_SIZE = os.path.getsize(STUB_PATH)

class file:
    def __init__(self, path, offset, size, compressed, runHidden, runCount, runIndex):
        self.path = path
        self.name = os.path.basename(self.path)
        self.offset = offset
        self.size = size
        self.compressed = compressed
        self.runHidden = runHidden
        self.runCount = runCount
        self.runIndex = runIndex
    
    def serialize(self):
        return {
            "name": self.name,
            "relativePath": self.path,
            "offset": self.offset,
            "size": self.size,
            "compressed": self.compressed,
            "runHidden": self.runHidden,
            "runCount": self.runCount,
            "runIndex": self.runIndex
        }
    
def getFileList():
    print("\n---------------------")
    askingForFiles = True
    fileList = []
    index = 1
    curOffset = STUB_SIZE
    while(askingForFiles):
        print(f"File {index}:")
        path = input("[+] Enter file path to pack: ")
        size = os.path.getsize(path)
        # TODO COMPRESSION
        runCount = int(input("[+] Enter how many times this file should run on extract: "))
        runIndex = int(input("[+] Enter run order (lower runs first): "))
        runHidden = input("[+] Should the file run hidden (y/n)?: ")
        runHidden = (runHidden.lower() == "y")

        newFile = file(path, size, curOffset, False, runHidden, runCount, runIndex)
        fileList.append(newFile)

        curOffset += size

        print("\n---------------------")
        done = input("[+] Add another file? (y/n): ")
        if(done == "n" or done == "N"): askingForFiles = False
        print("\n---------------------")

        index += 1
        
    return fileList

def main():
    response = input("[0] Create new SFX\n[1] Quit\nYour choice: ")
    if(response == "1" or response == "QUIT"): print("Quitting...")
    if(response == "0"):
        #Format [AutoSfxSrouce][packed file data][File tabel][MARKER][file tabel size]
        print("Please enter file information below...")
        fileList = getFileList()
        
        packedData = b""
        fileTableDict = [exfile.serialize() for exfile in fileList]
        fileTable = json.dumps(fileTableDict, indent=4).encode('utf-8')
        fileTableSize = len(fileTable)

        # Fill packed data
        for exfile in fileList:
            file = open(exfile.path, "rb")
            data = file.read()
            file.close()
            packedData += data

        # First write stub.exe data
        file = open(STUB_PATH, "rb")
        data = file.read()
        file.close()
        
        try:
            outputPath = input("[+] Please enter name for output SFX file (with .exe extension): ")
            with open(outputPath, "wb") as f:
                f.write(data)
                f.write(packedData)
                f.write(fileTable)
                f.write(b"SFXPKGv1")
                f.write(fileTableSize.to_bytes(4, byteorder="little"))
            print(f"[✓] Successfully wrote SFX file: {outputPath}")
        except Exception as e:
            print(f"[X] Failed to write SFX file: {e}")

        


    else: 
        print("invalid response") 
        main()


# Prompt user
print("Welcome to autoSFX wizzard / v1.0")
main()