#ifndef WORLD_REGION_H
#define WORLD_REGION_H

#include "../chunk.h"

// top-level region save api. this is the thing the rest of the engine calls;
// everything under region/ is plumbing. it owns a single open-file cache keyed
// on a save directory, so you point it at a world dir once and then just
// load/save chunks by their (cx,cz).
//
// drop-in-ish replacement for the old one-file-per-chunk save.c. the loader
// path tries region first and you can keep the legacy reader as a fallback for
// old worlds during migration.

// point the region system at a save dir (e.g. "saves/world0"). makes the dir if
// missing. safe to call again to switch worlds; flushes the previous one.
void region_set_dir(const char *dir);

// flush + close everything. call on world unload / shutdown.
void region_shutdown(void);

// load chunk c from its region file. returns 1 if loaded, 0 if not present,
// <0 on error. fills c->blocks (+ light when present), sets generated.
int  region_load_chunk(chunk *c);

// persist chunk c. no-op if c->saved is already set. clears nothing on the
// chunk except c->saved. returns 0 on success.
int  region_save_chunk(chunk *c);

// force-save ignoring c->saved (used by the "save all" path).
int  region_save_chunk_force(chunk *c);

// remove a chunk from disk entirely.
int  region_drop_chunk(int cx, int cz);

// has this chunk ever been written?
int  region_chunk_exists(int cx, int cz);

// flush headers of any open region files without closing them. cheap, call it
// periodically so a crash doesnt lose the location table.
void region_checkpoint(void);

#endif
