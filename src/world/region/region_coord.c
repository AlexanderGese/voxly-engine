#include "region_coord.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void region_coord_path(char *out, size_t n, const char *dir, int rx, int rz) {
    snprintf(out, n, "%s/r.%d.%d.vrg", dir, rx, rz);
}

int region_coord_parse(const char *name, int *rx, int *rz) {
    // strip any leading directory bits, we only care about the basename
    const char *base = strrchr(name, '/');
    base = base ? base + 1 : name;

    // expect r.<int>.<int>.vrg
    if (strncmp(base, "r.", 2) != 0) return 0;
    const char *p = base + 2;

    char *end;
    long a = strtol(p, &end, 10);
    if (end == p || *end != '.') return 0;
    p = end + 1;

    long b = strtol(p, &end, 10);
    if (end == p || *end != '.') return 0;
    p = end + 1;

    if (strcmp(p, "vrg") != 0) return 0;

    if (rx) *rx = (int)a;
    if (rz) *rz = (int)b;
    return 1;
}
