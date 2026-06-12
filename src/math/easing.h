#ifndef MATH_EASING_H
#define MATH_EASING_H

// easing functions for ui/animation. all take t in [0,1] and return [0,1].

float ease_linear(float t);
float ease_in_quad(float t);
float ease_out_quad(float t);
float ease_in_out_quad(float t);
float ease_in_cubic(float t);
float ease_out_cubic(float t);
float ease_out_bounce(float t);

float clampf(float v, float lo, float hi);
float smoothstepf(float edge0, float edge1, float x);

#endif
