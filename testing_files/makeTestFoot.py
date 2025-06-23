import os
import json

# Paths
OUT_PATH = r"C:\Users\kalid.DESKTOP-TUS9USS.000\OneDrive\Documents\GitHub\autoSFX\testing_files\cat.txt"
STUB_PATH = r"C:\Users\kalid.DESKTOP-TUS9USS.000\OneDrive\Documents\GitHub\autoSFX\build\Debug\stub.exe"

FILES = [
    r"C:\Users\kalid.DESKTOP-TUS9USS.000\OneDrive\Documents\GitHub\autoSFX\testing_files\exe1.exe",
    r"C:\Users\kalid.DESKTOP-TUS9USS.000\OneDrive\Documents\GitHub\autoSFX\testing_files\exe2.exe"
]

# Get stub size
stub_size = os.path.getsize(STUB_PATH)

# Build file table and packed data
file_table = []
current_offset = stub_size  # Important: start after stub
packed_data = b""

curIndex = 0

for file_path in FILES:
    with open(file_path, "rb") as f:
        data = f.read()
    
    size = len(data)
    name = os.path.basename(file_path)
    runCount = 1
    
    file_entry = {
        "name": name,
        "relativePath": name,  # simple — write to current dir
        "offset": current_offset,
        "size": size,
        "compressed": False,
        "runHidden": False,
        "runCount": runCount,
        "runIndex": curIndex
    }

    file_table.append(file_entry)
    
    packed_data += data
    current_offset += size
    curIndex += 1

# Encode JSON
file_table_json = json.dumps(file_table, indent=4).encode("utf-8")
table_size = len(file_table_json)

# Write full test SFX file
with open(OUT_PATH, "wb") as f:
    f.write(packed_data)
    f.write(file_table_json)
    f.write(b"SFXPKGv1")
    f.write(table_size.to_bytes(4, byteorder="little"))

print("Test SFX written:", OUT_PATH)

