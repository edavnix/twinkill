/**
 * @file platform.c
 * @brief Cross-platform filesystem utilities for Twinkill.
 */

#include "tk_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

/* ── Internal: extension tables ─────────────────────────────────────────── */

static const char *const EXT_IMAGES[] = {
    ".jpg", ".jpeg", ".png",  ".webp", ".gif", ".bmp", ".tiff",
    ".tif", ".heic", ".avif", ".svg",  ".ico", NULL};

static const char *const EXT_DOCS[] = {
    ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx", ".txt",
    ".md",  ".csv", ".odt",  ".ods", ".odp",  ".rtf", ".epub", NULL};

static const char *const EXT_VIDEOS[] = {
    ".mp4", ".mkv", ".avi", ".mov", ".webm", ".flv",  ".wmv",
    ".m4v", ".ts",  ".3gp", ".ogv", ".mpg",  ".mpeg", NULL};

/* ── Internal: extension helpers ─────────────────────────────────────────── */

/**
 * @brief Returns the file extension in lowercase, or NULL if none.
 *
 * Writes the lowercase extension into @p buf. The dot is included.
 *
 * @param path File path.
 * @param buf  Output buffer for the lowercase extension.
 * @param sz   Buffer size.
 * @return Pointer to @p buf, or NULL if no extension found.
 */
static const char *get_ext(const char *path, char *buf, size_t sz) {
  const char *dot = strrchr(path, '.');
  if (!dot || dot == path)
    return NULL;

  size_t i = 0;
  while (dot[i] && i < sz - 1) {
    char ch = dot[i];
    buf[i] = (ch >= 'A' && ch <= 'Z') ? (char)(ch + 32) : ch;
    i++;
  }
  buf[i] = '\0';
  return buf;
}

/**
 * @brief Checks if @p ext matches any extension in @p table.
 * @param ext   Lowercase extension string including dot.
 * @param table NULL-terminated array of extension strings.
 * @return 1 if matched, 0 otherwise.
 */
static int ext_match(const char *ext, const char *const *table) {
  for (int i = 0; table[i]; i++)
    if (strcmp(ext, table[i]) == 0)
      return 1;
  return 0;
}

/* ── Internal: dynamic file list ─────────────────────────────────────────── */

/**
 * @brief Growable list of TkFileInfo entries.
 */
typedef struct {
  TkFileInfo *entries; /**< Heap-allocated array of file info structs. */
  int count;           /**< Number of entries stored. */
  int cap;             /**< Allocated capacity. */
} TkFileList;

/**
 * @brief Appends a new entry to the file list, doubling capacity on growth.
 * @param fl   List to append to.
 * @param info File info to copy into the list.
 * @return 0 on success, -1 on allocation failure.
 */
static int fl_push(TkFileList *fl, const TkFileInfo *info) {
  if (fl->count >= fl->cap) {
    int new_cap = fl->cap ? fl->cap * 2 : 256;
    TkFileInfo *tmp = (TkFileInfo *)realloc(
        fl->entries, (size_t)new_cap * sizeof(TkFileInfo));
    if (!tmp)
      return -1;
    fl->entries = tmp;
    fl->cap = new_cap;
  }
  fl->entries[fl->count++] = *info;
  return 0;
}

/* ── Internal: recursive collection ─────────────────────────────────────── */

#ifdef _WIN32

/**
 * @brief Recursively collects files under @p dir on Windows.
 * @param dir    Root directory.
 * @param filter File type filter.
 * @param fl     File list to populate.
 */
