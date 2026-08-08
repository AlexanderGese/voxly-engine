#ifndef PLAYER_CRAFTING_SERIAL_H
#define PLAYER_CRAFTING_SERIAL_H

#include <stddef.h>

// pack/unpack the per-player crafting progress (which recipes are unlocked,
// plus the usage stats) into a flat blob for the world save. the recipe *data*
// is not saved, it's rebuilt from code at load; we only persist progress, keyed
// by recipe index. if the recipe set grows between builds new ids just default
// to locked/zero, which is the behaviour we want.
//
// note: we save by index, not name. that's a bit fragile if recipes get
// reordered, but the registration order is append-only in practice so it holds.

#define CRAFT_SAVE_MAGIC    0x43524654u   /* 'CRFT' */
#define CRAFT_SAVE_VERSION  1

// serialize unlock bitset + stats into a freshly malloc'd buffer. *out_size is
// set to the byte count. caller frees. returns NULL on alloc failure.
void *craft_save_to_buffer(size_t *out_size);

// load from a buffer produced by craft_save_to_buffer. returns 0 on success,
// negative on bad magic / version / truncation. on failure nothing is applied,
// the in-memory state is left as it was.
int   craft_load_from_buffer(const void *buf, size_t size);

// convenience wrappers around util/file. same return convention.
int   craft_save_to_file(const char *path);
int   craft_load_from_file(const char *path);

#endif
