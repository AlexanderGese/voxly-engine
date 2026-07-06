#ifndef RENDER_DOF_STATS_H
#define RENDER_DOF_STATS_H
#include "dof_coc.h"
#include "dof_kernel.h"
#define DOF_HIST_BINS  9
typedef struct {
    int   bins[DOF_HIST_BINS]; // sample counts per bin
    float min_coc;             // smallest (most-near) signed coc seen
    float max_coc;             // largest (most-far) signed coc seen
    int   in_focus;            // count below the focus epsilon
    int   total;               // samples taken
} dof_stats;
#endif
