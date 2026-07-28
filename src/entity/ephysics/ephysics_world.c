#include "ephysics_world.h"
#include "ephysics_integrate.h"
#include "ephysics_material.h"
#include "ephysics_impulse.h"
#include <math.h>
#include <stddef.h>
int ephysics_world_tick_one(world *w, entity *e, const ephys_input *in, float dt,
                            ephys_event *out) {
    if (!e || !e->alive) return 0;

    ephys_body b = ephysics_body_from_entity(e);

    // snapshot the state we need to diff against after the step. impact speed is
    // recovered from the velocity we shed, since the integrator zeroes blocked
    // components internally and doesnt hand back a manifold.
    uint32_t pre_flags = b.flags;
    float    pre_vy    = b.vel.y;
    vec3     pre_vel   = b.vel;

    ephysics_step_body(w, &b, in, dt);
    ephysics_body_to_entity(&b, e);

    // figure out what, if anything, is worth reporting.
    int grounded_now = (b.flags & EPHYS_F_GROUNDED) != 0;
    int was_grounded = (pre_flags & EPHYS_F_GROUNDED) != 0;
    int landed = grounded_now && !was_grounded;

    int water_now = (b.flags & EPHYS_F_IN_WATER) != 0;
    int was_water = (pre_flags & EPHYS_F_IN_WATER) != 0;
    int entered_water = water_now && !was_water;

    // impact = how much speed we lost this tick that the integrator killed on a
    // contact. gravity/friction also change velocity, so this overestimates a
    // bit, but the dominant term on a real hit is the zeroed axis, which is what
    // we want for sfx thresholds.
    float lost = vec3_length(vec3_sub(pre_vel, b.vel));
    float land_speed = landed ? fabsf(pre_vy) : 0.0f;

    int notable = landed || entered_water ||
                  (b.flags & (EPHYS_F_WALL_X | EPHYS_F_WALL_Z | EPHYS_F_CEILING));

    if (!notable || !out) return 0;

    out->entity_id     = e->id;
    out->land_speed    = land_speed;
    out->impact_speed  = lost;
    out->flags         = b.flags;
    out->entered_water = entered_water;
    return 1;
}

int ephysics_world_tick(world *w, entity *ents, int n,
                        const ephys_input *inputs, float dt,
                        ephys_event *out_events, int out_cap,
                        ephys_world_stats *stats) {
    int nevents = 0;
ephys_world_stats st = {0, 0, 0, 0, 0.0f}
;
for (int i = 0;
i < n;
i++) {
        entity *e = &ents[i];
        if (!e->alive) continue;

        const ephys_input *in = inputs ? &inputs[i] : NULL;

        ephys_event ev;
        int got = ephysics_world_tick_one(w, e, in, dt,
                                          (nevents < out_cap) ? &ev : NULL);

        st.bodies++;

        // re-derive a couple of cheap stats from the entity that got written
        // back. on_ground is the only flag the entity carries, so fluid is read
        // from the event when we have one.
        if (e->on_ground) st.grounded++;
        float sp = vec3_length(e->vel);
        if (sp > st.max_speed_seen) st.max_speed_seen = sp;

        if (got) {
            if (ev.entered_water || (ev.flags & EPHYS_F_IN_WATER))
                st.in_fluid++;
            if (nevents < out_cap) {
                out_events[nevents++] = ev;
                st.events++;
            }
        }
    }

    if (stats) *stats = st;
return nevents;
}
