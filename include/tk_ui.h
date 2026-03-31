/**
 * @file tk_ui.h
 * @brief Terminal presentation layer for Twinkill.
 *
 * All user-facing output lives here. No other module may print
 * directly to stdout/stderr except through these functions.
 *
 * Color output is automatically disabled when stdout is not a TTY
 * (piped output, CI environments, redirected logs).
 */

#ifndef TK_UI_H
#define TK_UI_H

#include <stddef.h>

/* ── Initialization ──────────────────────────────────────────────────────── */

/**
 * @brief Initializes the UI layer.
 *
 * Detects TTY support and sets color/no-color mode accordingly.
 * Must be called once before any other tk_ui_* function.
 */
void tk_ui_init(void);

/* ── Status messages ─────────────────────────────────────────────────────── */

/**
 * @brief Prints an informational message to stdout.
 * @param msg Message text.
 */
void tk_ui_info(const char *msg);

/**
 * @brief Prints a success message to stdout.
 * @param msg Message text.
 */
void tk_ui_success(const char *msg);

/**
 * @brief Prints a warning message to stdout.
 * @param msg Message text.
 */
void tk_ui_warn(const char *msg);

/**
 * @brief Prints an error message to stderr.
 * @param msg Message text.
 */
void tk_ui_error(const char *msg);

/* ── Progress ────────────────────────────────────────────────────────────── */

/**
 * @brief Prints a scanning progress line for a single file.
 *
 * Overwrites the current line — used during the hashing phase
 * to show activity without flooding the terminal.
 *
 * @param path  File currently being hashed.
 * @param index Current file index (1-based).
 * @param total Total number of files to scan.
 */
void tk_ui_scan_progress(const char *path, int index, int total);

/**
 * @brief Clears the progress line after scanning completes.
 *
 * Called once after tk_ui_scan_progress to leave a clean terminal.
 */
void tk_ui_scan_done(int total);

/**
 * @brief Prints a deletion status line for a single file.
 *
 * @param path    File being deleted.
 * @param success 1 if deletion succeeded, 0 if it failed.
 */
void tk_ui_delete_file(const char *path, int success);

/* ── Summary ─────────────────────────────────────────────────────────────── */

/**
 * @brief Prints the final scan summary line.
 *
 * @param total_files  Total files scanned.
 * @param dup_files    Total duplicate files found.
 * @param group_count  Number of duplicate groups.
 * @param wasted       Total bytes wasted by duplicates.
 */
void tk_ui_scan_summary(int total_files, int dup_files, int group_count,
                        size_t wasted);

/**
 * @brief Prints the final clean summary line.
 *
 * @param deleted  Number of files deleted.
 * @param failed   Number of files that could not be deleted.
 * @param freed    Total bytes freed.
 * @param dry_run  1 if this was a dry-run.
 */
void tk_ui_clean_summary(int deleted, int failed, size_t freed, int dry_run);

/* ── Help ────────────────────────────────────────────────────────────────── */

/**
 * @brief Prints the full help text to stdout.
 * @param version Application version string.
 */
void tk_ui_help(const char *version);

/**
 * @brief Prints the version string to stdout.
 * @param version Application version string.
 */
void tk_ui_version(const char *version);

#endif /* TK_UI_H */
