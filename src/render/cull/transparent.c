#include "transparent.h"

int cull_transparent_build(const cull_item *src, int src_count,
                           cull_item *out, int cap) {
    int n = 0;
    // walk src backwards: src is near->far, so reversed gives far->near.
    for (int i = src_count - 1; i >= 0 && n < cap; i--) {
        // skip anything that wont contribute. empty chunks shouldnt be in
        // the survivor list at all, but be defensive.
        if (!src[i].c || src[i].c->vertex_count <= 0) continue;
        out[n++] = src[i];
    }
    return n;
}

void cull_transparent_reverse(cull_item *items, int count) {
    int i = 0, j = count - 1;
    while (i < j) {
        cull_item t = items[i];
        items[i] = items[j];
        items[j] = t;
        i++;
        j--;
    }
}

int cull_transparent_estimate(const cull_item *items, int count) {
    int n = 0;
    for (int i = 0; i < count; i++) {
        if (items[i].c && items[i].c->vertex_count > 0) n++;
    }
    return n;
}