static void collect_win(const char *dir, TkFilterMode filter, TkFileList *fl) {
  char pattern[4096];
  snprintf(pattern, sizeof(pattern), "%s\\*", dir);

  WIN32_FIND_DATAA fd;
  HANDLE h = FindFirstFileA(pattern, &fd);
  if (h == INVALID_HANDLE_VALUE)
    return;

  do {
    if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0 ||
        fd.cFileName[0] == '.')
      continue;

    char full[4096];
    snprintf(full, sizeof(full), "%s\\%s", dir, fd.cFileName);

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      collect_win(full, filter, fl);
      continue;
    }

    TkFileType ftype = tk_platform_file_type(full);

    /* Apply filter */
    if (filter == TK_FILTER_IMAGES && ftype != TK_FILE_IMAGE)
      continue;
    if (filter == TK_FILTER_DOCS && ftype != TK_FILE_DOCUMENT)
      continue;
    if (filter == TK_FILTER_VIDEOS && ftype != TK_FILE_VIDEO)
      continue;

    struct stat st;
    if (stat(full, &st) != 0)
      continue;

    TkFileInfo info;
    memset(&info, 0, sizeof(info));
    snprintf(info.path, sizeof(info.path), "%s", full);
    info.size = (size_t)st.st_size;
    info.mtime = st.st_mtime;
    info.type = ftype;

    fl_push(fl, &info);

  } while (FindNextFileA(h, &fd));

  FindClose(h);
}

#else

/**
 * @brief Recursively collects files under @p dir on POSIX systems.
 *
 * Hidden entries (names starting with '.') are skipped.
 *
 * @param dir    Root directory.
 * @param filter File type filter.
 * @param fl     File list to populate.
 */
static void collect_posix(const char *dir, TkFilterMode filter,
                          TkFileList *fl) {
  DIR *d = opendir(dir);
  if (!d)
    return;

  struct dirent *entry;
  while ((entry = readdir(d)) != NULL) {
    if (entry->d_name[0] == '.')
      continue;

    char full[4096];
    snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);

    struct stat st;
    if (stat(full, &st) != 0)
      continue;

    if (S_ISDIR(st.st_mode)) {
      collect_posix(full, filter, fl);
      continue;
    }

    if (!S_ISREG(st.st_mode))
      continue;

    TkFileType ftype = tk_platform_file_type(full);

    /* Apply filter */
    if (filter == TK_FILTER_IMAGES && ftype != TK_FILE_IMAGE)
      continue;
    if (filter == TK_FILTER_DOCS && ftype != TK_FILE_DOCUMENT)
      continue;
    if (filter == TK_FILTER_VIDEOS && ftype != TK_FILE_VIDEO)
      continue;

    TkFileInfo info;
    memset(&info, 0, sizeof(info));
    snprintf(info.path, sizeof(info.path), "%s", full);
    info.size = (size_t)st.st_size;
    info.mtime = st.st_mtime;
    info.type = ftype;

    fl_push(fl, &info);
  }

  closedir(d);
}

#endif

/* ── Public API ──────────────────────────────────────────────────────────── */

TkFileType tk_platform_file_type(const char *path) {
  char ext[32];
  if (!get_ext(path, ext, sizeof(ext)))
    return TK_FILE_UNKNOWN;
  if (ext_match(ext, EXT_IMAGES))
    return TK_FILE_IMAGE;
  if (ext_match(ext, EXT_DOCS))
    return TK_FILE_DOCUMENT;
  if (ext_match(ext, EXT_VIDEOS))
    return TK_FILE_VIDEO;
  return TK_FILE_UNKNOWN;
}

int tk_platform_is_dir(const char *path) {
  struct stat st;
  if (stat(path, &st) != 0)
    return 0;
  return S_ISDIR(st.st_mode) ? 1 : 0;
}

int tk_platform_collect_files(const char *dir, TkFilterMode filter,
                              TkFileInfo **out_files, int *out_count) {
  TkFileList fl = {NULL, 0, 0};

#ifdef _WIN32
  collect_win(dir, filter, &fl);
#else
  collect_posix(dir, filter, &fl);
#endif

  *out_files = fl.entries;
  *out_count = fl.count;
  return 0;
}

int tk_platform_delete(const char *path) {
#ifdef _WIN32
  return DeleteFileA(path) ? 0 : -1;
#else
  return unlink(path);
#endif
}
