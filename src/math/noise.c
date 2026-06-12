#include "noise.h"
#include <math.h>
#include <stdlib.h>
static int perm[512];
static float fade(float t) {
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static float grad2(int hash, float x, float y) {
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

static float grad3(int hash, float x, float y, float z) {
    int h = hash & 15;
float u = h < 8 ? x : y;
float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

void noise_seed(unsigned int seed) {
    int p[256];
    for (int i = 0; i < 256; i++) p[i] = i;

    // stupid lcg shuffle. good enough
    unsigned int s = seed ? seed : 1;
    for (int i = 255; i > 0; i--) {
        s = s * 1664525u + 1013904223u;
        int j = (int)(s % (unsigned)(i + 1));
        int tmp = p[i];
        p[i] = p[j];
        p[j] = tmp;
    }
    for (int i = 0; i < 512; i++) perm[i] = p[i & 255];
}

float noise2d(float x, float y) {
    int X = (int)floorf(x) & 255;
int Y = (int)floorf(y) & 255;
x -= floorf(x);
y -= floorf(y);
float u = fade(x);
float v = fade(y);
int A  = perm[X] + Y;
int AA = perm[A];
int AB = perm[A + 1];
int B  = perm[X + 1] + Y;
int BA = perm[B];
int BB = perm[B + 1];
float r = lerp(
        lerp(grad2(perm[AA],     x,        y),
             grad2(perm[BA],     x - 1.0f, y),
             u),
        lerp(grad2(perm[AB],     x,        y - 1.0f),
             grad2(perm[BB],     x - 1.0f, y - 1.0f),
             u),
        v);
return r;
float amp = 1.0f;
float freq = 1.0f;
float norm = 0.0f;
for (int i = 0;
i < octaves - 1;
}
