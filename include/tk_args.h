/**
 * @file tk_args.h
 * @brief Command-line argument parsing for Twinkill.
 *
 * This module defines the CLI interface and populates a @ref TkArgs
 * structure from argc/argv. It performs no I/O and has no knowledge
 * of file scanning or the UI layer.
 */

#ifndef TK_ARGS_H
#define TK_ARGS_H

#include "tk_image.h"

/**
 * @enum TkCommand
 * @brief Top-level command provided by the user.
 */
typedef enum {
  TK_CMD_UNKNOWN = 0, /**< No valid command recognized.                  */
  TK_CMD_SCAN,        /**< Analyze duplicates without modifying files.   */
  TK_CMD_CLEAN        /**< Delete duplicate files, keep most recent.     */
} TkCommand;

/**
 * @struct TkArgs
 * @brief Parsed representation of all CLI arguments.
 */
typedef struct {
  TkCommand command;   /**< Top-level command (scan or clean).       */
  TkFilterMode filter; /**< File type filter (default: all).         */
  int dry_run;         /**< If 1, preview deletions without writing. */
  char **paths;        /**< Input directory or file paths.           */
  int path_count;      /**< Number of entries in @p paths.           */
} TkArgs;

/**
 * @brief Parses argc/argv into a @ref TkArgs structure.
 *
 * Recognized commands:
 *   - @c scan  <path>
 *   - @c clean <path>
 *   - @c clean <path> --dry
 *   - @c clean <path> --images
 *   - @c clean <path> --docs
 *   - @c clean <path> --videos
 *
 * The @p paths array points directly into @p argv — no heap allocation
 * is performed for the strings themselves.
 * Call @ref tk_args_free when done to release the array.
 *
 * @param args  Output structure to populate.
 * @param argc  Argument count from main().
 * @param argv  Argument vector from main().
 * @return 0 on success, -1 on invalid or missing arguments.
 */
int tk_args_parse(TkArgs *args, int argc, char *argv[]);

/**
 * @brief Releases heap memory allocated by @ref tk_args_parse.
 * @param args Pointer to the structure to free.
 */
void tk_args_free(TkArgs *args);

#endif /* TK_ARGS_H */
