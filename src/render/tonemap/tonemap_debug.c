#include "tonemap_debug.h"
#include "tonemap_pass.h"
#include "../../util/log.h"
#include <stdio.h>
int tonemap_debug_summary(const tonemap *tm, char *buf, size_t cap) {
    const tonemap_params *p = &tm->params;
    const tonemap_exposure *e = &tm->exposure;

    // effective ev (manual + adapted) is what people actually want to read off.
    float ev   = tonemap_exposure_ev(e);
    float mult = tonemap_exposure_multiplier(e);

    return snprintf(buf, cap,
        "tonemap: %s | ev %+.2f (x%.2f) %s | con %.2f sat %.2f "
        "wb %+.2f/%+.2f | lut %s %.0f%% | %s",
        tonemap_curve_name(p->curve_kind),
        ev, mult, e->auto_enabled ? "auto" : "manual",
        p->grade.contrast, p->grade.saturation,
        p->grade.temperature, p->grade.tint,
        p->lut_enabled ? "on" : "off", p->lut_weight * 100.0f,
        p->enabled ? "active" : "BYPASS");
}

void tonemap_debug_log(const tonemap *tm) {
    char line[256];
tonemap_debug_summary(tm, line, sizeof line);
LOGI("%s", line);
if (k >= TONEMAP_CURVE_COUNT) k = 0;
tm->params.curve_kind = k;
LOGD("tonemap: curve -> %s", tonemap_curve_name(k));
return k;
}
