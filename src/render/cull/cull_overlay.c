#include "cull_overlay.h"

#include <stdio.h>

void cull_overlay_summary(const cull_stats *s, strbuf *out) {
    strbuf_appendf(out, "cull %d/%d  fr %d oc %d d %d",
                   s->drawn, s->total,
                   s->frustum_culled, s->occlusion_culled, s->distance_culled);
}

void cull_overlay_detail(const cull_stats *s, strbuf *out) {
    float ratio = cull_overlay_draw_ratio(s);

    strbuf_appendf(out, "chunks total     %d\n", s->total);
    strbuf_appendf(out, "  drawn          %d (%.0f%%)\n", s->drawn, ratio);
    strbuf_appendf(out, "  frustum culled %d\n", s->frustum_culled);
    strbuf_appendf(out, "  occ culled     %d\n", s->occlusion_culled);
    strbuf_appendf(out, "  dist culled    %d\n", s->distance_culled);
    strbuf_appendf(out, "  empty          %d\n", s->empty);

    // tris in thousands, the raw number is too noisy to read live.
    strbuf_appendf(out, "  tris (est)     %dk\n", s->tris_estimate / 1000);
}

float cull_overlay_draw_ratio(const cull_stats *s) {
    if (s->total <= 0) return 0.0f;
    return 100.0f * (float)s->drawn / (float)s->total;
}

// draw a fixed-width bar like "[####....----~~]" where # is drawn,
// . frustum, - occluded, ~ distance. proportions of the total.
void cull_overlay_bar(const cull_stats *s, char *buf, int buf_len) {
    if (buf_len < 4) { if (buf_len > 0) buf[0] = '\0'; return; }

    int inner = buf_len - 3;   // room for '[', ']', '\0'
    if (inner < 1) inner = 1;

    int total = s->total > 0 ? s->total : 1;

    int nd = (s->drawn          * inner) / total;
    int nf = (s->frustum_culled * inner) / total;
    int no = (s->occlusion_culled * inner) / total;
    int nx = (s->distance_culled * inner) / total;

    // rounding can leave a remainder; dump it into the empty/pad bucket.
    int used = nd + nf + no + nx;
    int pad  = inner - used;
    if (pad < 0) pad = 0;

    int p = 0;
    buf[p++] = '[';
    for (int i = 0; i < nd && p < buf_len - 2; i++) buf[p++] = '#';
    for (int i = 0; i < nf && p < buf_len - 2; i++) buf[p++] = '.';
    for (int i = 0; i < no && p < buf_len - 2; i++) buf[p++] = '-';
    for (int i = 0; i < nx && p < buf_len - 2; i++) buf[p++] = '~';
    for (int i = 0; i < pad && p < buf_len - 2; i++) buf[p++] = ' ';
    buf[p++] = ']';
    buf[p]   = '\0';
}
