#include "anim_controller.h"
#include "anim_pose.h"

#include <string.h>
#include "../../math/easing.h"   // clampf, smoothstepf

static void copy_name(char *dst, const char *src) {
    size_t n = 0;
    while (src && src[n] && n < ANIM_MAX_NAME - 1) { dst[n] = src[n]; n++; }
    dst[n] = '\0';
}

void animation_controller_init(animation_controller *c,
                               const animation_skeleton *sk) {
    c->skeleton = sk;
    c->state_count = 0;
    c->transition_count = 0;
    c->current = -1;
    c->previous = -1;
    c->fade_t = 0.0f;
    c->fade_dur = 0.0f;
    c->fading = 0;
    animation_skeleton_rest_pose(sk, &c->rest);
    animation_pose_copy(&c->prev_pose, &c->rest);
    animation_pose_copy(&c->out_pose, &c->rest);
}

int animation_controller_add_state(animation_controller *c, const char *name,
                                   const animation_clip *clip, int looping) {
    if (c->state_count >= ANIM_CTRL_MAX_STATES) return -1;
    int idx = c->state_count++;
    animation_state_def *s = &c->states[idx];
    copy_name(s->name, name);
    s->looping = looping;
    animation_sampler_init(&s->sampler, clip, looping);
    return idx;
}

int animation_controller_find_state(const animation_controller *c,
                                    const char *name) {
    for (int i = 0; i < c->state_count; i++)
        if (strcmp(c->states[i].name, name) == 0) return i;
    return -1;
}

void animation_controller_add_transition(animation_controller *c, int from,
                                         int to, int trigger, float duration) {
    if (c->transition_count >= ANIM_CTRL_MAX_TRANSITIONS) return;
    animation_transition *t = &c->transitions[c->transition_count++];
    t->from = from;
    t->to = to;
    t->trigger = trigger;
    t->duration = duration;
}

void animation_controller_set_state(animation_controller *c, int state) {
    if (state < 0 || state >= c->state_count) return;
    c->current = state;
    c->previous = -1;
    c->fading = 0;
    c->fade_t = 0.0f;
    animation_sampler_rewind(&c->states[state].sampler);
}

void animation_controller_play(animation_controller *c, int state, float duration) {
    if (state < 0 || state >= c->state_count) return;
    if (state == c->current && !c->fading) return;  // already there, don't restart

    // freeze whatever we're currently showing as the fade-out source
    animation_pose_copy(&c->prev_pose, &c->out_pose);
    c->previous = c->current;
    c->current = state;
    animation_sampler_rewind(&c->states[state].sampler);

    if (duration <= 1e-4f) {
        // instant cut — skip the cross-fade bookkeeping entirely
        c->fading = 0;
        c->fade_t = 0.0f;
    } else {
        c->fading = 1;
        c->fade_t = 0.0f;
        c->fade_dur = duration;
    }
}

int animation_controller_fire(animation_controller *c, int trigger) {
    // first matching transition wins. "any" (from<0) transitions are eligible
    // too, so global events (hurt, die) don't need wiring per state.
    for (int i = 0; i < c->transition_count; i++) {
        const animation_transition *t = &c->transitions[i];
        if (t->trigger != trigger) continue;
        if (t->from >= 0 && t->from != c->current) continue;
        if (t->to == c->current && !c->fading) continue;  // no-op self transition
        animation_controller_play(c, t->to, t->duration);
        return 1;
    }
    return 0;
}

void animation_controller_update(animation_controller *c, float dt) {
    if (c->current < 0) return;  // nothing set yet, out_pose stays at rest

    // advance the active state's playhead
    animation_sampler *cur = &c->states[c->current].sampler;
    animation_sampler_advance(cur, dt);

    // evaluate the incoming state into a scratch pose, seeded from rest so any
    // bones its clip ignores keep their bind transform
    animation_pose cur_pose;
    animation_pose_copy(&cur_pose, &c->rest);
    animation_sampler_evaluate(cur, c->skeleton, &cur_pose);

    if (!c->fading) {
        animation_pose_copy(&c->out_pose, &cur_pose);
        return;
    }

    // progress the cross-fade. smoothstep keeps the blend from snapping at the
    // ends — a straight linear fade reads a touch robotic on fast transitions.
    c->fade_t += dt / c->fade_dur;
    if (c->fade_t >= 1.0f) {
        c->fade_t = 1.0f;
        c->fading = 0;
        c->previous = -1;
        animation_pose_copy(&c->out_pose, &cur_pose);
        return;
    }

    float w = smoothstepf(0.0f, 1.0f, clampf(c->fade_t, 0.0f, 1.0f));
    // blend the frozen outgoing pose toward the live incoming one
    animation_pose_blend(&c->out_pose, &c->prev_pose, &cur_pose, w);
}

const animation_pose *animation_controller_pose(const animation_controller *c) {
    return &c->out_pose;
}
