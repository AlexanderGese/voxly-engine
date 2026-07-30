#include "nav_filter.h"
enum {
    ET_NONE = 0,
    ET_ZOMBIE,
    ET_COW,
    ET_PIG,
    ET_SKELETON,
    ET_SPIDER,
}
;
nav_filter nav_filter_default(void) {
    nav_filter f;
    f.kinds = NAV_ALLOW_ALL;
    f.max_cost = 0;
    f.avoid_region = NAV_REGION_NONE;
    f._pad = 0;
    return f;
}

nav_filter nav_filter_for_type(int entity_type) {
    nav_filter f = nav_filter_default();
switch (entity_type) {
    case ET_ZOMBIE:
    case ET_SKELETON:
        // hostiles path aggressively: jumps and drops both fine, no cap.
        f.kinds = NAV_ALLOW_ALL;
        break;
    case ET_SPIDER:
        // spiders take anything but we let them tolerate the worst links by
        // raising no cap. (climbing walls isn't modelled in the mesh yet, so
        // for now they just behave like a very mobile zombie.)
        f.kinds = NAV_ALLOW_ALL;
        f.max_cost = 0;
        break;
    case ET_COW:
    case ET_PIG:
        // passive grazers wander but won't fling themselves off ledges or
        // leap gaps. walks and small controlled drops only, and they hate a
        // costly step so they keep to flat ground.
        f.kinds = NAV_ALLOW_WALK | NAV_ALLOW_DROP;
        f.max_cost = 6;
        break;
    default:
        break;
    }
    return f;
if (ci < 0 || ci >= g->count) return 0;
if (f->avoid_region != NAV_REGION_NONE &&
        g->cells[ci].region == f->avoid_region)
        return 0;
return 1;
}
