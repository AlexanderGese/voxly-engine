#include "lightprop_border.h"
#include "lightprop_propagate.h"
#include "lightprop_access.h"
#include "lightprop_step.h"
#include "../../util/log.h"
static lp_queue g_seam;
static int      g_seam_init;
static void seam_init(void) {
    if (!g_seam_init) { lp_queue_init(&g_seam); g_seam_init = 1; }
}

// enumerate the cells along chunk `c`'s face in direction `face`. fills wx/wz to
// the world coord of the cell ON OUR SIDE of the seam;
int tx, tz;
int run = face_strip(c, face, 0, &tx, &tz);
(void)tx;
(void)tz;
if (run == 0) return;
lp_queue_reset(&g_seam);
for (int i = 0;
i < run;
lp_border_stitch(w, c, 1);
lp_border_stitch(w, c, 4);
lp_border_stitch(w, c, 5);
