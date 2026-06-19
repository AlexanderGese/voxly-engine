#include "loader_priority.h"

#include "../../config.h"

loader_priority_cfg loader_priority_default(void) {
    loader_priority_cfg c;
    c.ring_weight    = 100;   // one chunk further out costs ~one stage's worth *100
    c.facing_bonus   = 250;   // ahead of the player: jump the line a bit
    c.facing_penalty = 150;   // behind: wait your turn
    c.stage_weight   = 20;    // early stages win ties at equal distance
    // facing cone ~ 60deg half-angle: cos(60)=0.5 -> dot >= 0.5*len. squared form
    // below avoids the sqrt. 1/4 because we compare dot^2 against (0.5)^2*lensq.
    c.dot_num = 1;
    c.dot_den = 4;
    return c;
}

int loader_priority_dist2(loader_focus focus, int cx, int cz) {
    int dx = cx - focus.cx;
    int dz = cz - focus.cz;
    return dx * dx + dz * dz;
}

// integer "is this chunk within the forward cone of the heading". we work with
// the heading scaled to ints to dodge floats; precision doesnt matter, the cone
// is fuzzy by design. returns >0 ahead, <0 behind, 0 to the side.
static int facing_sign(loader_focus focus, int dx, int dz,
                       int dot_num, int dot_den) {
    // heading comes in as floats; scale to a fixed-point int vector once. a chunk
    // sitting on the player (dx==dz==0) counts as "ahead" so the centre loads first.
    if (dx == 0 && dz == 0) return 1;

    int hx = (int)(focus.fx * 1024.0f);
    int hz = (int)(focus.fz * 1024.0f);
    long dot   = (long)hx * dx + (long)hz * dz;
    if (dot <= 0) return -1;   // pointing away, behind us

    long lensq = (long)(dx * dx + dz * dz) * ((long)hx * hx + (long)hz * hz);
    // ahead-cone test: dot^2 / lensq >= (num/den)^2 -> dot^2*den^2 >= num^2*lensq
    long lhs = dot * dot * (long)dot_den * dot_den;
    long rhs = lensq * (long)dot_num * dot_num;
    if (lhs >= rhs) return 1;   // inside the cone
    return 0;                   // off to one side
}

int loader_priority_score(const loader_priority_cfg *cfg, loader_focus focus,
                          int cx, int cz, loader_job_kind kind) {
    int dx = cx - focus.cx;
    int dz = cz - focus.cz;
    int d2 = dx * dx + dz * dz;

    // chebyshev-ish radius: heap order tracks the L2 distance via d2 directly, but
    // we scale it down so a single facing bonus can still leapfrog ~one ring.
    int score = d2 * cfg->ring_weight;

    // facing bias. only meaningful when the heading is actually set.
    if (focus.valid && (focus.fx != 0.0f || focus.fz != 0.0f)) {
        int s = facing_sign(focus, dx, dz, cfg->dot_num, cfg->dot_den);
        if (s > 0)      score -= cfg->facing_bonus;
        else if (s < 0) score += cfg->facing_penalty;
    }

    // stage bias: earlier jobs are cheaper and unblock neighbours, run them first.
    score += (int)kind * cfg->stage_weight;

    // never let the facing bonus drag a far chunk ahead of a near one we havent
    // even alloc'd. clamp so adjacent chunks (d2<=1) always stay urgent.
    if (d2 <= 1 && score < 0) score = 0;
    return score;
}
