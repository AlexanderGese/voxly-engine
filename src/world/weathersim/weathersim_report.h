#ifndef WORLD_WEATHERSIM_REPORT_H
#define WORLD_WEATHERSIM_REPORT_H

#include <stddef.h>
#include "weathersim.h"

// human-readable telemetry for the weather sim. this is the debug-overlay /
// log-line side of the house: it doesn't drive anything, it just formats the
// current sim state into strings so you can see what the sky is thinking. i
// leaned on this constantly while tuning the front rates; left it in because a
// barometer readout in the corner is genuinely nice.

// one-line summary into `buf`: state at the player, wind, pressure, live front
// count. returns the number of chars written (excluding the nul), clamped to
// cap. safe with cap == 0 (writes nothing, returns 0).
size_t weathersim_report_line(const weathersim_ctx *ws, vec3 player_pos,
                              char *buf, size_t cap);

// multi-line dump of every live front: kind, lifecycle, age, position, depth.
// for an expanded debug panel. returns chars written.
size_t weathersim_report_fronts(const weathersim_ctx *ws,
                                char *buf, size_t cap);

// a single readable word for a precip kind, for the hud.
const char *weathersim_report_precip_name(weathersim_precip p);

// barometric trend arrow: compares the player cell pressure to the windward
// neighbour so you can tell if it's rising or falling. returns -1/0/+1.
int weathersim_report_baro_trend(const weathersim_ctx *ws, vec3 player_pos);

#endif
