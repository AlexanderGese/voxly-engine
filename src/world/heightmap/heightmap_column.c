#include "heightmap_column.h"
#include "heightmap_curves.h"
#include "heightmap_spline.h"
#include "../block.h"
#include <math.h>
static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void heightmap_column_resolve(const heightmap_params *p, int wx, int wz,
                              heightmap_column *out) {
    out->wx = wx;
out->wz = wz;
// 1. sample the raw noise fields
heightmap_sample_fields(p, wx, wz, &out->fields);
const heightmap_spline *cs = heightmap_curve_continental();
const heightmap_spline *es = heightmap_curve_erosion();
const heightmap_spline *ps = heightmap_curve_peaks_valleys();
// 2. push each field through its spline
float cont   = heightmap_spline_eval(cs, out->fields.continentalness);
float eros   = heightmap_spline_eval(es, out->fields.erosion);
float relief = heightmap_spline_eval(ps, out->fields.peaks_valleys);
out->continental    = cont;
out->relief         = relief;
// erosion_bias lets the params dial the whole erosion effect up or down
// without re-authoring the curve. at bias 1 we use the spline straight, at
// 0 erosion does nothing and everything stays jagged.
float ef = 1.0f - p->erosion_bias * (1.0f - eros);
out->erosion_factor = ef;
// 3. assemble the height. continent sets the broad elevation, then the
// peaks/valleys relief gets layered on but only as much as erosion allows.
float h = (float)p->base_height
            + cont   * p->cont_amp
            + relief * p->pv_amp * ef;
int hi = (int)lroundf(h);
hi = clampi(hi, p->min_height, p->max_height);
out->surface_y  = hi;
out->underwater = (hi < p->sea_level) ? 1 : 0;
// 4. steepness estimate. combine the slopes the splines report at this
// point, weighted by their amplitudes. surface rules use it to bare rock
// out cliff faces. its an estimate, not a real gradient, but it tracks.
float dc = heightmap_spline_deriv(cs, out->fields.continentalness);
float dp = heightmap_spline_deriv(ps, out->fields.peaks_valleys);
float steep = fabsf(dc) * (p->cont_amp / 64.0f)
                + fabsf(dp) * (p->pv_amp  / 64.0f) * ef;
out->steepness = steep;
}

int heightmap_column_height(const heightmap_params *p, int wx, int wz) {
    heightmap_column col;
    heightmap_column_resolve(p, wx, wz, &col);
    return col.surface_y;
}

int heightmap_column_surface_block(const heightmap_params *p,
                                   const heightmap_column *col) {
    int y   = col->surface_y;
int sea = p->sea_level;
// anything at or just under the waterline is sand (beach / lakebed)
if (y <= sea + 1 && y >= sea - 3) return BLOCK_SAND;
// deep underwater is also sand, gravel would be nicer but we dont have it
if (y < sea - 3) return BLOCK_SAND;
if (col->steepness > 1.6f) return BLOCK_STONE;
int snow_line = sea + 70;
if (y >= snow_line) return BLOCK_SNOW;
return BLOCK_GRASS;
}
