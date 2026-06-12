#ifndef WORLD_DAYNIGHT_H
#define WORLD_DAYNIGHT_H

// day/night cycle. time in [0, 24) hours. used to drive skybox colors
// and sun light multiplier.

typedef struct {
    float hours;          // 0..24
    float rate;           // hours per real second
} daynight;

void  daynight_init(daynight *d, float start_hour);
void  daynight_update(daynight *d, float dt);
float daynight_sun_factor(const daynight *d);   // 0..1
void  daynight_sky_color(const daynight *d, float *r, float *g, float *b);
int   daynight_is_night(const daynight *d);

#endif
