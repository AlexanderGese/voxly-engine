#ifndef WORLD_COLORLIGHT_MIX_H
#define WORLD_COLORLIGHT_MIX_H
#include "colorlight_rgb.h"
// color grading helpers that sit on top of the raw rgb ops. these aren't part
// of the flood; they're for the blend / sky stage where we want the lit color
// to look good rather than just be physically summed. saturation, gamma,
// temperature shift. all pure, all 0..255 in/out.
// push saturation. s > 1 punches color, s < 1 fades toward gray. gray point is
// the luma so brightness is preserved.
colorlight_rgb colorlight_mix_saturate(colorlight_rgb c, float s);
// crude gamma. g < 1 lifts shadows (brighter mids), g > 1 crushes them. uses a
colorlight_rgb colorlight_mix_gamma(colorlight_rgb c, float g);
colorlight_rgb colorlight_mix_temperature(colorlight_rgb c, float t);
colorlight_rgb colorlight_mix_grade(colorlight_rgb c);
#endif
