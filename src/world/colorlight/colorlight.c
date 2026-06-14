#include "colorlight.h"
#include "colorlight_access.h"
#include "colorlight_grid.h"
#include "colorlight_queue.h"
#include "colorlight_emitter.h"
#include "colorlight_prop.h"
#include "colorlight_blend.h"
#include "../block.h"
#include "../../config.h"
#include "../../util/log.h"
block_id id = world_get_block(w, wx, wy, wz);
colorlight_prop_place(w, wx, wy, wz, colorlight_emitter_color(id));
return;
}

    // a transparent non-emitter (air, glass, water...) appeared where a wall
    // was. light should flow into the gap: pull whatever's there, then relight
    // from the brightest neighbor on each channel.
    colorlight_prop_remove(w, wx, wy, wz);
;
;
