/**
 * @file tk_batch.h
 * @brief Batch orchestration for Twinkill.
 *
 * This module is the single entry point between main() and the
 * processing pipeline. It resolves input paths, triggers the scan,
 * dispatches results to the UI layer, and handles deletion.
 *
 * It has no knowledge of hashing or duplicate detection internals —
 * it delegates entirely to platform.c, scan.c and ui_report.c.
 */

#ifndef TK_BATCH_H
#define TK_BATCH_H

#include "tk_args.h"

/**
 * @brief Runs the full Twinkill pipeline for the given arguments.
 *
 * For @c TK_CMD_SCAN:
 *   - Collects all files under each path in @p args->paths.
 *   - Hashes and groups duplicates via tk_scan_find_duplicates().
 *   - Prints a detailed report via tk_ui_report_scan().
 *   - Prints a summary via tk_ui_scan_summary().
 *
 * For @c TK_CMD_CLEAN:
 *   - Same collection and scan as above.
 *   - If @p args->dry_run is 1, prints what would be deleted.
 *   - Otherwise deletes all copy files, keeping the original.
 *   - Prints a summary via tk_ui_clean_summary().
 *
 * @param args Parsed CLI arguments from tk_args_parse().
 * @return 0 if all operations succeeded, 1 if any error occurred.
 */
int tk_batch_run(const TkArgs *args);

#endif /* TK_BATCH_H */
