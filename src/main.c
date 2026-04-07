/**
 * @file main.c
 * @brief Entry point for Twinkill.
 *
 * Handles --version and --help before argument parsing, then
 * initializes the UI layer and delegates all processing to
 * tk_batch_run. No detection or deletion logic lives here.
 */

#include "tk_args.h"
#include "tk_batch.h"
#include "tk_ui.h"

#include <string.h>

/** @brief Application version string. */
#define TK_VERSION "1.0.0"

/**
 * @brief Program entry point.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, 1 on error or invalid arguments.
 */
int main(int argc, char *argv[]) {
  tk_ui_init();

  if (argc < 2) {
    tk_ui_help(TK_VERSION);
    return 1;
  }

  if (strcmp(argv[1], "--version") == 0) {
    tk_ui_version(TK_VERSION);
    return 0;
  }

  if (strcmp(argv[1], "--help") == 0) {
    tk_ui_help(TK_VERSION);
    return 0;
  }

  TkArgs args;
  if (tk_args_parse(&args, argc, argv) != 0) {
    tk_ui_error("invalid command or arguments");
    tk_ui_help(TK_VERSION);
    return 1;
  }

  int r = tk_batch_run(&args);
  tk_args_free(&args);
  return r;
}
