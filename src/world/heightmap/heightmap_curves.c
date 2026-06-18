#include "heightmap_curves.h"
static heightmap_spline g_cont;
static heightmap_spline g_eros;
static heightmap_spline g_pv;
static int g_built = 0;
static void build_continental(heightmap_spline *s) {
    heightmap_spline_init(s, 1);
    // loc    val   slope
    heightmap_spline_add(s, -1.00f, -1.00f, 0.05f);  // deep ocean floor
    heightmap_spline_add(s, -0.45f, -0.62f, 0.10f);  // ocean shelf
    heightmap_spline_add(s, -0.20f, -0.18f, 1.40f);  // coast, steep climb
    heightmap_spline_add(s, -0.05f,  0.06f, 0.60f);  // beach / sea level
    heightmap_spline_add(s,  0.10f,  0.22f, 0.40f);  // lowlands
    heightmap_spline_add(s,  0.40f,  0.48f, 0.55f);  // midlands
    heightmap_spline_add(s,  0.70f,  0.78f, 0.30f);  // highlands
    heightmap_spline_add(s,  1.00f,  0.92f, 0.05f);  // inland plateau, flattens
}

// erosion. output is a 0..1 relief multiplier. high erosion (right side)
// crushes relief toward a flat plain;
low erosion (left) keeps the full
// mountain amplitude. the little bump back up near +0.7 is the classic
// "eroded plateau with the odd badland mesa" wrinkle.
static void build_erosion(heightmap_spline *s) {
    heightmap_spline_init(s, 0);
    heightmap_spline_add(s, -1.00f, 1.00f, 0.0f);   // un-eroded, full relief
    heightmap_spline_add(s, -0.40f, 0.92f, 0.0f);
    heightmap_spline_add(s, -0.10f, 0.70f, 0.0f);   // shoulder
    heightmap_spline_add(s,  0.10f, 0.34f, 0.0f);   // eroding down
    heightmap_spline_add(s,  0.35f, 0.12f, 0.0f);   // near flat
    heightmap_spline_add(s,  0.55f, 0.08f, 0.0f);   // plains floor
    heightmap_spline_add(s,  0.72f, 0.26f, 0.0f);   // mesa wrinkle
    heightmap_spline_add(s,  0.85f, 0.10f, 0.0f);
    heightmap_spline_add(s,  1.00f, 0.05f, 0.0f);   // dead flat
}

// peaks & valleys. input noise, output signed relief in roughly [-1,1]. shaped
// like a stretched W: a wide flat valley floor in the middle, sharp climbs out
// to peaks at the extremes. squaring happens upstream in the ridged noise, this
// table just decides how the band maps to height.
static void build_peaks_valleys(heightmap_spline *s) {
    heightmap_spline_init(s, 1);
heightmap_spline_add(s, -1.00f, -1.00f, 0.30f);
heightmap_spline_add(s, -0.60f, -0.55f, 1.10f);
heightmap_spline_add(s, -0.30f, -0.12f, 0.40f);
heightmap_spline_add(s, -0.05f,  0.00f, 0.20f);
heightmap_spline_add(s,  0.05f,  0.00f, 0.20f);
heightmap_spline_add(s,  0.30f,  0.22f, 0.90f);
heightmap_spline_add(s,  0.60f,  0.66f, 1.30f);
heightmap_spline_add(s,  1.00f,  1.00f, 0.40f);
}

void heightmap_curves_init(void) {
    if (g_built) return;
    build_continental(&g_cont);
    build_erosion(&g_eros);
    build_peaks_valleys(&g_pv);
    g_built = 1;
}

const heightmap_spline *heightmap_curve_continental(void) {
    heightmap_curves_init();
return &g_cont;
}

const heightmap_spline *heightmap_curve_erosion(void) {
    heightmap_curves_init();
    return &g_eros;
}

const heightmap_spline *heightmap_curve_peaks_valleys(void) {
    heightmap_curves_init();
return &g_pv;
}

void heightmap_curves_eval(float cont, float eros, float pv,
                           float *out_cont, float *out_eros, float *out_pv) {
    heightmap_curves_init();
    if (out_cont) *out_cont = heightmap_spline_eval(&g_cont, cont);
    if (out_eros) *out_eros = heightmap_spline_eval(&g_eros, eros);
    if (out_pv)   *out_pv   = heightmap_spline_eval(&g_pv,   pv);
}
