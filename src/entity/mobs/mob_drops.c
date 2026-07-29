#include "mob_drops.h"

// small helper: push a stack if there's room. returns the new write index.
static int voxl_drop_push(voxl_drop *out, int n, int max,
                          voxl_drop_item item, int count) {
    if (n >= max || count <= 0 || item == VOXL_DROP_NONE) return n;
    out[n].item  = item;
    out[n].count = count;
    return n + 1;
}

const char *voxl_drop_name(voxl_drop_item item) {
    switch (item) {
    case VOXL_DROP_ROTTEN_FLESH: return "rotten flesh";
    case VOXL_DROP_BONE:         return "bone";
    case VOXL_DROP_ARROW:        return "arrow";
    case VOXL_DROP_GUNPOWDER:    return "gunpowder";
    case VOXL_DROP_STRING:       return "string";
    case VOXL_DROP_SPIDER_EYE:   return "spider eye";
    case VOXL_DROP_RAW_BEEF:     return "raw beef";
    case VOXL_DROP_COOKED_BEEF:  return "steak";
    case VOXL_DROP_LEATHER:      return "leather";
    case VOXL_DROP_RAW_PORK:     return "raw porkchop";
    case VOXL_DROP_COOKED_PORK:  return "cooked porkchop";
    case VOXL_DROP_RAW_CHICKEN:  return "raw chicken";
    case VOXL_DROP_COOKED_CHICKEN: return "cooked chicken";
    case VOXL_DROP_FEATHER:      return "feather";
    case VOXL_DROP_WOOL:         return "wool";
    case VOXL_DROP_MUTTON:       return "mutton";
    case VOXL_DROP_COOKED_MUTTON: return "cooked mutton";
    default:                     return "nothing";
    }
}

int voxl_mob_roll_drops(voxl_mob_kind kind, int burning,
                        voxl_mob_rng *r, voxl_drop *out, int max) {
    int n = 0;
    if (!out || max <= 0) return 0;

    switch (kind) {
    case VOXL_MOB_ZOMBIE:
        n = voxl_drop_push(out, n, max, VOXL_DROP_ROTTEN_FLESH,
                           voxl_mob_rng_range(r, 0, 2));
        break;

    case VOXL_MOB_SKELETON:
        n = voxl_drop_push(out, n, max, VOXL_DROP_BONE,
                           voxl_mob_rng_range(r, 0, 2));
        n = voxl_drop_push(out, n, max, VOXL_DROP_ARROW,
                           voxl_mob_rng_range(r, 0, 2));
        break;

    case VOXL_MOB_CREEPER:
        n = voxl_drop_push(out, n, max, VOXL_DROP_GUNPOWDER,
                           voxl_mob_rng_range(r, 0, 2));
        break;

    case VOXL_MOB_SPIDER:
        n = voxl_drop_push(out, n, max, VOXL_DROP_STRING,
                           voxl_mob_rng_range(r, 0, 2));
        // spiders occasionally drop an eye.
        if (voxl_mob_rng_chance(r, 0.33f)) {
            n = voxl_drop_push(out, n, max, VOXL_DROP_SPIDER_EYE, 1);
        }
        break;

    case VOXL_MOB_COW:
        n = voxl_drop_push(out, n, max,
                           burning ? VOXL_DROP_COOKED_BEEF : VOXL_DROP_RAW_BEEF,
                           voxl_mob_rng_range(r, 1, 3));
        n = voxl_drop_push(out, n, max, VOXL_DROP_LEATHER,
                           voxl_mob_rng_range(r, 0, 2));
        break;

    case VOXL_MOB_PIG:
        n = voxl_drop_push(out, n, max,
                           burning ? VOXL_DROP_COOKED_PORK : VOXL_DROP_RAW_PORK,
                           voxl_mob_rng_range(r, 1, 3));
        break;

    case VOXL_MOB_CHICKEN:
        n = voxl_drop_push(out, n, max,
                           burning ? VOXL_DROP_COOKED_CHICKEN : VOXL_DROP_RAW_CHICKEN, 1);
        n = voxl_drop_push(out, n, max, VOXL_DROP_FEATHER,
                           voxl_mob_rng_range(r, 0, 2));
        break;

    case VOXL_MOB_SHEEP:
        n = voxl_drop_push(out, n, max, VOXL_DROP_WOOL, 1);
        n = voxl_drop_push(out, n, max,
                           burning ? VOXL_DROP_COOKED_MUTTON : VOXL_DROP_MUTTON,
                           voxl_mob_rng_range(r, 1, 2));
        break;

    default:
        break;
    }

    return n;
}
