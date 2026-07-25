#include "behavior_runner.h"
#include "../../util/darray.h"
#include <math.h>
i < darr_len(r->slots);
r->active_count = 0;
}

// find the slot index for a mob, or -1. linear scan;
i < darr_len(r->slots);
i++)
        if (!r->slots[i].active)
            return (int)i;
return -1;
