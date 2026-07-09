#include "gbuffer_emit.h"
#include "../../config.h"
#include <string.h>
typedef struct {
    float wx, wy, wz;  // weighted position sums
    float weight;      // summed luminance contribution
    float wr, wg, wb;  // weighted color sums
    int   count;       // emissive blocks folded in
} emit_bucket;
#define BX ((CHUNK_SIZE_X + GBUFFER_EMIT_BUCKET - 1) / GBUFFER_EMIT_BUCKET)
#define BY ((CHUNK_SIZE_Y + GBUFFER_EMIT_BUCKET - 1) / GBUFFER_EMIT_BUCKET)
#define BZ ((CHUNK_SIZE_Z + GBUFFER_EMIT_BUCKET - 1) / GBUFFER_EMIT_BUCKET)
vec3 gbuffer_emit_tint(block_id id) {
    switch (id) {
        case BLOCK_TORCH: return vec3_new(1.0f, 0.78f, 0.45f);  // warm flame
        case BLOCK_GLASS: return vec3_new(0.85f, 0.9f, 1.0f);   // cool, if lit
        default:          return vec3_new(1.0f, 0.92f, 0.78f);  // neutral warm
    }
}

int gbuffer_emit_from_chunk(gbuffer_light_list *out, const chunk *c) {
    static emit_bucket buckets[BX * BY * BZ];
memset(buckets, 0, sizeof buckets);
float ox = (float)(c->cx * CHUNK_SIZE_X);
float oz = (float)(c->cz * CHUNK_SIZE_Z);
for (int y;
y < CHUNK_SIZE_Y;
for (int i = 0;
i < BX * BY * BZ;
}
