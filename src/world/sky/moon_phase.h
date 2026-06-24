#ifndef WORLD_SKY_MOON_PHASE_H
#define WORLD_SKY_MOON_PHASE_H

// 8-step moon phase, minecraft style. driven by an in-game day counter.
// phase 0 = full moon, 4 = new moon, wraps every 8 days.

typedef enum {
    VOXL_MOON_FULL = 0,
    VOXL_MOON_WANING_GIBBOUS,
    VOXL_MOON_LAST_QUARTER,
    VOXL_MOON_WANING_CRESCENT,
    VOXL_MOON_NEW,
    VOXL_MOON_WAXING_CRESCENT,
    VOXL_MOON_FIRST_QUARTER,
    VOXL_MOON_WAXING_GIBBOUS,
    VOXL_MOON_PHASE_COUNT
} voxl_moon_phase;

// phase index for a given (0-based) world day.
voxl_moon_phase voxl_sky_moon_phase(long day);

// 0..1 lit fraction of the disc (1 = full, 0 = new).
float voxl_sky_moon_illumination(voxl_moon_phase p);

// human readable name.
const char *voxl_sky_moon_phase_name(voxl_moon_phase p);

#endif
