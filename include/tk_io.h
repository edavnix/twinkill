/**
 * @file tk_io.h
 * @brief File I/O and SHA-256 hashing for Twinkill.
 *
 * This module provides the core hashing function used to identify
 * duplicate files by content. SHA-256 is implemented without any
 * external dependency — pure C99, portable across all platforms.
 */

#ifndef TK_IO_H
#define TK_IO_H

#include <stddef.h>

/**
 * @brief Computes the SHA-256 hash of a file's contents.
 *
 * Reads the file in chunks to handle files of any size without
 * loading them entirely into memory. Writes the result as a
 * null-terminated 64-character hex string into @p out_hex.
 *
 * @param path    Path to the file to hash.
 * @param out_hex Output buffer for the hex digest (must be >= 65 bytes).
 * @return 0 on success, -1 if the file cannot be opened or read.
 */
int tk_io_hash_file(const char *path, char *out_hex);

#endif /* TK_IO_H */
