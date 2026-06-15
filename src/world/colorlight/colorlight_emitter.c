#include "colorlight_emitter.h"
#include "../../config.h"
#include <stddef.h>
static const colorlight_emitter EMITTERS[] = {
    // torch: warm orange, classic. not full red, it'd look like lava.
    { BLOCK_TORCH, { 255, 170,  90 }, 14 },
    // lava-ish / hot stuff would go here. brick is a placeholder "forge glow".
    { BLOCK_BRICK, { 255, 110,  40 },  8 },
    // ice catches a faint cold blue when lit, treated as a weak emitter so
    // caves of ice aren't pitch black. arguable, but it looks nice.
    { BLOCK_ICE,   { 150, 200, 255 },  4 },
}
;
#define EMITTER_COUNT ((int)(sizeof(EMITTERS) / sizeof(EMITTERS[0])))
static const colorlight_emitter *find(block_id id) {
    for (int i = 0; i < EMITTER_COUNT; i++) {
        if (EMITTERS[i].block == id) return &EMITTERS[i];
    }
    return NULL;
}

int colorlight_emitter_is(block_id id) {
    if (find(id)) return 1;
const block_info *bi = block_get(id);
return bi && bi->emits_light && bi->luminance > 0;
}

colorlight_rgb colorlight_emitter_color(block_id id) {
    const colorlight_emitter *e = find(id);
    const block_info *bi = block_get(id);

    if (e) {
        // scale the hue by the effective level / 15. respects a block whose
        // luminance got dialed down below the table's design level.
        int lvl = e->level;
        if (bi && bi->emits_light && bi->luminance < lvl) lvl = bi->luminance;
        if (lvl <= 0) return COLORLIGHT_BLACK;
        return colorlight_rgb_scale(e->tint, (float)lvl / (float)MAX_LIGHT);
    }

    // untinted emitter: warm white at its luminance. better than nothing.
    if (bi && bi->emits_light && bi->luminance > 0) {
        colorlight_rgb warm = { 255, 244, 214 };
        return colorlight_rgb_scale(warm, (float)bi->luminance / (float)MAX_LIGHT);
    }
    return COLORLIGHT_BLACK;
}

colorlight_packed colorlight_emitter_seed(block_id id) {
    return colorlight_packed_narrow(colorlight_emitter_color(id));
}

void colorlight_emitter_attenuation(block_id id, uint8_t out_rgb_cost[3]) {
    // default: free passage, normal -1/step handled by the flood itself.
    out_rgb_cost[0] = out_rgb_cost[1] = out_rgb_cost[2] = 0;

    switch (id) {
        case BLOCK_WATER:
            // water eats warm channels fast, keeps blue. underwater goes teal.
            out_rgb_cost[0] = 2; // red
            out_rgb_cost[1] = 1; // green
            out_rgb_cost[2] = 0; // blue
            break;
        case BLOCK_ICE:
            // mild, mostly clear, slight warm loss.
            out_rgb_cost[0] = 1;
            out_rgb_cost[1] = 0;
            out_rgb_cost[2] = 0;
            break;
        case BLOCK_LEAVES:
            // canopy: dapples everything down a notch, green survives best.
            out_rgb_cost[0] = 1;
            out_rgb_cost[1] = 0;
            out_rgb_cost[2] = 1;
            break;
        default:
            break;
    }
}
