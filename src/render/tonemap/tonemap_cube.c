#include "tonemap_cube.h"
#include "../../util/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// tiny line cursor over a fixed buffer. returns the next line into `dst`
// (nul-terminated, trimmed of the newline), advances *pos. returns 0 at eof.
static int next_line(const char *text, size_t len, size_t *pos,
                     char *dst, size_t cap) {
    if (*pos >= len) return 0;
    size_t i = *pos, j = 0;
    while (i < len && text[i] != '\n' && text[i] != '\r') {
        if (j + 1 < cap) dst[j++] = text[i];
        i++;
    }
    // swallow the line terminator (handles \r\n too)
    while (i < len && (text[i] == '\n' || text[i] == '\r')) i++;
    dst[j] = '\0';
    *pos = i;
    return 1;
}

// is the line blank or a comment? .cube comments are '#' to end of line.
static int skippable(const char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return (*s == '\0' || *s == '#');
}

int tonemap_cube_parse(tonemap_lut *out, const char *text, size_t len) {
    // start from a safe identity; if we bail mid-file the caller still has a
    // usable lut.
    tonemap_lut_destroy(out);

    int dim = 0;
    char line[256];
    size_t pos = 0;

    // first pass: find LUT_3D_SIZE so we can allocate before reading data.
    size_t scan = 0;
    while (next_line(text, len, &scan, line, sizeof line)) {
        if (skippable(line)) continue;
        int n = 0;
        if (sscanf(line, "LUT_3D_SIZE %d", &n) == 1) { dim = n; break; }
        // a bare data line before the size keyword means a malformed header.
        float a, b, c;
        if (sscanf(line, "%f %f %f", &a, &b, &c) == 3) {
            LOGW("tonemap: cube data before LUT_3D_SIZE, bailing");
            tonemap_lut_make_identity(out, TONEMAP_LUT_DEFAULT_DIM);
            return 0;
        }
    }

    if (dim < 2 || dim > TONEMAP_LUT_MAX_DIM) {
        LOGW("tonemap: cube missing/bad LUT_3D_SIZE (%d)", dim);
        tonemap_lut_make_identity(out, TONEMAP_LUT_DEFAULT_DIM);
        return 0;
    }

    if (!tonemap_lut_make_identity(out, dim)) return 0;

    // second pass: read dim^3 triples in rgb-major order. the identity layout
    // already matches that order, so we can write linearly.
    size_t want = (size_t)dim * dim * dim;
    size_t got = 0;
    pos = 0;
    while (next_line(text, len, &pos, line, sizeof line)) {
        if (skippable(line)) continue;
        // skip the header keywords (TITLE, DOMAIN_*, LUT_3D_SIZE)
        if (!isdigit((unsigned char)line[0]) && line[0] != '+' &&
            line[0] != '-' && line[0] != '.') {
            continue;
        }
        float r, g, b;
        if (sscanf(line, "%f %f %f", &r, &g, &b) != 3) continue;
        if (got >= want) break;   // junk past the table, ignore

        size_t i = got * 3u;
        out->data[i + 0] = r;
        out->data[i + 1] = g;
        out->data[i + 2] = b;
        got++;
    }

    if (got != want) {
        LOGW("tonemap: cube truncated, got %zu of %zu entries", got, want);
        // partial is better than nothing; the tail stays identity.
    }
    out->dirty = 1;
    LOGI("tonemap: loaded %dx%dx%d cube (%zu entries)", dim, dim, dim, got);
    return got == want;
}

int tonemap_cube_load_file(tonemap_lut *out, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        LOGW("tonemap: cant open cube file '%s'", path);
        tonemap_lut_make_identity(out, TONEMAP_LUT_DEFAULT_DIM);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        LOGW("tonemap: empty cube file '%s'", path);
        tonemap_lut_make_identity(out, TONEMAP_LUT_DEFAULT_DIM);
        return 0;
    }

    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return 0; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = '\0';
    fclose(f);

    int ok = tonemap_cube_parse(out, buf, rd);
    free(buf);
    return ok;
}
