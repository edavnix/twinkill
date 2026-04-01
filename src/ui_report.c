/**
 * @file ui_report.c
 * @brief Formatted report output for Twinkill.
 */

#include "tk_ui.h"
#include "tk_ui_report.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ── Internal: color state ───────────────────────────────────────────────── */

#define COL_RESET "\033[0m"
#define COL_BOLD "\033[1m"
#define COL_RED "\033[31m"
#define COL_GREEN "\033[32m"
#define COL_YELLOW "\033[33m"
#define COL_CYAN "\033[36m"
#define COL_GRAY "\033[90m"

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define STDOUT_FILENO 1
#else
#include <unistd.h>
#endif

/**
 * @brief Returns an ANSI code if stdout is a TTY, empty string otherwise.
 * @param code ANSI escape sequence.
 * @return The code or "".
 */
static const char *c(const char *code) {
  return isatty(STDOUT_FILENO) ? code : "";
}

/* ── Internal: formatting helpers ───────────────────────────────────────── */

/**
 * @brief Formats a byte count into a human-readable string.
 * @param bytes Byte count.
 * @param buf   Output buffer.
 * @param sz    Buffer size.
 */
static void fmt_bytes(size_t bytes, char *buf, size_t sz) {
  if (bytes < 1024)
    snprintf(buf, sz, "%zu B", bytes);
  else if (bytes < 1024 * 1024)
    snprintf(buf, sz, "%.1f KB", (double)bytes / 1024.0);
  else if (bytes < 1024 * 1024 * 1024)
    snprintf(buf, sz, "%.2f MB", (double)bytes / (1024.0 * 1024.0));
  else
    snprintf(buf, sz, "%.2f GB", (double)bytes / (1024.0 * 1024.0 * 1024.0));
}

/**
 * @brief Formats a Unix timestamp into a readable date string.
 * @param t   Unix timestamp.
 * @param buf Output buffer.
 * @param sz  Buffer size.
 */
static void fmt_time(time_t t, char *buf, size_t sz) {
  struct tm *tm_info = localtime(&t);
  if (tm_info)
    strftime(buf, sz, "%Y-%m-%d %H:%M", tm_info);
  else
    snprintf(buf, sz, "unknown");
}

/**
 * @brief Truncates a path for display, keeping the last N characters.
 *
 * If the path is longer than @p max, prefixes with "...".
 *
 * @param path Full path string.
 * @param max  Maximum display length.
 * @return Pointer into path at the truncation point, or the full path.
 */
static const char *fmt_path(const char *path, int max) {
  int len = (int)strlen(path);
  if (len <= max)
    return path;
  return path + (len - max + 3); /* caller prefixes "..." */
}

/* ── Public API ──────────────────────────────────────────────────────────── */

void tk_ui_report_group(const TkDupGroup *group, int index, int total) {
  char b_size[32], b_wasted[32], b_time[32];
  fmt_bytes(group->original->size, b_size, sizeof(b_size));
  fmt_bytes(group->wasted, b_wasted, sizeof(b_wasted));

  /* ── Group header ── */
  printf("\n%s[%d/%d]%s  %s%s%s  %s%s%s  %swasted %s%s\n", c(COL_GRAY), index,
         total, c(COL_RESET), c(COL_BOLD), group->hash, c(COL_RESET),
         c(COL_CYAN), b_size, c(COL_RESET), c(COL_YELLOW), b_wasted,
         c(COL_RESET));

  /* ── Original ── */
  fmt_time(group->original->mtime, b_time, sizeof(b_time));
  const char *orig_path = group->original->path;
  int orig_len = (int)strlen(orig_path);
  int truncated = orig_len > 72;

  printf("  %s✓ original%s  %s  %s%s%s%s\n", c(COL_GREEN), c(COL_RESET), b_time,
         truncated ? "..." : "", c(COL_BOLD), fmt_path(orig_path, 72),
         c(COL_RESET));

  /* ── Copies ── */
  for (int i = 0; i < group->copy_count; i++) {
    const TkFileInfo *copy = group->copies[i];
    fmt_time(copy->mtime, b_time, sizeof(b_time));
    const char *copy_path = copy->path;
    int copy_len = (int)strlen(copy_path);
    int copy_trunc = copy_len > 72;

    printf("  %s✗ copy%s      %s  %s%s\n", c(COL_RED), c(COL_RESET), b_time,
           copy_trunc ? "..." : "", fmt_path(copy_path, 72));
  }
}

void tk_ui_report_scan(const TkScanResult *result) {
  if (result->group_count == 0)
    return;

  printf("\n%sDuplicate groups%s\n", c(COL_BOLD), c(COL_RESET));

  for (int i = 0; i < result->group_count; i++)
    tk_ui_report_group(result->groups[i], i + 1, result->group_count);

  printf("\n");
}
