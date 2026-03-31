/**
 * @file ui.c
 * @brief Terminal presentation layer for Twinkill.
 */

#include "tk_ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#else
#include <unistd.h>
#endif

/* ── Internal: color state ───────────────────────────────────────────────── */

static int tk_color = 0;

#define COL_RESET "\033[0m"
#define COL_BOLD "\033[1m"
#define COL_RED "\033[31m"
#define COL_GREEN "\033[32m"
#define COL_YELLOW "\033[33m"
#define COL_CYAN "\033[36m"
#define COL_GRAY "\033[90m"
#define COL_WHITE "\033[97m"

#define BG_RED "\033[41m\033[97m"
#define BG_GREEN "\033[42m\033[30m"
#define BG_YELLOW "\033[43m\033[30m"
#define BG_CYAN "\033[46m\033[30m"

/**
 * @brief Returns an ANSI code if color is enabled, empty string otherwise.
 * @param code ANSI escape sequence.
 * @return The code or "".
 */
static const char *c(const char *code) { return tk_color ? code : ""; }

/* ── Internal: byte formatting ───────────────────────────────────────────── */

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

/* ── Public API ──────────────────────────────────────────────────────────── */

void tk_ui_init(void) { tk_color = isatty(STDOUT_FILENO); }

void tk_ui_info(const char *msg) {
  if (tk_color)
    printf("%s info %s  %s\n", BG_CYAN, COL_RESET, msg);
  else
    printf("info   %s\n", msg);
}

void tk_ui_success(const char *msg) {
  if (tk_color)
    printf("%s  ok  %s  %s\n", BG_GREEN, COL_RESET, msg);
  else
    printf("ok     %s\n", msg);
}

void tk_ui_warn(const char *msg) {
  if (tk_color)
    printf("%s warn %s  %s\n", BG_YELLOW, COL_RESET, msg);
  else
    printf("warn   %s\n", msg);
}

void tk_ui_error(const char *msg) {
  if (tk_color)
    fprintf(stderr, "%s error %s  %s\n", BG_RED, COL_RESET, msg);
  else
    fprintf(stderr, "error  %s\n", msg);
}

void tk_ui_scan_progress(const char *path, int index, int total) {
  if (!tk_color) {
    printf("scanning [%d/%d]  %s\n", index, total, path);
    return;
  }
  /* Overwrite current line — keeps terminal clean during long scans */
  printf("\r%s[%d/%d]%s  hashing  %s%-60.60s%s", c(COL_GRAY), index, total,
         c(COL_RESET), c(COL_CYAN), path, c(COL_RESET));
  fflush(stdout);
}

void tk_ui_scan_done(int total) {
  if (tk_color) {
    /* Clear the progress line */
    printf("\r%-80s\r", "");
    fflush(stdout);
  }
  char msg[64];
  snprintf(msg, sizeof(msg), "scanned %d file(s)", total);
  tk_ui_success(msg);
}

void tk_ui_delete_file(const char *path, int success) {
  if (success)
    printf("  %s-%s  %s\n", c(COL_GREEN), c(COL_RESET), path);
  else
    printf("  %s!%s  %s%s%s\n", c(COL_RED), c(COL_RESET), c(COL_RED), path,
           c(COL_RESET));
}

void tk_ui_scan_summary(int total_files, int dup_files, int group_count,
                        size_t wasted) {
  char b_wasted[32];
  fmt_bytes(wasted, b_wasted, sizeof(b_wasted));

  printf("\n");
  if (dup_files == 0) {
    printf("%s done %s  %s%d%s file(s) scanned  "
           "%sno duplicates found%s\n",
           c(BG_GREEN), c(COL_RESET), c(COL_BOLD), total_files, c(COL_RESET),
           c(COL_GREEN), c(COL_RESET));
  } else {
    printf("%s done %s  %s%d%s file(s) scanned  "
           "%s%d duplicate(s)%s in %s%d group(s)%s  "
           "%s%s wasted%s\n",
           c(BG_YELLOW), c(COL_RESET), c(COL_BOLD), total_files, c(COL_RESET),
           c(COL_RED), dup_files, c(COL_RESET), c(COL_BOLD), group_count,
           c(COL_RESET), c(COL_YELLOW), b_wasted, c(COL_RESET));
  }
}

