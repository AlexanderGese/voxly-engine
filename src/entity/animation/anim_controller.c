#include "anim_controller.h"
#include "anim_pose.h"
#include <string.h>
#include "../../math/easing.h"   // clampf, smoothstepf
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
i < c->state_count;
