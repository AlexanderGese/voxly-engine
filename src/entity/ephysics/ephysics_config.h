#ifndef ENTITY_EPHYSICS_CONFIG_H
#define ENTITY_EPHYSICS_CONFIG_H

// shared physics tunables. these were scattered as #defines across half the
// module's .c files (skin, ccd step, etc). pulling them here so the player and
// mob tuning lives in one place; the per-file defines that predate this stay as
// they are because i didnt want a big sweep just to dedupe constants.
//
// units are meters, seconds, m/s unless noted. tickrate is the fixed step the
// engine integrates at; most of the per-tick decay numbers assume it.

#define EPHYS_TICKRATE        20.0f    // fixed steps per second
#define EPHYS_FIXED_DT        (1.0f / EPHYS_TICKRATE)

// the gap kept between a body and a surface after a sweep, so we never rest
// exactly on geometry (which would re-trigger the started-inside guard).
#define EPHYS_CONTACT_SKIN    0.001f

// grounded recheck probe: how far below the feet we look for a floor when the
// sweep didnt land us on one. small, just enough to keep coyote-frame jumps.
#define EPHYS_GROUND_PROBE    0.05f

// max blocks of free fall before fall damage kicks in.
#define EPHYS_SAFE_FALL       3.0f

// horizontal speed below this reads as "stopped" and gets snapped to zero so
// bodies dont creep forever on near-frictionless surfaces.
#define EPHYS_REST_SPEED      0.02f

// buoyancy: how much of normal gravity still applies while submerged. the
// buoyant force is layered on top of this in the fluid pass.
#define EPHYS_WATER_GRAVITY   0.35f

// default acceleration toward a controller's wish-velocity, m/s^2. mobs override
// this; it's here as the sane fallback.
#define EPHYS_DEFAULT_ACCEL   40.0f

#endif
