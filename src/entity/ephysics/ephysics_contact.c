#include "ephysics_contact.h"
#include <math.h>

void ephysics_manifold_reset(ephys_manifold *m) {
    m->count = 0;
    m->max_impact = 0.0f;
    m->dropped = 0;
}

void ephysics_manifold_add(ephys_manifold *m, const ephys_hit *h, vec3 pre_vel) {
    if (!h->hit) return;

    // closing speed = component of the incoming velocity along the contact
    // normal. normal points back at us so a head-on hit gives a negative dot;
    // flip the sign so impact is a positive magnitude.
    float closing = -vec3_dot(pre_vel, h->normal);
    if (closing < 0.0f) closing = 0.0f;   // glancing/separating, ignore

    if (closing > m->max_impact) m->max_impact = closing;

    if (m->count >= EPHYS_MAX_CONTACTS) {
        m->dropped++;
        return;
    }

    ephys_contact *c = &m->list[m->count++];
    c->axis   = h->axis;
    c->normal = h->normal;
    c->impact = closing;
    c->t      = h->t;
}

int ephysics_manifold_grounded(const ephys_manifold *m) {
    for (int i = 0; i < m->count; i++)
        if (m->list[i].normal.y > 0.5f) return 1;
    return 0;
}

float ephysics_manifold_land_speed(const ephys_manifold *m) {
    float worst = 0.0f;
    for (int i = 0; i < m->count; i++) {
        const ephys_contact *c = &m->list[i];
        if (c->normal.y > 0.5f && c->impact > worst) worst = c->impact;
    }
    return worst;
}
