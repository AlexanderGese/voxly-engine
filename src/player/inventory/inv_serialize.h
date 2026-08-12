#ifndef PLAYER_INVENTORY_SERIALIZE_H
#define PLAYER_INVENTORY_SERIALIZE_H

#include "inv_types.h"
#include "inv_player.h"
#include <stddef.h>

// pack/unpack a player inventory into a flat byte blob for the save file.
// the world save (see config.h SAVE_MAGIC/SAVE_VERSION) calls into this for
// the inventory section. we write item *names* not raw ids so the save stays
// valid even if the registry order shifts between builds; the loader resolves
// names back through inv_registry_find().
//
// the in-memory cursor stack is intentionally NOT serialized: a half-dragged
// item shouldn't survive a save. it gets flushed into the bag first.

#define INV_SAVE_MAGIC    0x494E5630u   /* 'INV0' */
#define INV_SAVE_VERSION  1

// flush the cursor back into the bag so nothing is lost on save. called
// automatically by inv_save_to_buffer, exposed for the ui to call on close.
void inv_flush_cursor(inv_player *p);

// serialize into a freshly malloc'd buffer. *out_size gets the byte count.
// caller frees. returns NULL on allocation failure.
void *inv_save_to_buffer(inv_player *p, size_t *out_size);

// load from a buffer previously produced by inv_save_to_buffer. returns 0 on
// success, negative on a bad header / truncated / version mismatch. on
// failure the player is left initialized-but-empty rather than half-written.
int   inv_load_from_buffer(inv_player *p, const void *buf, size_t size);

// convenience wrappers around util/file.
int   inv_save_to_file(inv_player *p, const char *path);
int   inv_load_from_file(inv_player *p, const char *path);

#endif
