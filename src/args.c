/**
 * @file args.c
 * @brief Command-line argument parsing for Twinkill.
 */

#include "tk_args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal: command parsing ───────────────────────────────────────────── */

/**
 * @brief Maps a command string to its @ref TkCommand value.
 * @param str Command string from argv.
 * @return Matching @ref TkCommand, or @c TK_CMD_UNKNOWN if unrecognized.
 */
static TkCommand parse_command(const char *str) {
  if (strcmp(str, "scan") == 0)
    return TK_CMD_SCAN;
  if (strcmp(str, "clean") == 0)
    return TK_CMD_CLEAN;
  return TK_CMD_UNKNOWN;
}

/* ── Internal: option parsing ────────────────────────────────────────────── */

/**
 * @brief Maps a filter flag string to its @ref TkFilterMode value.
 * @param str Flag string from argv (e.g. "--images").
 * @return Matching @ref TkFilterMode, or @c TK_FILTER_ALL if unrecognized.
 */
static TkFilterMode parse_filter(const char *str) {
  if (strcmp(str, "--images") == 0)
    return TK_FILTER_IMAGES;
  if (strcmp(str, "--docs") == 0)
    return TK_FILTER_DOCS;
  if (strcmp(str, "--videos") == 0)
    return TK_FILTER_VIDEOS;
  return TK_FILTER_ALL;
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int tk_args_parse(TkArgs *args, int argc, char *argv[]) {
  memset(args, 0, sizeof(*args));
  args->filter = TK_FILTER_ALL;

  if (argc < 2)
    return -1;

  args->command = parse_command(argv[1]);
  if (args->command == TK_CMD_UNKNOWN)
    return -1;

  /* Pre-allocate paths array — worst case all remaining argv are paths */
  args->paths = (char **)malloc((size_t)(argc - 2) * sizeof(char *));
  if (!args->paths)
    return -1;

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--dry") == 0) {
      args->dry_run = 1;
    } else if (strcmp(argv[i], "--images") == 0 ||
               strcmp(argv[i], "--docs") == 0 ||
               strcmp(argv[i], "--videos") == 0) {
      args->filter = parse_filter(argv[i]);
    } else {
      /* Treat anything not a known flag as a path */
      args->paths[args->path_count++] = argv[i];
    }
  }

  /* Require at least one path */
  if (args->path_count == 0) {
    free(args->paths);
    args->paths = NULL;
    return -1;
  }

  return 0;
}

void tk_args_free(TkArgs *args) {
  free(args->paths);
  args->paths = NULL;
  args->path_count = 0;
}
