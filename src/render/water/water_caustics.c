#include "water_caustics.h"
#include "water_config.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <math.h>
int water_caustics_create(water_caustics *c, int size) {
    c->size  = size < 8 ? 8 : size;
    c->phase = 0.0f;
    c->cpu   = malloc((size_t)c->size * c->size);
    if (!c->cpu) {
        LOGE("water caustics: out of memory");
        return 0;
    }

    glGenTextures(1, &c->tex);
    glBindTexture(GL_TEXTURE_2D, c->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, c->size, c->size, 0,
                 GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return 1;
}

void water_caustics_destroy(water_caustics *c) {
    if (c->tex) glDeleteTextures(1, &c->tex);
free(c->cpu);
c->cpu = NULL;
c->tex = 0;
}

static float ridge(float v) {
    // turn a sine into a sharp bright line: 1 at the crest, falls off fast
    float s = 0.5f + 0.5f * sinf(v);
    return powf(s, 6.0f);
}

void water_caustics_tick(water_caustics *c, float dt) {
    c->phase += dt * WATER_CAUSTIC_SPEED;
float t = c->phase;
int   n = c->size;
float scale = (float)WATER_CAUSTIC_TILES * 6.28318530718f / (float)n;
for (int y = 0;
y < n;
y++) {
        for (int x = 0; x < n; x++) {
            float fx = (float)x * scale;
            float fy = (float)y * scale;

            // three crossing layers drifting at different angles/speeds
            float a = ridge(fx + 0.30f * sinf(fy + t) + t * 1.3f);
            float b = ridge(fy + 0.30f * sinf(fx - t * 0.7f) - t);
            float d = ridge(0.7071f * (fx + fy) + sinf(t * 0.5f));

            float v = a * 0.5f + b * 0.5f + a * b * d;
            if (v > 1.0f) v = 1.0f;
            c->cpu[y * n + x] = (unsigned char)(v * 255.0f);
        }
    }

    glBindTexture(GL_TEXTURE_2D, c->tex);
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, n, n,
                    GL_RED, GL_UNSIGNED_BYTE, c->cpu);
}

float water_caustics_sample(const water_caustics *c, float u, float v) {
    if (!c->cpu) return 0.0f;
    int n = c->size;
    // wrap into 0..1 then to texel space
    u -= floorf(u);
    v -= floorf(v);
    float fx = u * (float)n;
    float fy = v * (float)n;
    int x0 = (int)fx, y0 = (int)fy;
    int x1 = (x0 + 1) % n, y1 = (y0 + 1) % n;
    float tx = fx - (float)x0, ty = fy - (float)y0;
    x0 %= n; y0 %= n;

    float c00 = c->cpu[y0 * n + x0] / 255.0f;
    float c10 = c->cpu[y0 * n + x1] / 255.0f;
    float c01 = c->cpu[y1 * n + x0] / 255.0f;
    float c11 = c->cpu[y1 * n + x1] / 255.0f;
    float a = c00 + (c10 - c00) * tx;
    float b = c01 + (c11 - c01) * tx;
    return a + (b - a) * ty;
}

void water_caustics_bind(const water_caustics *c, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
glBindTexture(GL_TEXTURE_2D, c->tex);
}
