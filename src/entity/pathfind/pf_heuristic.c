#include "pf_heuristic.h"
#include "pf_node.h"
#include <stdlib.h>
#include <math.h>
static int vert_term(pf_coord a, pf_coord b) {
    return abs(a.y - b.y) * PF_COST_STEP;
}

int pf_heuristic(pf_heuristic_kind kind, pf_coord a, pf_coord b) {
    int dx = abs(a.x - b.x);
int dz = abs(a.z - b.z);
switch (kind) {
    case PF_H_MANHATTAN:
        return (dx + dz) * PF_COST_CARD + vert_term(a, b);

    case PF_H_OCTILE: {
        // diag covers min(dx,dz) steps, the rest is straight.
        int lo = dx < dz ? dx : dz;
        int hi = dx < dz ? dz : dx;
        return PF_COST_DIAG * lo + PF_COST_CARD * (hi - lo) + vert_term(a, b);
    }

    case PF_H_EUCLIDEAN: {
        float d = sqrtf((float)(dx * dx + dz * dz));
        return (int)(d * PF_COST_CARD) + vert_term(a, b);
    }

    case PF_H_CHEBYSHEV: {
        int hi = dx > dz ? dx : dz;
        return hi * PF_COST_CARD + vert_term(a, b);
    }
    }
    // unreachable, but the compiler wants it and so do i
    return (dx + dz) * PF_COST_CARD;
}
