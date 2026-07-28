#include "ephysics_integrate.h"
#include "ephysics_material.h"
#include "ephysics_broadphase.h"
#include "ephysics_sweep.h"
#include "ephysics_step.h"
#include "ephysics_friction.h"
#include "ephysics_fluid.h"
#include "../../config.h"

#include <math.h>

ephys_input ephysics_input_none(void) {
    ephys_input in;
    in.wish_vel = VEC3_ZERO;
    in.accel    = 0.0f;
    in.jump     = 0;
    in.swim_up  = 0;
    return in;
}

// blend current horizontal velocity toward wish_vel by accel*dt. simple capped
// approach so direction changes feel weighty but responsive.
static void apply_wish(ephys_body *b, const ephys_input *in, float dt) {
    if (in->accel <= 0.0f) return;
    vec3 cur  = vec3_new(b->vel.x, 0.0f, b->vel.z);
    vec3 want = vec3_new(in->wish_vel.x, 0.0f, in->wish_vel.z);
    vec3 diff = vec3_sub(want, cur);
    float dist = vec3_length(diff);
    if (dist < 1e-5f) return;
    float dv = in->accel * dt;
    if (dv > dist) dv = dist;          // dont overshoot the target velocity
    vec3 add = vec3_scale(vec3_normalize(diff), dv);
    b->vel.x += add.x;
    b->vel.z += add.z;
}

static void apply_gravity(ephys_body *b, float dt) {
    if (b->flags & EPHYS_F_IN_WATER) {
        // reduced gravity in water; buoyancy pass adds the float force back.
        b->vel.y += GRAVITY * 0.35f * dt;
    } else {
        b->vel.y += GRAVITY * dt;
    }
    if (b->vel.y < TERMINAL_VELOCITY) b->vel.y = TERMINAL_VELOCITY;
}

static void handle_jump(ephys_body *b, const ephys_input *in) {
    if (!in) return;
    if (in->jump && (b->flags & EPHYS_F_GROUNDED)) {
        b->vel.y = PLAYER_JUMP_VEL;
        b->flags &= ~(uint32_t)EPHYS_F_GROUNDED;
    } else if (in->swim_up && (b->flags & EPHYS_F_IN_WATER)) {
        // paddle: bleed toward an upward swim speed instead of snapping.
        float target = 3.0f;
        if (b->vel.y < target) b->vel.y += (target - b->vel.y) * 0.4f;
    }
}

void ephysics_step_body(world *w, ephys_body *b, const ephys_input *in, float dt) {
    ephys_input none = ephysics_input_none();
    if (!in) in = &none;

    // 1. medium forces first so jump/grav see the right grounded/water state.
    // we re-gather grounding at the end, but the *previous* tick's flags are
    // a good enough basis for buoyancy and input handling this tick.
    ephysics_fluid_apply(w, b, dt);

    // 2. controller input + impulses
    apply_wish(b, in, dt);
    handle_jump(b, in);

    // 3. forces
    apply_gravity(b, dt);
    ephysics_clamp_speed(b);

    // 4. broadphase against the intended move
    vec3 delta = vec3_scale(b->vel, dt);
    ephys_candidates cand;
    ephysics_gather(w, b, delta, &cand);
    if (cand.overflowed) {
        // way too many candidates -> we're probably inside geometry. clamp the
        // move hard so we dont tunnel through the world this tick.
        delta = vec3_scale(delta, 0.25f);
    }

    // 5. flat resolve. snapshot pos first so step-up can compare.
    vec3 pre_pos = b->pos;
    ephys_body flat = *b;
    ephysics_resolve(&cand, &flat, delta, 4);
    vec3 flat_pos = flat.pos;

    // 6. try step-up using the same candidate set if we hit a wall.
    *b = flat;   // adopt the flat result as the baseline
    if (b->mat.step_height > 0.0f &&
        (b->flags & (EPHYS_F_WALL_X | EPHYS_F_WALL_Z))) {
        // restore pre-move pos for the step attempt; it sweeps from scratch.
        ephys_body stepper = *b;
        stepper.pos = pre_pos;
        if (ephysics_step(&cand, &stepper, delta, flat_pos)) {
            b->pos   = stepper.pos;
            b->vel   = stepper.vel;
            b->flags = stepper.flags;
        }
    }

    // 7. ground recheck: a tiny downward probe so grounded state is fresh for
    // the NEXT tick's friction/jump even if we didnt move into the floor.
    if (!(b->flags & EPHYS_F_GROUNDED) && b->vel.y <= 0.0f) {
        ephys_body probe = *b;
        float dropped = ephysics_settle(&cand, &probe, 0.05f);
        if (dropped < 0.05f && (probe.flags & EPHYS_F_GROUNDED)) {
            b->pos = probe.pos;
            b->flags |= EPHYS_F_GROUNDED;
        }
    }

    // 8. friction last, on the post-collision velocity.
    ephysics_friction_apply(w, b, dt);
}

void ephysics_tick_entity(world *w, entity *e, const ephys_input *in, float dt) {
    if (!e || !e->alive) return;
    ephys_body b = ephysics_body_from_entity(e);
    ephysics_step_body(w, &b, in, dt);
    ephysics_body_to_entity(&b, e);
}
