<!-- Banner Divider -->
<img align="center" src="https://user-images.githubusercontent.com/73097560/115834477-dbab4500-a447-11eb-908a-139a6edaec5c.gif">

<!-- Project Name -->
<div align="center">
  <h1><code>AutoSFX Wizard</code></h1>
  <h3>Version 1.0 - <em>Beta</em></h3>
</div>

<!-- Typing Banner -->
<p align="center">
  <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&pause=1000&color=00FFA2&center=true&vCenter=true&width=600&lines=Make+your+own+single-file+installer!;Pack+anything+into+a+self-extracting+EXE.;Compress+%2B+Auto-Run+%2B+Easy+Packing.;Beta+version+%7C+More+coming+soon." />
</p>

---

## 📦 Purpose

AutoSFX Wizard is a CLI tool to bundle multiple files or folders into a **single self-extracting executable** (`.exe`).  
Each file supports:
- Optional compression (zlib level 1–9)
- Visibility setting (run hidden or shown)
- Run count and run order (via index)

---

## ⚙️ How it Works

1. `packer.py` reads your file paths and settings  
2. It appends your data and metadata to an unpacked `stub.exe`  
3. The stub reads itself at runtime, extracts the files to `%TEMP%/ASFX_EXTRC`  
4. Each file is executed according to the metadata you set (order, count, visibility)

Format:
```
[stub.exe][compressed/uncompressed data][fileTable JSON][SFXPKGv1][tableSize]
```

---

## 📁 Example

File: `examples/cat_showcase.exe`  
This single EXE extracts and displays a gallery of cat images automatically.  
All assets were packed using AutoSFX Wizard.

---

## ▶️ Usage

Run the `packer.py` script **with the unpacked `stub.exe` located in the same folder**:

```bash
python packer.py
```

Follow the CLI prompts to:
- Select files/folders
- Choose compression and level
- Set run count, run index, and hidden/shown mode

Output: a single `.exe` with all packed logic.

---

## 🔜 Coming Soon

- A **standalone `.exe` version** of the packer (no Python required)

