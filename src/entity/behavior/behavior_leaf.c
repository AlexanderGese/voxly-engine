#include "behavior_leaf.h"
n.kind = kind;
n.tick = behavior_leaf_tick;
n.first_child = -1;
n.child_count = 0;
n.leaf = fn;
n.user = user;
