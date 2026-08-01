#ifndef ENTITY_PROJECTILE_SAVE_H
#define ENTITY_PROJECTILE_SAVE_H

#include "projectile_pool.h"

#include <stddef.h>
#include <stdint.h>

// (de)serialize the live pool to a flat byte blob. only STUCK projectiles are
// worth persisting — a stuck arrow is basically a placed block the player might
// come back for, whereas in-flight ones can just be dropped on save (you werent
// going to track a mid-air arrow across a relog anyway). the host writes the
// returned blob next to its chunk saves.
//
// blob layout (little-endian, mirrors world/save.h's spirit):
// u32 magic  ('PROJ')
// u32 version
// u32 count                 // number of stuck records
// record[count]:
// u8  kind
// i32 stuck_bx, stuck_by, stuck_bz
// f32 stuck_off x,y,z
// f32 forward x,y,z        // so it re-renders at the right angle
// f32 age                  // resume the decay clock
// i32 owner_id

#define PROJECTILE_SAVE_MAGIC   0x4A4F5250u  /* 'PROJ' little-endian */
#define PROJECTILE_SAVE_VERSION 1

// serialize stuck projectiles into out (caller-owned, size out_cap). returns the
// number of bytes written, or 0 if the buffer was too small. pass out=NULL to
// just measure the required size (returned via out_cap semantics below).
size_t projectile_save_write(const projectile_pool *pool,
                             uint8_t *out, size_t out_cap);

// exact byte size a write of this pool would produce. call this to size a buffer.
size_t projectile_save_size(const projectile_pool *pool);

// load stuck projectiles from a blob back into pool. existing slots are left
// alone; loaded ones take fresh slots (skipped if the pool fills). returns the
// number restored, or -1 on a bad magic/version/truncation.
int projectile_save_read(projectile_pool *pool,
                         const uint8_t *data, size_t len);

#endif
