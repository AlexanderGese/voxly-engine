#include "spawn_rules.h"
#include "mob_util.h"

// crude minecraft-ish rules: monsters in the dark, animals on sunny grass.

#define VOXL_DARK_THRESHOLD  7   // light <= this counts as "dark enough"

int voxl_spawn_is_valid(voxl_mob_kind kind, const voxl_spawn_ctx *ctx) {
    if (kind == VOXL_MOB_NONE || !ctx) return 0;
    // everything needs solid footing and dislikes drowning.
    if (!ctx->on_solid) return 0;
    if (ctx->in_water) return 0;

    if (voxl_mob_is_hostile(kind)) {
        return ctx->light <= VOXL_DARK_THRESHOLD;
    }
    if (voxl_mob_is_passive(kind)) {
        // animals spawn in daylight on the surface.
        return ctx->sky_visible && ctx->light >= 9;
    }
    return 0;
}

int voxl_spawn_under_cap(voxl_mob_kind kind, int current_of_kind) {
    // rough soft caps per kind. keeps the world from filling up.
    int cap;
    if (voxl_mob_is_hostile(kind)) cap = 24;
    else if (voxl_mob_is_passive(kind)) cap = 16;
    else cap = 0;
    return current_of_kind < cap;
}

voxl_mob_kind voxl_spawn_pick(const voxl_spawn_ctx *ctx, voxl_mob_rng *r) {
    if (!ctx) return VOXL_MOB_NONE;

    if (ctx->light <= VOXL_DARK_THRESHOLD && ctx->on_solid && !ctx->in_water) {
        // dark: weighted roll across hostiles.
        // zombie most common, creeper/skeleton next, spider least.
        int roll = voxl_mob_rng_range(r, 0, 99);
        voxl_mob_kind k;
        if      (roll < 45) k = VOXL_MOB_ZOMBIE;
        else if (roll < 70) k = VOXL_MOB_SKELETON;
        else if (roll < 88) k = VOXL_MOB_CREEPER;
        else                k = VOXL_MOB_SPIDER;
        return voxl_spawn_is_valid(k, ctx) ? k : VOXL_MOB_NONE;
    }

    if (ctx->sky_visible && ctx->light >= 9 && ctx->on_solid && !ctx->in_water) {
        // daylight surface: pick a farm animal.
        static const voxl_mob_kind animals[] = {
            VOXL_MOB_COW, VOXL_MOB_PIG, VOXL_MOB_CHICKEN, VOXL_MOB_SHEEP
        };
        int n = (int)(sizeof animals / sizeof animals[0]);
        voxl_mob_kind k = animals[voxl_mob_rng_range(r, 0, n - 1)];
        return voxl_spawn_is_valid(k, ctx) ? k : VOXL_MOB_NONE;
    }

    return VOXL_MOB_NONE;
}
