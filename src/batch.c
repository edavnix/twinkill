/**
 * @file batch.c
 * @brief Batch orchestration for Twinkill.
 */

#include "tk_batch.h"
#include "tk_platform.h"
#include "tk_scan.h"
#include "tk_ui.h"
#include "tk_ui_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal: multi-path file collection ────────────────────────────────── */

/**
 * @brief Collects files from all paths in @p args into a single flat array.
 *
 * Merges results from multiple directories into one contiguous array.
 * The caller is responsible for freeing @p out_files.
 *
 * @param args       Parsed CLI arguments.
 * @param out_files  Output heap-allocated array of TkFileInfo.
 * @param out_count  Output number of files collected.
 * @return 0 on success, -1 on allocation failure.
 */
static int collect_all(const TkArgs *args, TkFileInfo **out_files,
                       int *out_count) {
  TkFileInfo *all = NULL;
  int total = 0;
  int cap = 0;

  for (int i = 0; i < args->path_count; i++) {
    const char *path = args->paths[i];

    if (!tk_platform_is_dir(path)) {
      char msg[512];
      snprintf(msg, sizeof(msg), "'%s' is not a directory — skipping", path);
      tk_ui_warn(msg);
      continue;
    }

    TkFileInfo *found = NULL;
    int n = 0;

    if (tk_platform_collect_files(path, args->filter, &found, &n) != 0) {
      tk_ui_error("file collection failed");
      free(all);
      return -1;
    }

    if (n == 0) {
      char msg[512];
      snprintf(msg, sizeof(msg), "no files found in '%s'", path);
      tk_ui_warn(msg);
      free(found);
      continue;
    }

    /* Grow combined array */
    if (total + n > cap) {
      int new_cap = (total + n) * 2;
      TkFileInfo *tmp =
          (TkFileInfo *)realloc(all, (size_t)new_cap * sizeof(TkFileInfo));
      if (!tmp) {
        free(found);
        free(all);
        return -1;
      }
      all = tmp;
      cap = new_cap;
    }

    memcpy(all + total, found, (size_t)n * sizeof(TkFileInfo));
    total += n;
    free(found);
  }

  *out_files = all;
  *out_count = total;
  return 0;
}

/* ── Internal: progress callback ─────────────────────────────────────────── */

/**
 * @brief Progress callback passed to tk_scan_find_duplicates().
 * @param path  File currently being hashed.
 * @param index Current index (1-based).
 * @param total Total files.
 */
static void on_progress(const char *path, int index, int total) {
  tk_ui_scan_progress(path, index, total);
}

/* ── Internal: clean duplicate groups ───────────────────────────────────── */

/**
 * @brief Deletes all copy files across all duplicate groups.
 *
 * Prints each deletion result via tk_ui_delete_file().
 * In dry-run mode, prints what would be deleted without touching files.
 *
 * @param result  Scan result containing duplicate groups.
 * @param dry_run If 1, only preview deletions.
 * @param deleted Output: number of files successfully deleted.
 * @param failed  Output: number of files that could not be deleted.
 * @param freed   Output: total bytes freed.
 */
static void clean_groups(const TkScanResult *result, int dry_run, int *deleted,
                         int *failed, size_t *freed) {
  *deleted = 0;
  *failed = 0;
  *freed = 0;

  for (int i = 0; i < result->group_count; i++) {
    const TkDupGroup *g = result->groups[i];

    tk_ui_report_group(g, i + 1, result->group_count);

    for (int j = 0; j < g->copy_count; j++) {
      const TkFileInfo *copy = g->copies[j];

      if (dry_run) {
        tk_ui_delete_file(copy->path, 1);
        (*deleted)++;
        *freed += copy->size;
      } else {
        int ok = (tk_platform_delete(copy->path) == 0);
        tk_ui_delete_file(copy->path, ok);
        if (ok) {
          (*deleted)++;
          *freed += copy->size;
        } else {
          (*failed)++;
        }
      }
    }
  }
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int tk_batch_run(const TkArgs *args) {
  /* ── Collect files ── */
  TkFileInfo *files = NULL;
  int count = 0;

  if (collect_all(args, &files, &count) != 0) {
    tk_ui_error("out of memory during file collection");
    return 1;
  }

  if (count == 0) {
    tk_ui_warn("no files to process");
    free(files);
    return 0;
  }

  char msg[64];
  snprintf(msg, sizeof(msg), "scanning %d file(s)...", count);
  tk_ui_info(msg);

  /* ── Scan ── */
  TkScanResult result;
  if (tk_scan_find_duplicates(files, count, &result, on_progress) != 0) {
    tk_ui_error("scan failed — out of memory");
    free(files);
    return 1;
  }
  tk_ui_scan_done(result.total_files);

  /* ── No duplicates found ── */
  if (result.group_count == 0) {
    tk_ui_scan_summary(result.total_files, 0, 0, 0);
    tk_scan_result_free(&result);
    free(files);
    return 0;
  }

  /* ── Scan command: report only ── */
  if (args->command == TK_CMD_SCAN) {
    tk_ui_report_scan(&result);
    tk_ui_scan_summary(result.total_files, result.dup_files, result.group_count,
                       result.wasted);
    tk_scan_result_free(&result);
    free(files);
    return 0;
  }

  /* ── Clean command ── */
  int deleted = 0;
  int failed = 0;
  size_t freed = 0;

  clean_groups(&result, args->dry_run, &deleted, &failed, &freed);
  tk_ui_clean_summary(deleted, failed, freed, args->dry_run);

  tk_scan_result_free(&result);
  free(files);
  return failed > 0 ? 1 : 0;
}
