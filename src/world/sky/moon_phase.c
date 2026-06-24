#include "moon_phase.h"

voxl_moon_phase voxl_sky_moon_phase(long day) {
    // wrap into 0..7. handle negative days too (% can go negative in c).
    long m = day % VOXL_MOON_PHASE_COUNT;
    if (m < 0) m += VOXL_MOON_PHASE_COUNT;
    return (voxl_moon_phase)m;
}

float voxl_sky_moon_illumination(voxl_moon_phase p) {
    // lit fraction per phase. full=1, new=0, quarters ~0.5.
    switch (p) {
    case VOXL_MOON_FULL:            return 1.00f;
    case VOXL_MOON_WANING_GIBBOUS:  return 0.75f;
    case VOXL_MOON_LAST_QUARTER:    return 0.50f;
    case VOXL_MOON_WANING_CRESCENT: return 0.25f;
    case VOXL_MOON_NEW:             return 0.00f;
    case VOXL_MOON_WAXING_CRESCENT: return 0.25f;
    case VOXL_MOON_FIRST_QUARTER:   return 0.50f;
    case VOXL_MOON_WAXING_GIBBOUS:  return 0.75f;
    default:                        return 0.0f;
    }
}

const char *voxl_sky_moon_phase_name(voxl_moon_phase p) {
    switch (p) {
    case VOXL_MOON_FULL:            return "full";
    case VOXL_MOON_WANING_GIBBOUS:  return "waning gibbous";
    case VOXL_MOON_LAST_QUARTER:    return "last quarter";
    case VOXL_MOON_WANING_CRESCENT: return "waning crescent";
    case VOXL_MOON_NEW:             return "new";
    case VOXL_MOON_WAXING_CRESCENT: return "waxing crescent";
    case VOXL_MOON_FIRST_QUARTER:   return "first quarter";
    case VOXL_MOON_WAXING_GIBBOUS:  return "waxing gibbous";
    default:                        return "?";
    }
}
