/**
 * @file scan.c
 * @brief Duplicate file detection for Twinkill.
 *
 * Pipeline:
 *   1. Hash every file via tk_io_hash_file() (OpenSSL SHA-256).
 *   2. Sort the file array by hash — O(n log n), brings duplicates adjacent.
 *   3. Walk the sorted array grouping consecutive equal hashes.
 *   4. Within each group, find the most recent mtime → original.
 *   5. Build TkDupGroup structs and populate TkScanResult.
 */

#include "tk_io.h"
#include "tk_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal: sort by hash ──────────────────────────────────────────────── */

/**
 * @brief qsort comparator — sorts TkFileInfo by hash lexicographically.
 * @param a Pointer to first TkFileInfo.
 * @param b Pointer to second TkFileInfo.
 * @return strcmp result on the hash field.
 */
static int cmp_by_hash(const void *a, const void *b) {
  const TkFileInfo *fa = (const TkFileInfo *)a;
  const TkFileInfo *fb = (const TkFileInfo *)b;
  return strcmp(fa->hash, fb->hash);
}

/* ── Internal: find original within a group ──────────────────────────────── */

/**
 * @brief Returns the index of the file with the most recent mtime.
 *
 * @param files Array of file info pointers within the group.
 * @param count Number of entries.
 * @return Index of the most recently modified file.
 */
static int find_original(TkFileInfo **files, int count) {
  int best = 0;
  for (int i = 1; i < count; i++)
    if (files[i]->mtime > files[best]->mtime)
      best = i;
  return best;
}

/* ── Internal: build a single TkDupGroup ─────────────────────────────────── */

/**
 * @brief Allocates and populates a @ref TkDupGroup from a slice of files.
 *
 * The file with the most recent mtime becomes @c original.
 * All others are stored in @c copies.
 *
 * @param slice Array of pointers to files sharing the same hash.
 * @param count Number of files in the slice (must be >= 2).
 * @return Heap-allocated TkDupGroup, or NULL on allocation failure.
 */
static TkDupGroup *build_group(TkFileInfo **slice, int count) {
  TkDupGroup *g = (TkDupGroup *)calloc(1, sizeof(TkDupGroup));
  if (!g)
    return NULL;

  int orig_idx = find_original(slice, count);
  g->original = slice[orig_idx];
  strncpy(g->hash, slice[orig_idx]->hash, sizeof(g->hash) - 1);

  int copy_count = count - 1;
  g->copies = (TkFileInfo **)malloc((size_t)copy_count * sizeof(TkFileInfo *));
  if (!g->copies) {
    free(g);
    return NULL;
  }

  int ci = 0;
  for (int i = 0; i < count; i++) {
    if (i == orig_idx)
      continue;
    g->copies[ci++] = slice[i];
    g->wasted += slice[i]->size;
  }
  g->copy_count = copy_count;

  return g;
}

/* ── Internal: dynamic group list ────────────────────────────────────────── */

/**
 * @brief Appends a group to the result's group array, growing as needed.
 *
 * @param result Result struct to append to.
 * @param group  Group to append.
 * @param cap    Pointer to current array capacity (updated on growth).
 * @return 0 on success, -1 on allocation failure.
 */
static int result_push(TkScanResult *result, TkDupGroup *group, int *cap) {
  if (result->group_count >= *cap) {
    int new_cap = *cap ? *cap * 2 : 16;
    TkDupGroup **tmp = (TkDupGroup **)realloc(
        result->groups, (size_t)new_cap * sizeof(TkDupGroup *));
    if (!tmp)
      return -1;
    result->groups = tmp;
    *cap = new_cap;
  }
  result->groups[result->group_count++] = group;
  return 0;
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int tk_scan_find_duplicates(TkFileInfo *files, int file_count,
                            TkScanResult *result,
                            void (*progress)(const char *path, int index,
                                             int total)) {
  memset(result, 0, sizeof(*result));
  if (file_count == 0)
    return 0;

  /* ── Step 1: hash all files ── */
  for (int i = 0; i < file_count; i++) {
    if (progress)
      progress(files[i].path, i + 1, file_count);

    if (tk_io_hash_file(files[i].path, files[i].hash) != 0)
      files[i].hash[0] = '\0'; /* mark as unreadable */
  }

  /* ── Step 2: sort by hash ── */
  qsort(files, (size_t)file_count, sizeof(TkFileInfo), cmp_by_hash);

  /* ── Step 3: group consecutive equal hashes ── */
  /* Temporary pointer array for current group */
  TkFileInfo **slice =
      (TkFileInfo **)malloc((size_t)file_count * sizeof(TkFileInfo *));
  if (!slice)
    return -1;

  int cap = 0;
  int i = 0;

  while (i < file_count) {
    /* Skip unreadable files (empty hash) */
    if (files[i].hash[0] == '\0') {
      i++;
      continue;
    }

    /* Collect all files sharing files[i].hash */
    int j = i;
    while (j < file_count && strcmp(files[j].hash, files[i].hash) == 0) {
      slice[j - i] = &files[j];
      j++;
    }
    int group_size = j - i;

    if (group_size >= 2) {
      TkDupGroup *g = build_group(slice, group_size);
      if (!g) {
        free(slice);
        return -1;
      }
      if (result_push(result, g, &cap) != 0) {
        free(g->copies);
        free(g);
        free(slice);
        return -1;
      }
      result->dup_files += g->copy_count;
      result->wasted += g->wasted;
    }

    i = j;
  }
  free(slice);

  result->total_files = file_count;
  return 0;
}

void tk_scan_result_free(TkScanResult *result) {
  if (!result)
    return;
  for (int i = 0; i < result->group_count; i++) {
    if (result->groups[i]) {
      free(result->groups[i]->copies);
      free(result->groups[i]);
    }
  }
  free(result->groups);
  memset(result, 0, sizeof(*result));
}
