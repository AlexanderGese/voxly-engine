#include "instance_data.h"

#include "../../util/darray.h"

void instancing_data_init(instancing_instance_data *d) {
    d->items = NULL;
    d->freelist = NULL;
    d->live = 0;
}

void instancing_data_free(instancing_instance_data *d) {
    darr_free(d->items);
    darr_free(d->freelist);
    d->live = 0;
}

int instancing_data_add(instancing_instance_data *d,
                        const instancing_instance *it) {
    int handle;
    if (!darr_empty(d->freelist)) {
        // recycle a freed slot. cheaper than growing and keeps handles small.
        handle = darr_pop(d->freelist);
        d->items[handle] = *it;
    } else {
        handle = (int)darr_len(d->items);
        darr_push(d->items, *it);
    }

    instancing_instance *rec = &d->items[handle];
    rec->flags |= INSTANCING_FLAG_ACTIVE | INSTANCING_FLAG_DIRTY;
    d->live++;
    return handle;
}

void instancing_data_remove(instancing_instance_data *d, int handle) {
    if (handle < 0 || handle >= (int)darr_len(d->items)) return;
    instancing_instance *rec = &d->items[handle];
    if (!(rec->flags & INSTANCING_FLAG_ACTIVE)) return;  // already dead

    rec->flags = 0;   // clears ACTIVE; the slot is now inert
    darr_push(d->freelist, handle);
    if (d->live > 0) d->live--;
}

instancing_instance *instancing_data_get(instancing_instance_data *d,
                                         int handle) {
    if (handle < 0 || handle >= (int)darr_len(d->items)) return NULL;
    instancing_instance *rec = &d->items[handle];
    if (!(rec->flags & INSTANCING_FLAG_ACTIVE)) return NULL;
    return rec;
}

int instancing_data_capacity(const instancing_instance_data *d) {
    return (int)darr_len(d->items);
}

int instancing_data_live(const instancing_instance_data *d) {
    return d->live;
}
