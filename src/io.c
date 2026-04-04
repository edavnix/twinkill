/**
 * @file io.c
 * @brief File I/O and SHA-256 hashing for Twinkill.
 *
 * Uses OpenSSL's libcrypto for SHA-256 — a battle-tested, audited
 * implementation used across the industry. Available on Linux, macOS
 * and Windows (MSYS2/MinGW via mingw-w64-x86_64-openssl).
 */

#include "tk_io.h"

#include <stdio.h>
#include <string.h>

#include <openssl/evp.h>

/* ── Internal: hex encoding ──────────────────────────────────────────────── */

/** @brief Hex character lookup table. */
static const char HEX[] = "0123456789abcdef";

/**
 * @brief Encodes a byte array as a lowercase hex string.
 * @param src    Binary input.
 * @param src_len Number of bytes.
 * @param dst    Output buffer (must be >= src_len * 2 + 1 bytes).
 */
static void to_hex(const unsigned char *src, int src_len, char *dst) {
  for (int i = 0; i < src_len; i++) {
    dst[i * 2] = HEX[src[i] >> 4];
    dst[i * 2 + 1] = HEX[src[i] & 0x0F];
  }
  dst[src_len * 2] = '\0';
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int tk_io_hash_file(const char *path, char *out_hex) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return -1;

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx) {
    fclose(f);
    return -1;
  }

  if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
    EVP_MD_CTX_free(ctx);
    fclose(f);
    return -1;
  }

  unsigned char buf[65536];
  size_t n;

  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    if (EVP_DigestUpdate(ctx, buf, n) != 1) {
      EVP_MD_CTX_free(ctx);
      fclose(f);
      return -1;
    }
  }
  fclose(f);

  unsigned char digest[EVP_MAX_MD_SIZE];
  unsigned int digest_len = 0;

  if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
    EVP_MD_CTX_free(ctx);
    return -1;
  }

  EVP_MD_CTX_free(ctx);
  to_hex(digest, (int)digest_len, out_hex);
  return 0;
}
