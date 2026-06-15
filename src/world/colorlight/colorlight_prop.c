#include "colorlight_prop.h"
#include "colorlight_access.h"
#include "colorlight_queue.h"
#include "colorlight_emitter.h"
#include "../block.h"
#include "../../config.h"
;
;
;
if (colorlight_world_get_chan(w, wx, wy, wz, chan) >= level) return;
colorlight_world_set_chan(w, wx, wy, wz, chan, level);
colorlight_queue_push(colorlight_queue_add(), wx, wy, wz, (uint8_t)chan, level);
seed_channel(w, wx, wy, wz, 0, colorlight_packed_r(seed));
seed_channel(w, wx, wy, wz, 1, colorlight_packed_g(seed));
seed_channel(w, wx, wy, wz, 2, colorlight_packed_b(seed));
colorlight_prop_drain_add(w);
