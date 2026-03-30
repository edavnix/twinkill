/**
 * @file tk_image.h
 * @brief Shared types, enumerations and result structs for Twinkill.
 *
 * This is the central contract between all processing modules (scan.c,
 * filter.c, batch.c) and the presentation layer (ui.c, ui_report.c).
 * No module should define its own result types — use these instead.
 */

#ifndef TK_IMAGE_H
#define TK_IMAGE_H

#include <stddef.h>
#include <time.h>

/* ── File type categories ───────────────────────────────────────────────────
 */

/**
 * @enum TkFileType
 * @brief Broad category of a file, used for --images / --docs / --videos.
 */
typedef enum {
  TK_FILE_UNKNOWN = 0, /**< Unrecognized or uncategorized file. */
  TK_FILE_IMAGE,       /**< Image: JPEG, PNG, WebP, GIF, BMP, TIFF, etc. */
  TK_FILE_DOCUMENT,    /**< Document: PDF, DOCX, XLSX, TXT, MD, etc.      */
  TK_FILE_VIDEO        /**< Video: MP4, MKV, AVI, MOV, WebM, etc.         */
} TkFileType;

/* ── Filter mode ────────────────────────────────────────────────────────────
 */

/**
 * @enum TkFilterMode
 * @brief Scope of files to consider during scan and clean.
 */
typedef enum {
  TK_FILTER_ALL = 0, /**< Consider all file types (default). */
  TK_FILTER_IMAGES,  /**< Consider only image files.         */
  TK_FILTER_DOCS,    /**< Consider only document files.      */
  TK_FILTER_VIDEOS   /**< Consider only video files.         */
} TkFilterMode;

/* ── Single file info ───────────────────────────────────────────────────────
 */

/**
 * @struct TkFileInfo
 * @brief Metadata for a single file collected during scanning.
 *
 * Populated by tk_io_hash_file() and platform collection.
 * Used to build duplicate groups in TkDupGroup.
 */
typedef struct {
  char path[4096]; /**< Absolute file path.                        */
  char hash[65];   /**< SHA-256 hex digest (64 chars + null).      */
  size_t size;     /**< File size in bytes.                        */
  time_t mtime;    /**< Last modification time (Unix timestamp).   */
  TkFileType type; /**< Detected file category.                    */
} TkFileInfo;

/* ── Duplicate group ────────────────────────────────────────────────────────
 */

/**
 * @struct TkDupGroup
 * @brief A group of files sharing the same SHA-256 hash.
 *
 * The file with the most recent mtime is designated as the original.
 * All other files in the group are duplicates (copies).
 *
 * Populated by tk_scan_find_duplicates().
 * Passed to ui_report and batch for display and deletion.
 */
typedef struct {
  char hash[65];        /**< Shared SHA-256 hash of all files.        */
  TkFileInfo *original; /**< Pointer to the most recent file (kept).  */
  TkFileInfo **copies;  /**< Array of pointers to duplicate files.    */
  int copy_count;       /**< Number of duplicates in this group.      */
  size_t wasted;        /**< Total bytes wasted by copies.            */
} TkDupGroup;

/* ── Scan result ────────────────────────────────────────────────────────────
 */

/**
 * @struct TkScanResult
 * @brief Summary of a full directory scan.
 *
 * Populated by tk_scan_find_duplicates().
 * Passed to ui_report for display and to batch for processing.
 */
typedef struct {
  TkDupGroup **groups; /**< Array of duplicate groups found.        */
  int group_count;     /**< Number of groups.                       */
  int total_files;     /**< Total files scanned.                    */
  int dup_files;       /**< Total duplicate files found.            */
  size_t wasted;       /**< Total bytes wasted by all duplicates.   */
} TkScanResult;

/* ── Clean result ───────────────────────────────────────────────────────────
 */

/**
 * @struct TkCleanResult
 * @brief Outcome of a clean or dry-run operation.
 *
 * Populated by tk_batch_run() after processing all duplicate groups.
 * Passed to ui_report for display.
 */
typedef struct {
  int dry_run;  /**< 1 if no files were actually deleted.       */
  int deleted;  /**< Number of files deleted.                   */
  int failed;   /**< Number of files that could not be deleted. */
  size_t freed; /**< Total bytes freed by deletions.            */
} TkCleanResult;

#endif /* TK_IMAGE_H */
