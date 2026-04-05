/**
 * @file tk_scan.h
 * @brief Duplicate file detection for Twinkill.
 *
 * This module orchestrates the full duplicate detection pipeline:
 * hash all collected files, group them by SHA-256 digest, identify
 * the original (most recent mtime) within each group, and return
 * a structured result ready for display or deletion.
 *
 * No output is printed here — results are passed to ui_report.
 */

#ifndef TK_SCAN_H
#define TK_SCAN_H

#include "tk_image.h"

/**
 * @brief Scans a flat file list and finds all duplicate groups.
 *
 * Pipeline:
 *   1. Hash each file via tk_io_hash_file().
 *   2. Group files by identical SHA-256 digest.
 *   3. Within each group, designate the file with the most recent
 *      mtime as the original — all others are copies.
 *   4. Populate a @ref TkScanResult with all groups and totals.
 *
 * Groups with only one file (no duplicates) are discarded.
 *
 * The caller must release the result with @ref tk_scan_result_free
 * when done.
 *
 * @param files      Array of file info structs to scan.
 * @param file_count Number of entries in @p files.
 * @param result     Output struct to populate.
 * @param progress   Optional callback invoked after each file is hashed.
 *                   Receives the file path, current index (1-based) and
 *                   total count. Pass NULL to disable progress reporting.
 * @return 0 on success, -1 on allocation failure.
 */
int tk_scan_find_duplicates(TkFileInfo *files, int file_count,
                            TkScanResult *result,
                            void (*progress)(const char *path, int index,
                                             int total));

/**
 * @brief Releases all heap memory allocated inside a @ref TkScanResult.
 *
 * Does not free the result struct itself — only its internal arrays.
 *
 * @param result Result to free.
 */
void tk_scan_result_free(TkScanResult *result);

#endif /* TK_SCAN_H */
