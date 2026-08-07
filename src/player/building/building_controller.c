#include "building_controller.h"
#include "building_reach.h"
#include "building_place.h"
#include "building_break.h"
#include "building_validate.h"

#include "../../config.h"

// held-place repeat rate. minecraft-ish: a quick tap places one, holding
// places ~4/sec. tap (rising edge) always fires immediately.
#define BUILDING_PLACE_INTERVAL 0.22f

void building_controller_init(building_controller *bc) {
    building_history_init(&bc->hist);
    building_preview_init(&bc->preview);
    break_progress_init(&bc->breaking);
    bc->target = (building_target){0};
    bc->place_cooldown = 0.0f;
    bc->prev_placing = 0;
    bc->reach = PLAYER_REACH;
    bc->enabled = 1;
}

// try a single placement at the current target. updates the event + history.
static void do_place(building_controller *bc, world *w,
                     const building_input *in, building_event *out) {
    if (in->hand_id == BLOCK_AIR) {
        if (out) out->place_fail = BPLACE_EMPTY_HAND;
        return;
    }

    building_edit e;
    int r = building_place_block(w, &bc->hist, in->hand_id, &bc->target,
                                 in->yaw, in->feet, &e);
    if (r == BPLACE_OK) {
        if (out) {
            out->placed = 1;
            out->placed_id = e.after;
            out->placed_x = e.wx;
            out->placed_y = e.wy;
            out->placed_z = e.wz;
        }
    } else if (out) {
        out->place_fail = r;
    }
}

// commit the break once the progress bar has filled. caller guarantees the
// target is valid and the timer is full.
static void do_break(building_controller *bc, world *w, building_event *out) {
    building_drop drop;
    building_edit e;
    int r = building_break_block(w, &bc->hist, &bc->target, &drop, &e);
    if (r == BBREAK_OK && out) {
        out->broke = 1;
        out->broke_id = e.before;
        out->broke_drop = drop.drop_id;
        out->broke_count = drop.drop_count;
        out->broke_x = e.wx;
        out->broke_y = e.wy;
        out->broke_z = e.wz;
    }
    break_progress_cancel(&bc->breaking);
}

building_target building_controller_update(building_controller *bc, world *w,
                                           const building_input *in,
                                           float dt, building_event *out) {
    if (out) {
        building_event z = {0};
        *out = z;
    }
    if (!bc->enabled) return (building_target){0};

    // resolve where we're pointing, surface-clamped to arm length.
    vec3 eye = building_eye_from_feet(in->feet);
    vec3 fwd = building_forward_from_angles(in->yaw, in->pitch);
    bc->target = building_reach_resolve(w, eye, fwd, bc->reach);

    // --- breaking ----------------------------------------------------------
    if (in->mining) {
        // drive the real progress with the real dt here (the helper above used
        // a placeholder; this is the authoritative advance).
        if (bc->target.valid) {
            int hx = bc->target.hit_x, hy = bc->target.hit_y, hz = bc->target.hit_z;
            block_id id = world_get_block(w, hx, hy, hz);
            if (building_is_breakable(id)) {
                if (!bc->breaking.active ||
                    bc->breaking.x != hx || bc->breaking.y != hy ||
                    bc->breaking.z != hz) {
                    float secs = building_break_seconds(id, in->tool_tier);
                    break_progress_begin(&bc->breaking, hx, hy, hz, secs);
                }
                break_progress_tick(&bc->breaking, hx, hy, hz, 1, dt);
                if (bc->breaking.progress >= 1.0f) {
                    do_break(bc, w, out);
                }
            } else {
                break_progress_cancel(&bc->breaking);
            }
        } else {
            break_progress_cancel(&bc->breaking);
        }
    } else {
        break_progress_cancel(&bc->breaking);
    }

    // --- placing -----------------------------------------------------------
    if (bc->place_cooldown > 0.0f) bc->place_cooldown -= dt;

    int rising = in->placing && !bc->prev_placing;
    if (in->placing) {
        if (rising) {
            do_place(bc, w, in, out);          // tap: instant
            bc->place_cooldown = BUILDING_PLACE_INTERVAL;
        } else if (bc->place_cooldown <= 0.0f) {
            do_place(bc, w, in, out);          // held: repeat on cooldown
            bc->place_cooldown = BUILDING_PLACE_INTERVAL;
        }
    }
    bc->prev_placing = in->placing;

    // --- preview -----------------------------------------------------------
    // only validate for the ghost tint when the hand actually holds a block.
    int verdict = BPLACE_OK;
    if (in->hand_id != BLOCK_AIR && bc->target.valid) {
        verdict = building_validate_place(w, in->hand_id,
                                          bc->target.place_x,
                                          bc->target.place_y,
                                          bc->target.place_z,
                                          bc->target.face, in->feet);
    }
    building_preview_update(&bc->preview, &bc->target, in->hand_id, verdict, 3);

    return bc->target;
}

int building_controller_undo(building_controller *bc, world *w) {
    return building_history_undo(&bc->hist, w);
}

int building_controller_redo(building_controller *bc, world *w) {
    return building_history_redo(&bc->hist, w);
}
