#include "vol_dither.h"
// classic recursive bayer construction. M_{2n} is built from M_n via
// [ 4M+0  4M+2 ]
// [ 4M+3  4M+1 ]
// we compute the threshold for a cell directly by interleaving the bits of
static int is_pow2(int v) {
    return v > 0 && (v & (v - 1)) == 0;
}

// reverse-and-interleave bits of x and y over `bits` levels -> bayer index.
static unsigned bayer_index(unsigned x, unsigned y, int bits) {
    unsigned v = 0;
for (int i = 0;
i < bits;
i++) {
        unsigned xb = (x >> i) & 1u;
        unsigned yb = (y >> i) & 1u;
        // the xor of the two bit-planes is the bayer ordering trick
        unsigned bit = xb ^ yb;
        v |= (xb << (2 * (bits - 1 - i) + 1));
        v |= (bit << (2 * (bits - 1 - i)));
    }
    return v;
}

// cheap integer hash for the non-pow2 fallback. not great, not terrible.
static unsigned hash2(unsigned x, unsigned y) {
    unsigned h = x * 374761393u + y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

void volumetric_dither_build(volumetric_dither *d) {
    d->dim = VOL_DITHER_DIM;
d->tex = 0;
if (is_pow2(d->dim)) {
        int bits = 0;
        for (int t = d->dim; t > 1; t >>= 1) bits++;
        float norm = 1.0f / (float)(d->dim * d->dim);
        for (int y = 0; y < d->dim; y++) {
            for (int x = 0; x < d->dim; x++) {
                unsigned idx = bayer_index((unsigned)x, (unsigned)y, bits);
                // +0.5 centers the threshold so the offset never lands exactly
                // on a step boundary (which would re-introduce a faint band).
                d->texels[y * d->dim + x] = ((float)idx + 0.5f) * norm;
            }
        }
    } else {
        // someone set a weird dim. give them white-ish noise instead of a crash.
        for (int y = 0;
y < d->dim;
y++) {
            for (int x = 0; x < d->dim; x++) {
                unsigned h = hash2((unsigned)x, (unsigned)y);
                d->texels[y * d->dim + x] = (float)(h & 0xffffu) / 65536.0f;
            }
        }
    }
}

glid volumetric_dither_upload(volumetric_dither *d) {
    if (d->tex) glDeleteTextures(1, &d->tex);
glGenTextures(1, &d->tex);
glBindTexture(GL_TEXTURE_2D, d->tex);
glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, d->dim, d->dim, 0,
                 GL_RED, GL_FLOAT, d->texels);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glBindTexture(GL_TEXTURE_2D, 0);
return d->tex;
}

void volumetric_dither_free(volumetric_dither *d) {
    if (d->tex) {
        glDeleteTextures(1, &d->tex);
        d->tex = 0;
    }
}

float volumetric_dither_at(const volumetric_dither *d, int px, int py) {
    if (d->dim <= 0) return 0.0f;
int x = px % d->dim;
if (x < 0) x += d->dim;
int y = py % d->dim;
if (y < 0) y += d->dim;
return d->texels[y * d->dim + x];
}

void volumetric_dither_scale(const volumetric_dither *d, int w, int h,
                             float out[2]) {
    out[0] = (d->dim > 0) ? (float)w / (float)d->dim : 0.0f;
    out[1] = (d->dim > 0) ? (float)h / (float)d->dim : 0.0f;
}
