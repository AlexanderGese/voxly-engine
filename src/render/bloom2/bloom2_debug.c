#include "bloom2_debug.h"
#include "../../util/log.h"

#include <stdio.h>

size_t bloom2_debug_chain_bytes(const bloom2 *b) {
    size_t total = 0;
    for (int i = 0; i < b->chain.count; i++) {
        const bloom2_target *t = &b->chain.mip[i];
        // 8 bytes per texel for rgba16f. no mipmaps on these so thats it.
        total += (size_t)t->w * (size_t)t->h * 8u;
    }
    return total;
}

int bloom2_debug_summary(const bloom2 *b, char *buf, size_t cap) {
    if (cap == 0) return 0;

    size_t bytes = bloom2_debug_chain_bytes(b);
    double kib   = (double)bytes / 1024.0;

    int n = snprintf(buf, cap,
        "bloom2 %s | mips %d | %.1f KiB | thr %.2f knee %.2f int %.3f rad %.2f",
        b->ready && b->params.enabled ? "on" : "off",
        b->chain.count, kib,
        b->params.threshold, b->params.knee,
        b->params.intensity, b->params.radius);

    return n;
}

void bloom2_debug_log(const bloom2 *b) {
    char line[160];
    bloom2_debug_summary(b, line, sizeof line);
    LOGI("%s", line);

    for (int i = 0; i < b->chain.count; i++) {
        const bloom2_target *t = &b->chain.mip[i];
        LOGD("bloom2:   mip[%d] = %dx%d (tex %u)", i, t->w, t->h, t->tex);
    }
}
