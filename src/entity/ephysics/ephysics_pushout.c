#include "ephysics_pushout.h"
#include "ephysics_material.h"
#include <math.h>
#define EPHYS_PUSH_NEIGHBOURS 16
float ephysics_pushout_overlap(const entity *a, const entity *b) {
    float ra = entity_width(a->type) * 0.5f;
    float rb = entity_width(b->type) * 0.5f;

    float dx = b->pos.x - a->pos.x;
    float dz = b->pos.z - a->pos.z;
    float dist = sqrtf(dx * dx + dz * dz);

    return (ra + rb) - dist;   // positive == overlapping by this much
}

void ephysics_pushout_pair(entity *a, entity *b, float stiffness) {
    float dx = b->pos.x - a->pos.x;
float dz = b->pos.z - a->pos.z;
float dist = sqrtf(dx * dx + dz * dz);
float overlap = ephysics_pushout_overlap(a, b);
if (overlap <= 0.0f) return;
vec3 dir;
if (dist < 1e-4f) {
        // exactly coincident (spawned on the same spot). pick a deterministic
        // direction from the ids so the two dont pick the same way and re-stack.
        float ang = (float)((a->id ^ b->id) & 7) * 0.785398f;  // 8 dirs
        dir = vec3_new(cosf(ang), 0, sinf(ang));
        dist = 0.0f;
    } else {
        dir = vec3_new(dx / dist, 0, dz / dist);
}

    // mass weighting: lighter body takes the larger share of the push.
    float ma = ephysics_material_for(a->type).mass;
float mb = ephysics_material_for(b->type).mass;
if (ma < 0.01f) ma = 0.01f;
if (mb < 0.01f) mb = 0.01f;
float total = ma + mb;
float push = overlap * stiffness;
float share_a = mb / total;
// a moves proportional to b's mass
float share_b = ma / total;
// apply as a velocity nudge; the next world step's friction settles it.
a->vel.x -= dir.x * push * share_a;
a->vel.z -= dir.z * push * share_a;
b->vel.x += dir.x * push * share_b;
b->vel.z += dir.z * push * share_b;
}

int ephysics_pushout_all(entity *ents, int n, const ephys_grid *g,
                         float stiffness) {
    int pairs = 0;
    int neigh[EPHYS_PUSH_NEIGHBOURS];

    for (int i = 0; i < n; i++) {
        entity *a = &ents[i];
        if (!a->alive) continue;

        float r = entity_width(a->type) + EPHYS_GRID_CELL;
        int m = ephysics_grid_query(g, a->pos, r, neigh, EPHYS_PUSH_NEIGHBOURS);

        for (int k = 0; k < m; k++) {
            int j = neigh[k];
            if (j <= i) continue;          // each pair once, skip self
            entity *b = &ents[j];
            if (!b->alive) continue;

            if (ephysics_pushout_overlap(a, b) > 0.0f) {
                ephysics_pushout_pair(a, b, stiffness);
                pairs++;
            }
        }
    }
    return pairs;
}
