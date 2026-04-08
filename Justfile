# Justfile for twinkill
# Documentation: https://just.systems/man/en/

set shell := ["bash", "-euo", "pipefail", "-c"]

# Build config
BUILD_DIR := "build"
BINARY    := BUILD_DIR + "/twinkill"

# Colors
RED    := '\033[0;31m'
GREEN  := '\033[0;32m'
YELLOW := '\033[0;33m'
CYAN   := '\033[0;36m'
END    := '\033[0m'

# Status prefixes
ERROR   := RED    + "error  " + END
INFO    := YELLOW + "info   " + END
SUCCESS := GREEN  + "ok     " + END

default: help

# ── Build ──────────────────────────────────────────────────────────────────

# Build twinkill in debug mode (address + undefined sanitizers)
[no-exit-message]
build:
  #!/usr/bin/env bash
  echo -e "{{INFO}} Building twinkill (debug)..."
  cmake -B {{BUILD_DIR}} \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build {{BUILD_DIR}}
  echo -e "{{SUCCESS}} Binary ready: {{BINARY}}"

# Build twinkill in release mode (optimized, stripped)
[no-exit-message]
release:
  #!/usr/bin/env bash
  echo -e "{{INFO}} Building twinkill (release)..."
  cmake -B {{BUILD_DIR}} -DCMAKE_BUILD_TYPE=Release
  cmake --build {{BUILD_DIR}}
  echo -e "{{SUCCESS}} Binary ready: {{BINARY}}"

# ── Internal: require binary ───────────────────────────────────────────────

_require:
  #!/usr/bin/env bash
  [[ -f "{{BINARY}}" ]] || {
    echo -e "{{ERROR}} Not built yet — run: just build"
    exit 1
  }

# ── User commands ──────────────────────────────────────────────────────────

# Analyze duplicates without modifying any file
[no-exit-message]
scan path: _require
  {{BINARY}} scan "{{path}}"

# Analyze only images
[no-exit-message]
scan-images path: _require
  {{BINARY}} scan "{{path}}" --images

# Analyze only documents
[no-exit-message]
scan-docs path: _require
  {{BINARY}} scan "{{path}}" --docs

# Analyze only videos
[no-exit-message]
scan-videos path: _require
  {{BINARY}} scan "{{path}}" --videos

# Preview what clean would delete without touching files
[no-exit-message]
dry path: _require
  {{BINARY}} clean "{{path}}" --dry

# Delete duplicate files, keep the most recent
[no-exit-message]
clean path: _require
  {{BINARY}} clean "{{path}}"

# ── Maintenance ────────────────────────────────────────────────────────────

# Delete the build directory
[no-exit-message]
wipe:
  #!/usr/bin/env bash
  rm -rf {{BUILD_DIR}}
  echo -e "{{SUCCESS}} Build directory removed"

# Show available recipes
help:
  @echo -e "{{INFO}} twinkill — available recipes:"
  @echo ""
  @echo -e "  {{CYAN}}Build{{END}}"
  @echo -e "    just build                 # Debug build (sanitizers)"
  @echo -e "    just release               # Release build (optimized)"
  @echo -e "    just wipe                  # Delete build directory"
  @echo ""
  @echo -e "  {{CYAN}}Scan (no files modified){{END}}"
  @echo -e "    just scan        <path>    # All file types"
  @echo -e "    just scan-images <path>    # Images only"
  @echo -e "    just scan-docs   <path>    # Documents only"
  @echo -e "    just scan-videos <path>    # Videos only"
  @echo ""
  @echo -e "  {{CYAN}}Clean{{END}}"
  @echo -e "    just dry   <path>          # Preview deletions"
  @echo -e "    just clean <path>          # Delete duplicates"
  @echo ""
  @echo -e "  {{CYAN}}Examples{{END}}"
  @echo -e "    just scan        ./photos/"
  @echo -e "    just scan-images ./downloads/"
  @echo -e "    just dry         ./documents/"
  @echo -e "    just clean       ./photos/"
