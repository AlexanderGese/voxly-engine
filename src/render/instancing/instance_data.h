#ifndef RENDER_INSTANCING_INSTANCE_DATA_H
#define RENDER_INSTANCING_INSTANCE_DATA_H

// the cpu-side scene store: a flat growable list of instancing_instance
// records plus a free-list of recycled slots. callers add/remove props and we
// hand back stable handles (indices). the batch builder walks this list every
// frame, so keeping it dense-ish matters; we compact lazily on remove by
// reusing freed slots rather than shifting.

#include "instancing_types.h"

typedef struct {
    instancing_instance *items;   // darray of records (stb-style)
    int                 *freelist;// darray of free indices
    int                  live;    // count of active (non-free) records
} instancing_instance_data;

void instancing_data_init(instancing_instance_data *d);
void instancing_data_free(instancing_instance_data *d);

// add a record. returns a stable handle (index into items). the record is
// copied; ACTIVE flag is forced on. mesh_id must be valid for it to draw.
int  instancing_data_add(instancing_instance_data *d,
                         const instancing_instance *it);

// remove by handle. slot goes on the freelist for reuse. safe on a bad/dead
// handle (no-op).
void instancing_data_remove(instancing_instance_data *d, int handle);

// fetch a mutable pointer, or NULL if the handle is dead/out of range. handy
// for moving an instance: edit in place then mark DIRTY.
instancing_instance *instancing_data_get(instancing_instance_data *d,
                                         int handle);

// total slots (including freed). iterate 0..capacity and skip !ACTIVE.
int  instancing_data_capacity(const instancing_instance_data *d);
int  instancing_data_live(const instancing_instance_data *d);

#endif
