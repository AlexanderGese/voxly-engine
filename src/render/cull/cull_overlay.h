#ifndef RENDER_CULL_OVERLAY_H
#define RENDER_CULL_OVERLAY_H

#include "cull_types.h"
#include "../../util/strbuf.h"

// formats cull stats into text lines for the F3-ish debug overlay. doesnt
// draw anything itself — hands back strings, the text renderer deals with
// pixels. keeping it here so the overlay code doesnt have to know the
// cull_stats layout.

// one-line summary, e.g. "cull 142/389  fr 180 oc 51 d 16".
void cull_overlay_summary(const cull_stats *s, strbuf *out);

// multi-line breakdown for the expanded view. appends to out.
void cull_overlay_detail(const cull_stats *s, strbuf *out);

// percentage of total chunks that survived (0..100). guards div-by-zero.
float cull_overlay_draw_ratio(const cull_stats *s);

// a crude one-char-per-bucket histogram of where chunks went. useful for
// eyeballing whether occlusion is pulling its weight. writes into buf.
void cull_overlay_bar(const cull_stats *s, char *buf, int buf_len);

#endif
