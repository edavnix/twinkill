cmake_minimum_required(VERSION 3.15)
project(twinkill VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ── Find libraries ─────────────────────────────────────────────────────────
# OpenSSL is available on all three platforms:
#   Linux  — libssl-dev (apt) / openssl-devel (dnf/pacman)
#   macOS  — brew install openssl + CMAKE_PREFIX_PATH=/opt/homebrew
#   Windows — mingw-w64-x86_64-openssl (MSYS2, pre-built)

find_package(OpenSSL REQUIRED)

# ── Sources ────────────────────────────────────────────────────────────────
add_executable(twinkill
  src/main.c
  src/args.c
  src/batch.c
  src/scan.c
  src/io.c
  src/platform.c
  src/ui.c
  src/ui_report.c
)

# ── Includes ───────────────────────────────────────────────────────────────
# Headers live directly in include/ — no subdirectory needed for a CLI.
target_include_directories(twinkill PRIVATE
  include
  ${OPENSSL_INCLUDE_DIR}
)

# ── Link ───────────────────────────────────────────────────────────────────
target_link_libraries(twinkill PRIVATE
  OpenSSL::Crypto
)

# ── Compiler warnings ──────────────────────────────────────────────────────
if(MSVC)
  target_compile_options(twinkill PRIVATE /W4)
else()
  target_compile_options(twinkill PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
    -Wformat=2 -Wconversion -Wsign-conversion
    -Wstrict-prototypes -Wmissing-prototypes
  )
  # -static-libgcc is GNU ld only — not supported by Apple clang
  if(NOT APPLE AND NOT MSVC)
    target_link_options(twinkill PRIVATE -static-libgcc)
  endif()
endif()

# ── Sanitizers on Debug (Linux/macOS only) ─────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC AND NOT WIN32)
  target_compile_options(twinkill PRIVATE -fsanitize=address,undefined)
  target_link_options(twinkill    PRIVATE -fsanitize=address,undefined)
endif()

# ── Release: optimize + strip ──────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
  target_compile_options(twinkill PRIVATE -O2)
  if(NOT APPLE)
    target_link_options(twinkill PRIVATE -s)
  endif()
endif()

install(TARGETS twinkill RUNTIME DESTINATION bin)

message(STATUS "twinkill ${PROJECT_VERSION}  |  ${CMAKE_SYSTEM_NAME}  |  ${CMAKE_BUILD_TYPE}")
message(STATUS "  OpenSSL version : ${OPENSSL_VERSION}")
message(STATUS "  OpenSSL include : ${OPENSSL_INCLUDE_DIR}")
message(STATUS "  OpenSSL libs    : ${OPENSSL_LIBRARIES}")
