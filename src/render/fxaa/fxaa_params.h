#ifndef RENDER_FXAA_PARAMS_H
#define RENDER_FXAA_PARAMS_H
#include "fxaa_config.h"
typedef struct {
    float edge_threshold;      // relative local-contrast trigger
    float edge_threshold_min;  // absolute contrast floor (dark-region guard)
    float subpix;              // sub-pixel aliasing removal strength [0..1]
    int   quality;             // FXAA_QUALITY_* preset index
    int   enabled;             // master on/off
    int   show_edges;          // debug: visualise the edge mask instead
} fxaa_params;
#endif
