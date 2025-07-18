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
import zlib

STUB_PATH = "./stub.exe"
STUB_SIZE = os.path.getsize(STUB_PATH)

class fileObj:
    def __init__(self, path, offset, size, compressed, compressionLevel, runHidden, runCount, runIndex):
        self.path = path
        self.name = os.path.basename(self.path)
        self.offset = offset
        self.size = size
        self.sizeCompressed = 0
        self.compressed = compressed
        self.compressionLevel = compressionLevel
        self.runHidden = runHidden
        self.runCount = runCount
        self.runIndex = runIndex
    
    def serialize(self):
        return {
            "name": self.name,
            "relativePath": self.path,
            "offset": self.offset,
            "size": self.size,
            "sizeCompressed": self.sizeCompressed,
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

        # Validate path
        if(not os.path.exists(path)): 
            print("[X] Invalid path")
            continue
        # Folder or file logic
        if(os.path.isdir(path)):
            size = os.path.getsize(path)
            isCompressed = input("[+] Compress this file (y/n)?: ")
            isCompressed = isCompressed.lower() == "y"
            compressionLevel = None
            if(isCompressed): compressionLevel = input("[+] Enter your compression level (1-9 larger=more time and less space): ")
            runCount = int(input("[+] Enter how many times should files in this folder should run on extract: "))
            runIndex = int(input("[+] Enter run order (lower runs first & in alphabetical order): "))
            runHidden = input("[+] Should the files run hidden (y/n)?: ")
            runHidden = runHidden.lower() == "y"

            for file in os.listdir(path):
                newPath = os.path.join(path, file)
                newFile = fileObj(newPath, 0, 0, isCompressed, compressionLevel, runHidden, runCount, runIndex)
                fileList.append(newFile)

                curOffset += size
                index += 1
        else:
            size = os.path.getsize(path)
            isCompressed = input("[+] Compress this file (y/n)?: ")
            isCompressed = isCompressed.lower() == "y"
            compressionLevel = None
            if(isCompressed): compressionLevel = input("[+] Enter your compression level (1-9 larger=more time and less space): ")
            runCount = int(input("[+] Enter how many times this file should run on extract: "))
            runIndex = int(input("[+] Enter run order (lower runs first): "))
            runHidden = input("[+] Should the file run hidden (y/n)?: ")
            runHidden = (runHidden.lower() == "y")

            newFile = fileObj(path, 0, 0, isCompressed, compressionLevel, runHidden, runCount, runIndex)
            fileList.append(newFile)

            curOffset += size
            index += 1

        print("\n---------------------")
        done = input("[+] Add another file? (y/n): ")
        if(done == "n" or done == "N"): askingForFiles = False
        print("\n---------------------")

            
    return fileList

def main():
    response = input("[0] Create new SFX\n[1] Quit\nYour choice: ")
    if(response == "1" or response == "QUIT"): print("Quitting...")
    if(response == "0"):
        #Format [AutoSfxSrouce][packed file data][File tabel][MARKER][file tabel size]
        print("Please enter file information below...")
        fileList = getFileList()
        
        packedData = b""
        curOffset = STUB_SIZE

        # Fill packed data
        for exfile in fileList:
            file = open(exfile.path, "rb")
            data = file.read()
            exfile.size = len(data)
            file.close()
            # Compression
            if exfile.compressed:
                compressed = zlib.compress(data, int(exfile.compressionLevel))
                exfile.sizeCompressed = len(compressed)
                exfile.offset = curOffset
                packedData += compressed
                curOffset += len(compressed)
            else:
                exfile.offset = curOffset
                packedData += data
                curOffset += len(data)
      
        
        fileTableDict = [exfile.serialize() for exfile in fileList]
        fileTable = json.dumps(fileTableDict, indent=4).encode('utf-8')
        fileTableSize = len(fileTable)

        # First write stub.exe data
        file = open(STUB_PATH, "rb")
        data = file.read()
        file.close()
        
        try:
            outputPath = input("[+] Please enter name for output SFX file (without .exe extension): ") + ".exe"
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