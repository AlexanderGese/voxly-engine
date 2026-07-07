#version 330 core
// fxaa main pass. reads the prepass target (rgb scene + luma in .a), detects a
// luma edge in the plus-neighbourhood, picks an orientation, walks the edge to
// find its span, and blends across it. plus a sub-pixel term that softens thin
// one-texel features toward their local average. lo-fi but cheap and effective.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_src;
uniform vec3  u_rcp_frame;     // (1/w, 1/h, unused)
uniform float u_subpix;        // sub-pixel blend strength
uniform float u_subpix_quad;   // subpix^2 * 0.5, unused tail knob
uniform float u_edge_rcp;      // 1 / edge_threshold (unused, kept for symmetry)
uniform float u_edge_min;      // absolute contrast floor
uniform float u_edge_threshold;// relative contrast trigger
uniform int   u_search_steps;  // search iterations per side
uniform int   u_show_edges;    // debug: output the edge mask instead

float luma_at(vec2 uv) { return texture(u_src, uv).a; }

void main() {
    vec2 rcp = u_rcp_frame.xy;

    // plus-neighbourhood lumas.
    float lc = luma_at(v_uv);
    float ln = luma_at(v_uv + vec2(0.0, -rcp.y));
    float ls = luma_at(v_uv + vec2(0.0,  rcp.y));
    float lw = luma_at(v_uv + vec2(-rcp.x, 0.0));
    float le = luma_at(v_uv + vec2( rcp.x, 0.0));

    float mn = min(lc, min(min(ln, ls), min(lw, le)));
    float mx = max(lc, max(max(ln, ls), max(lw, le)));
    float contrast = mx - mn;

    // trigger: relative to the local max, with an absolute floor for darks.
    if (contrast < max(u_edge_min, mx * u_edge_threshold)) {
        frag = vec4(texture(u_src, v_uv).rgb, 1.0);
        return;
    }

    // diagonals for orientation + sub-pixel average.
    float lnw = luma_at(v_uv + vec2(-rcp.x, -rcp.y));
    float lne = luma_at(v_uv + vec2( rcp.x, -rcp.y));
    float lsw = luma_at(v_uv + vec2(-rcp.x,  rcp.y));
    float lse = luma_at(v_uv + vec2( rcp.x,  rcp.y));

    float grad_x = abs((lnw + lne) - 2.0 * ln)
                 + abs((lw  + le ) - 2.0 * lc) * 2.0
                 + abs((lsw + lse) - 2.0 * ls);
    float grad_y = abs((lnw + lsw) - 2.0 * lw)
                 + abs((ln  + ls ) - 2.0 * lc) * 2.0
                 + abs((lne + lse) - 2.0 * le);
    // dominant gradient along y -> the edge line runs horizontally.
    bool horizontal = grad_y >= grad_x;

    // step perpendicular to the edge for the blend, and along it for the search.
    vec2 step_perp = horizontal ? vec2(0.0, rcp.y) : vec2(rcp.x, 0.0);
    vec2 step_along = horizontal ? vec2(rcp.x, 0.0) : vec2(0.0, rcp.y);

    // gradient sign across the edge picks which side to lean toward.
    float lpos = horizontal ? ls : le;
    float lneg = horizontal ? ln : lw;
    float grad = lpos - lneg;
    float edge_avg = 0.5 * (lpos + lneg);

    // walk both directions until the local luma stops matching the edge.
    vec2 uvp = v_uv;
    vec2 uvn = v_uv;
    float end_pos = 0.0, end_neg = 0.0;
    bool done_pos = false, done_neg = false;

    for (int i = 0; i < u_search_steps; i++) {
        if (!done_pos) {
            uvp += step_along;
            float l = luma_at(uvp + 0.5 * step_perp);
            if (abs(l - edge_avg) < contrast * 0.25) done_pos = true;
            else end_pos += 1.0;
        }
        if (!done_neg) {
            uvn -= step_along;
            float l = luma_at(uvn + 0.5 * step_perp);
            if (abs(l - edge_avg) < contrast * 0.25) done_neg = true;
            else end_neg += 1.0;
        }
        if (done_pos && done_neg) break;
    }

    float span = end_pos + end_neg + 1.0;
    float nearest = min(end_pos, end_neg);
    float edge_blend = clamp(0.5 - nearest / span, 0.0, 0.5);

    // sub-pixel term: how far the center sits from the 3x3 average, shaped and
    // scaled by the subpix strength. blurs thin features the edge walk misses.
    float avg = (2.0 * (ln + ls + lw + le) + (lnw + lne + lsw + lse)) / 12.0;
    float subpix1 = clamp(abs(avg - lc) / max(contrast, 1e-5), 0.0, 1.0);
    float subpix2 = subpix1 * subpix1 * (3.0 - 2.0 * subpix1);
    float subpix_blend = subpix2 * u_subpix;

    float blend = max(edge_blend, subpix_blend);

    // pull toward the brighter/darker side along the perpendicular.
    float dir = (lc < edge_avg) ? 1.0 : -1.0;
    if (end_pos < end_neg) dir = -dir;
    vec2 final_uv = v_uv + step_perp * dir * blend;

    vec3 result = texture(u_src, final_uv).rgb;

    if (u_show_edges != 0) {
        // visualise: red channel = edge blend, green = subpix. quick sanity.
        frag = vec4(edge_blend * 2.0, subpix_blend, 0.0, 1.0);
        return;
    }
    frag = vec4(result, 1.0);
}
