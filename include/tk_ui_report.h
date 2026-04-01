/**
 * @file tk_ui_report.h
 * @brief Formatted report output for Twinkill.
 *
 * This module renders structured data (TkScanResult, TkDupGroup)
 * into human-readable terminal output. It is the only module allowed
 * to format and print duplicate-specific information.
 *
 * All functions receive pure data structs — no file I/O is performed here.
 */

#ifndef TK_UI_REPORT_H
#define TK_UI_REPORT_H

#include "tk_image.h"

/**
 * @brief Prints a detailed report of all duplicate groups found.
 *
 * Used by the @c scan command. For each group displays the original
 * file (most recent) and all its copies with their paths, sizes and
 * modification times.
 *
 * @param result Populated scan result from tk_scan_find_duplicates().
 */
void tk_ui_report_scan(const TkScanResult *result);

/**
 * @brief Prints a single duplicate group.
 *
 * Used internally by tk_ui_report_scan and by batch.c during
 * clean to show which files are being processed per group.
 *
 * @param group  Duplicate group to display.
 * @param index  Group index (1-based) for display.
 * @param total  Total number of groups.
 */
void tk_ui_report_group(const TkDupGroup *group, int index, int total);

#endif /* TK_UI_REPORT_H */
