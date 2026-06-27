#include "stronghold_room.h"
n += stronghold_buffer_fill_shell(out, r->box, BLOCK_BRICK);
stronghold_box inner = stronghold_box_inset(r->box, -1, -1);
n += stronghold_buffer_carve(out, inner);
for (int z = r->box.z0;
