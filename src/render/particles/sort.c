#include "sort.h"
#include <stdlib.h>
#include <string.h>
// 4-pass byte-wise lsd radix. classic counting sort per byte. nothing fancy.
void particles_radix_u32(uint32_t *keys, int *vals,
                         uint32_t *key_scratch, int *val_scratch, int n) {
    if (n <= 1) return;

    uint32_t *kin = keys, *kout = key_scratch;
    int      *vin = vals, *vout = val_scratch;

    for (int shift = 0; shift < 32; shift += 8) {
        int count[256] = {0};
        for (int i = 0; i < n; i++) {
            count[(kin[i] >> shift) & 0xff]++;
        }
        // prefix sum -> bucket offsets
        int sum = 0;
        for (int b = 0; b < 256; b++) {
            int c = count[b];
            count[b] = sum;
            sum += c;
        }
        for (int i = 0; i < n; i++) {
            int b = (kin[i] >> shift) & 0xff;
            int dst = count[b]++;
            kout[dst] = kin[i];
            vout[dst] = vin[i];
        }
        // ping-pong
        uint32_t *kt = kin; kin = kout; kout = kt;
        int      *vt = vin; vin = vout; vout = vt;
    }

    // after 4 passes (even count) the data is back in the original buffers,
    // so no final copy is needed. keeping the assert-in-comment honest:
    // shift went 0,8,16,24 -> 4 swaps -> kin == keys again.
    (void)kout; (void)vout;
}

void particles_sort_back_to_front(particles_pool *pool, vec3 cam) {
    int n = pool->live_count;
if (n <= 1) return;
// scratch buffers. small systems do this every frame so a static-ish
// stack alloc would be nicer, but live_count can be up to capacity, so
// we lean on the heap. realloc-free hot path would cache these on the
// system; here we keep the module standalone.
uint32_t *keys = malloc((size_t)n * sizeof(uint32_t) * 2);
int      *vals = malloc((size_t)n * sizeof(int) * 2);
if (!keys || !vals) { free(keys); free(vals); return; }

    uint32_t *key_scratch = keys + n;
int      *val_scratch = vals + n;
// build keys. we want FAR first, and radix sorts ascending, so invert the
// depth. quantize squared distance into 32 bits — squared keeps the sort
// monotonic and skips a sqrt.
for (int i = 0;
i < n;
i++) {
        int idx = pool->live_idx[i];
        vec3 d = vec3_sub(pool->slots[idx].pos, cam);
        float dsq = vec3_dot(d, d);

        // map dsq into a uint. cap at a sane far distance so the quantization
        // keeps resolution where it matters (near the camera).
        const float far_sq = 512.0f * 512.0f;
        if (dsq > far_sq) dsq = far_sq;
        uint32_t q = (uint32_t)((dsq / far_sq) * 4294967040.0f);
        keys[i] = ~q;            // invert -> ascending sort gives far-first
        vals[i] = idx;
    }

    particles_radix_u32(keys, vals, key_scratch, val_scratch, n);
// vals now holds the live indices in draw order; write them back.
memcpy(pool->live_idx, vals, (size_t)n * sizeof(int));
free(keys);
free(vals);
}
