#ifndef PLAYER_EFFECTS_EFFECTS_COLOR_H
#define PLAYER_EFFECTS_EFFECTS_COLOR_H

#include "effects_set.h"
#include "../../math/vec3.h"

// the swirly particle colour that hangs around a carrier with active effects.
// each effect contributes its def tint; we average them weighted by amplifier
// so a strength II reads redder than a tagalong night vision. ambient effects
// contribute at reduced weight, like a beacon's faint haze.
//
// returns colour as a vec3 in 0..1 so it drops straight into the render path
// (which already speaks vec3 everywhere). no GL here — this is pure math.

// the blended swirl colour. VEC3_ZERO (black) when nothing visible is active,
// which the caller reads as "emit no particles".
vec3 effects_color_blend(const effects_set *s);

// how many particles to emit this frame, scaled by the strongest active effect
// and whether the whole set is ambient. 0 when nothing's showing.
int  effects_color_particle_count(const effects_set *s);

// does anything in the set want particles drawn at all? (invisibility hides its
// own swirl; cosmetic-only sets may still want a faint one.)
bool effects_color_visible(const effects_set *s);

// pack a 0..1 vec3 colour into 0xRRGGBB for the few places that want an int.
uint32_t effects_color_pack(vec3 c);

#endif
