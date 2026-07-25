#include "behavior_prefab.h"
#include "behavior_builder.h"
#include "behavior_actions.h"
#include "../mobs/mob_common.h"
void behavior_prefab_hostile(behavior_tree *out) {
    behavior_tree_init(out);

    behavior_builder b;
    behavior_build_begin(&b, out);
      behavior_open_selector(&b);

        // retaliate: if we just got hit, face and swing.
        behavior_open_sequence(&b);
          behavior_condition(&b, behavior_act_is_hurt, NULL, "hurt?");
          behavior_leaf(&b, behavior_act_face_target, NULL, "face");
          behavior_leaf(&b, behavior_act_attack, NULL, "attack");
        behavior_close(&b);

        // already in range -> attack. gate swings behind a cooldown.
        behavior_open_sequence(&b);
          behavior_condition(&b, behavior_act_target_in_reach, NULL, "reach?");
          behavior_open_cooldown(&b, 0.0f);   // cooldown set from attack_cd bb at tune time
            behavior_leaf(&b, behavior_act_attack, NULL, "attack");
          behavior_close(&b);
        behavior_close(&b);

        // see them -> close the distance.
        behavior_open_sequence(&b);
          behavior_condition(&b, behavior_act_target_in_sight, NULL, "sight?");
          behavior_leaf(&b, behavior_act_chase, NULL, "chase");
        behavior_close(&b);

        // nothing doing -> mill about.
        behavior_leaf(&b, behavior_act_wander, NULL, "wander");

      behavior_close(&b);
    behavior_build_end(&b);
}

void behavior_prefab_passive(behavior_tree *out) {
    behavior_tree_init(out);
behavior_builder b;
behavior_build_begin(&b, out);
behavior_open_selector(&b);
behavior_open_sequence(&b);
behavior_condition(&b, behavior_act_is_hurt, NULL, "hurt?");
behavior_leaf(&b, behavior_act_flee, NULL, "flee");
behavior_close(&b);
behavior_open_sequence(&b);
behavior_condition(&b, behavior_act_target_in_sight, NULL, "sight?");
behavior_leaf(&b, behavior_act_flee, NULL, "flee");
behavior_close(&b);
behavior_leaf(&b, behavior_act_wander, NULL, "wander");
behavior_close(&b);
behavior_build_end(&b);
behavior_bb_set_float(bb, "sight", 16.0f);
behavior_bb_set_float(bb, "reach", 1.6f);
behavior_bb_set_float(bb, "lose", 22.0f);
behavior_bb_set_float(bb, "safe", 14.0f);
behavior_bb_set_float(bb, "wander_radius", 8.0f);
behavior_bb_set_float(bb, "attack_cd", 1.0f);
}
