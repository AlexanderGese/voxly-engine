#include "gbuffer_emit.h"
#include "../../config.h"

#include <string.h>

// per-bucket accumulator. sum of luminance-weighted positions + total weight
// so we can place one light at the centroid with the summed intensity.
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

    // fold every emissive block into its bucket
    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                block_id id = chunk_get_block(c, x, y, z);
                const block_info *bi = block_get(id);
                if (!bi || !bi->emits_light || bi->luminance <= 0) continue;

                float lum = (float)bi->luminance / (float)MAX_LIGHT;
                // block-center world position
                float px = ox + (float)x + 0.5f;
                float py = (float)y + 0.5f;
                float pz = oz + (float)z + 0.5f;

                int bi_x = x / GBUFFER_EMIT_BUCKET;
                int bi_y = y / GBUFFER_EMIT_BUCKET;
                int bi_z = z / GBUFFER_EMIT_BUCKET;
                emit_bucket *b = &buckets[(bi_y * BZ + bi_z) * BX + bi_x];

                vec3 tint = gbuffer_emit_tint(id);
                b->wx += px * lum;
                b->wy += py * lum;
                b->wz += pz * lum;
                b->wr += tint.x * lum;
                b->wg += tint.y * lum;
                b->wb += tint.z * lum;
                b->weight += lum;
                b->count++;
            }
        }
    }

    int emitted = 0;
    for (int i = 0; i < BX * BY * BZ; i++) {
        emit_bucket *b = &buckets[i];
        if (b->count == 0 || b->weight <= 0.0f) continue;

        vec3 pos   = vec3_new(b->wx / b->weight,
                              b->wy / b->weight,
                              b->wz / b->weight);
        vec3 color = vec3_new(b->wr / b->weight,
                              b->wg / b->weight,
                              b->wb / b->weight);

        // intensity scales with how much luminance we merged, but with
        // diminishing returns so a cluster of torches isn't a supernova
        float intensity = b->weight;
        if (intensity > 4.0f) intensity = 4.0f + (intensity - 4.0f) * 0.25f;

        gbuffer_light_add_point(out, pos, color, intensity, 0.0f);
        emitted++;
    }
    return emitted;
}
