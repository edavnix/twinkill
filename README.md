<div align="center">
  <h1>Twinkill</h1>
  <p>Cross-platform CLI tool to <strong>detect</strong> and <strong>remove duplicate files</strong> by content — images, documents, videos, and more</p>

![Twinkill](assets/banner.png)

![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=000&labelColor=fff&color=FCC624)
![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=000&labelColor=fff&color=000000)
![Windows](https://img.shields.io/badge/Windows-0078D4?logo=windows&logoColor=0078D4&labelColor=fff&color=0078D4)
![C](https://img.shields.io/badge/C99-00599C?logo=c&logoColor=00599C&labelColor=fff&color=00599C)

</div>

## Why Twinkill?

Duplicate files accumulate silently — the same photo in three folders, a document copied to the Desktop, a video downloaded twice. Over time they waste gigabytes of storage and make it impossible to know which file is the real one.

Twinkill solves this with a single command — **no GUI, no cloud uploads, no guesswork**. It reads the actual content of every file using SHA-256, groups exact duplicates regardless of filename or location, keeps the most recently modified copy as the original, and removes the rest.

## Features

| Feature                       | Description                                                                     |
| ----------------------------- | ------------------------------------------------------------------------------- |
| **Content-based detection**   | Compares files by SHA-256 hash — filename and extension are irrelevant.         |
| **Duplicate report**          | Displays every group of duplicates with original, copies, sizes and timestamps. |
| **Smart original selection**  | Always keeps the most recently modified file as the original.                   |
| **Dry-run preview**           | Shows exactly what would be deleted without touching any file.                  |
| **Recursive scanning**        | Walks entire directory trees, including nested subdirectories.                  |
| **File type filters**         | Scope the scan to images, documents, or videos with a single flag.              |
| **Multi-path support**        | Scan or clean multiple directories in a single command.                         |
| **Cross-platform**            | Builds and runs on Linux, macOS, and Windows.                                   |
| **Colored terminal output**   | ANSI-colored reports with progress tracking and per-group size summaries.       |
| **Zero runtime dependencies** | Single binary — no interpreters, no external tools required at runtime.         |

## Usage

```bash
twinkill <command> [options] <path>
```

### Commands

| Command                       | Description                                        |
| ----------------------------- | -------------------------------------------------- |
| `twinkill scan  <path>`       | Analyze duplicates — no files modified.            |
| `twinkill clean <path>`       | Delete duplicates, keep most recent.               |
| `twinkill clean <path> --dry` | Preview what clean would delete, no files written. |

### Examples

```bash
# Scan a directory and report all duplicate groups
twinkill scan ./photos/

# Scan only image files
twinkill scan ./downloads/ --images

# Scan only documents
twinkill scan ./documents/ --docs

# Preview what would be deleted without removing anything
twinkill clean ./photos/ --dry

# Delete all duplicate files, keep the most recent copy
twinkill clean ./photos/

# Scan multiple directories at once
twinkill scan ./photos/ ./downloads/ ./backups/
```

### Options

| Option      | Description                                          |
| ----------- | ---------------------------------------------------- |
| `--dry`     | Preview deletions without removing any file.         |
| `--images`  | Consider only image files (JPEG, PNG, WebP, etc.).   |
| `--docs`    | Consider only document files (PDF, DOCX, TXT, etc.). |
| `--videos`  | Consider only video files (MP4, MKV, MOV, etc.).     |
| `--version` | Print version and exit.                              |
| `--help`    | Print help and exit.                                 |

## Installation

### Automatic (recommended)

```bash
# Linux / macOS
curl -fsSL https://raw.githubusercontent.com/tu-usuario/twinkill/main/install.sh | bash

# Windows (PowerShell)
irm https://raw.githubusercontent.com/tu-usuario/twinkill/main/install.ps1 | iex
```

### Manual

Download the binary for your platform from the [Releases](https://github.com/tu-usuario/twinkill/releases) page and place it in any directory on your `PATH`.

| Platform       | Binary                        |
| -------------- | ----------------------------- |
| Linux x86-64   | `twinkill-linux-x86_64`       |
| macOS          | `twinkill-macos-universal`    |
| Windows x86-64 | `twinkill-windows-x86_64.exe` |

## Build from Source

### Requirements

- **C Compiler**: GCC 15.2.1 or higher.
- **CMake**: version 3.15 or higher.
- **Just**: version 1.46.0 or higher.
- **OpenSSL**: libssl-dev (Linux), `brew install openssl` (macOS), `mingw-w64-x86_64-openssl` (Windows/MSYS2).

### Arch Linux / WSL (Recommended)

1. **Clone the repository**:

   ```bash
   git clone https://github.com/tu-usuario/twinkill.git
   ```

   > **Note**: When using Arch Linux natively, proceed to **Step 4**. For Windows environments, complete all steps to install and configure WSL Arch.

2. **Download and install WSL Arch** (PowerShell):

   ```bash
   wsl --install -d archlinux
   ```

3. **Restart the system** and access Arch Linux.

4. **Install dependencies**:

   ```bash
   pacman -Syu
   pacman -S base-devel cmake just openssl
   ```

5. **Verify installation**:

   ```bash
   gcc --version     # e.g. gcc (GCC) 15.2.1
   cmake --version   # e.g. cmake version 3.28.3
   just --version    # e.g. just 1.46.0
   ```

6. **Navigate to the directory**:

   ```bash
   cd twinkill
   ```

7. **Build and run** using `just`:
   ```bash
   just                   # view available recipes
   just build             # debug build (address + undefined sanitizers)
   just release           # release build (optimized, stripped)
   ```

### Available Recipes

```bash
just build                 # Debug build (sanitizers)
just release               # Release build (optimized)
just wipe                  # Delete build directory
just scan        <path>    # Analyze all duplicates
just scan-images <path>    # Analyze image duplicates only
just scan-docs   <path>    # Analyze document duplicates only
just scan-videos <path>    # Analyze video duplicates only
just dry         <path>    # Preview deletions without writing
just clean       <path>    # Delete duplicates, keep most recent
```

## How It Works

1. **Collection** — Walks the directory tree recursively, collecting every regular file. Hidden files and directories are skipped. Optionally filtered by file type using extension matching.
2. **Hashing** — Computes a SHA-256 digest for each file using OpenSSL's `libcrypto`. Files are read in 64 KB chunks — memory usage stays constant regardless of file size.
3. **Grouping** — Sorts all files by hash in O(n log n), then walks the sorted array grouping consecutive identical hashes. Groups with only one file are discarded.
4. **Original selection** — Within each duplicate group, the file with the most recent modification time is designated as the original. All others are copies.
5. **Reporting** — Outputs a colored report per group showing the original (✓) and each copy (✗) with their paths, sizes, and timestamps. A final summary shows total duplicates, groups, and bytes wasted.
6. **Deletion** — In clean mode, removes every copy file via the OS delete API. The original is never touched. Each deletion is reported individually.

## Official Documentation

Additional resources:

- **[OpenSSL EVP API](https://docs.openssl.org/master/man3/EVP_DigestInit/)** - SHA-256 digest interface used for file hashing.
- **[SHA-256 Specification](https://csrc.nist.gov/publications/detail/fips/180/4/final)** - FIPS 180-4 secure hash standard.
- **[CMake Documentation](https://cmake.org/cmake/help/latest/)** - Build system reference.
- **[Just Documentation](https://just.systems/man/en/)** - Command runner reference.

<div align="center">
  <br>
  <img
    src="https://img.shields.io/badge/Made%20with-C%20%26%20SHA--256-00599C?style=for-the-badge"
    alt="Made with C"
  />
  <br><br>
  <p>⭐ <strong>Star this repository to show support</strong> ⭐</p>
</div>
