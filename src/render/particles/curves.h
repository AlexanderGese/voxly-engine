#ifndef RENDER_PARTICLES_CURVES_H
#define RENDER_PARTICLES_CURVES_H

// over-lifetime curves. a particle's color/size/alpha are not stored as
// keyframes per particle — instead the emitter holds one small gradient and
// we evaluate it at t = age/life. cheap, and it makes a smoke puff look like
// a smoke puff without any extra per-particle memory.

#include "../../math/vec4.h"

// gradient stops are kept sorted by t. 4 is plenty for everything we do;
// fire wants ~3 (bright -> orange -> dark), smoke wants 2.
#define PARTICLES_CURVE_MAX_STOPS 4

typedef struct {
    float t;       // 0..1 along lifetime
    float value;
} particles_curve_key;

typedef struct {
    particles_curve_key keys[PARTICLES_CURVE_MAX_STOPS];
    int count;
} particles_curve;

typedef struct {
    float t;       // 0..1 along lifetime
    vec4  color;   // rgba at this stop
} particles_grad_key;

typedef struct {
    particles_grad_key keys[PARTICLES_CURVE_MAX_STOPS];
    int count;
} particles_gradient;

// scalar curve -------------------------------------------------------------
void  particles_curve_init(particles_curve *c);
void  particles_curve_add(particles_curve *c, float t, float value);
float particles_curve_eval(const particles_curve *c, float t);

// a flat constant, since "size is always 1" is the common case.
particles_curve particles_curve_const(float value);

// a simple two-key ramp.
particles_curve particles_curve_ramp(float a, float b);

// color gradient -----------------------------------------------------------
void particles_gradient_init(particles_gradient *g);
void particles_gradient_add(particles_gradient *g, float t, vec4 color);
vec4 particles_gradient_eval(const particles_gradient *g, float t);

#endif
