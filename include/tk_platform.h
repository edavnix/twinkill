/**
 * @file tk_platform.h
 * @brief Cross-platform filesystem utilities for Twinkill.
 *
 * This module abstracts OS-specific operations: directory traversal,
 * recursive file collection, file deletion and metadata retrieval.
 * Supports Linux, macOS, and Windows.
 */

#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

#include "tk_image.h"

/**
 * @brief Checks whether a path points to a directory.
 * @param path Path to check.
 * @return 1 if it is a directory, 0 otherwise.
 */
int tk_platform_is_dir(const char *path);

/**
 * @brief Recursively collects all files under a directory.
 *
 * Walks the directory tree and collects every regular file.
 * Hidden files and directories (names starting with '.') are skipped.
 *
 * The caller is responsible for freeing each @ref TkFileInfo in
 * @p out_files and then freeing the array itself.
 *
 * @param dir       Root directory to search.
 * @param filter    File type filter — only files matching this category
 *                  are collected. @c TK_FILTER_ALL collects everything.
 * @param out_files Output pointer to a heap-allocated array of TkFileInfo.
 * @param out_count Output number of files collected.
 * @return 0 on success, -1 on allocation failure.
 */
int tk_platform_collect_files(const char *dir, TkFilterMode filter,
                              TkFileInfo **out_files, int *out_count);

/**
 * @brief Deletes a single file from the filesystem.
 * @param path Path to the file to delete.
 * @return 0 on success, -1 on failure.
 */
int tk_platform_delete(const char *path);

/**
 * @brief Detects the broad file type category from the file extension.
 *
 * Does not read file contents — uses extension only for speed during
 * collection. Sufficient for --images / --docs / --videos filtering.
 *
 * @param path File path.
 * @return Detected @ref TkFileType.
 */
TkFileType tk_platform_file_type(const char *path);

#endif /* TK_PLATFORM_H */