void tk_ui_clean_summary(int deleted, int failed, size_t freed, int dry_run) {
  char b_freed[32];
  fmt_bytes(freed, b_freed, sizeof(b_freed));

  printf("\n");
  if (dry_run) {
    printf("%s done %s  %s%d%s file(s) would be deleted  "
           "%s%s would be freed%s\n",
           c(BG_CYAN), c(COL_RESET), c(COL_BOLD), deleted, c(COL_RESET),
           c(COL_CYAN), b_freed, c(COL_RESET));
    return;
  }

  if (failed == 0) {
    printf("%s done %s  %s%d%s file(s) deleted  "
           "%s%s freed%s\n",
           c(BG_GREEN), c(COL_RESET), c(COL_BOLD), deleted, c(COL_RESET),
           c(COL_GREEN), b_freed, c(COL_RESET));
  } else {
    printf("%s done %s  %s%d%s deleted  "
           "%s%d failed%s  "
           "%s%s freed%s\n",
           c(BG_YELLOW), c(COL_RESET), c(COL_BOLD), deleted, c(COL_RESET),
           c(COL_RED), failed, c(COL_RESET), c(COL_GREEN), b_freed,
           c(COL_RESET));
  }
}

void tk_ui_help(const char *version) {
  printf("%stwinkill%s %s — duplicate file detector and remover\n\n",
         c(COL_BOLD), c(COL_RESET), version);

  printf("%sUSAGE%s\n", c(COL_BOLD), c(COL_RESET));
  printf("  twinkill <command> [options] <path>\n\n");

  printf("%sCOMMANDS%s\n", c(COL_BOLD), c(COL_RESET));
  printf("  %sscan%s   <path>          "
         "%sAnalyze duplicates — no files modified%s\n",
         c(COL_CYAN), c(COL_RESET), c(COL_GRAY), c(COL_RESET));
  printf("  %sclean%s  <path>          "
         "%sDelete duplicates, keep most recent%s\n",
         c(COL_CYAN), c(COL_RESET), c(COL_GRAY), c(COL_RESET));
  printf("  %sclean%s  <path> %s--dry%s   "
         "%sPreview what clean would delete%s\n",
         c(COL_CYAN), c(COL_RESET), c(COL_YELLOW), c(COL_RESET), c(COL_GRAY),
         c(COL_RESET));

  printf("\n%sOPTIONS%s\n", c(COL_BOLD), c(COL_RESET));
  printf("  %s--dry%s       Preview deletions without removing any file\n",
         c(COL_YELLOW), c(COL_RESET));
  printf("  %s--images%s    Consider only image files\n", c(COL_YELLOW),
         c(COL_RESET));
  printf("  %s--docs%s      Consider only document files\n", c(COL_YELLOW),
         c(COL_RESET));
  printf("  %s--videos%s    Consider only video files\n", c(COL_YELLOW),
         c(COL_RESET));
  printf("  %s--version%s   Print version and exit\n", c(COL_YELLOW),
         c(COL_RESET));
  printf("  %s--help%s      Print this help and exit\n\n", c(COL_YELLOW),
         c(COL_RESET));

  printf("%sEXAMPLES%s\n", c(COL_BOLD), c(COL_RESET));
  printf("  twinkill scan   ./photos/\n");
  printf("  twinkill clean  ./photos/\n");
  printf("  twinkill clean  ./photos/  --dry\n");
  printf("  twinkill clean  ./downloads/ --images\n");
  printf("  twinkill clean  ./documents/ --docs\n\n");
}

void tk_ui_version(const char *version) { printf("twinkill %s\n", version); }
