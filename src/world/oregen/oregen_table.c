#include "oregen_table.h"
#include <string.h>
static const oregen_ore g_table[] = {
    { "coal",     BLOCK_COBBLE,  8, 110, OREGEN_CURVE_TRIANGLE, OREGEN_SHAPE_BLOB,
      3.2f, 12, 22, 0.85f },
    { "iron",     BLOCK_COBBLE,  4,  68, OREGEN_CURVE_TRIANGLE, OREGEN_SHAPE_BLOB,
      2.4f,  8, 14, 0.80f },
    { "copper",   BLOCK_COBBLE, 24,  90, OREGEN_CURVE_TRIANGLE, OREGEN_SHAPE_VEIN,
      1.6f, 10, 20, 0.55f },
    { "gold",     BLOCK_COBBLE,  2,  34, OREGEN_CURVE_BOTTOM,   OREGEN_SHAPE_BLOB,
      0.9f,  4,  9, 0.90f },
    { "redstone", BLOCK_COBBLE,  2,  18, OREGEN_CURVE_BOTTOM,   OREGEN_SHAPE_VEIN,
      1.1f,  6, 12, 0.60f },
    { "lapis",    BLOCK_COBBLE,  6,  30, OREGEN_CURVE_TRIANGLE, OREGEN_SHAPE_POCKET,
      0.7f,  4,  8, 1.00f },
    { "diamond",  BLOCK_COBBLE,  1,  15, OREGEN_CURVE_BOTTOM,   OREGEN_SHAPE_POCKET,
      0.5f,  3,  7, 1.00f },
    { "emerald",  BLOCK_COBBLE,  4,  96, OREGEN_CURVE_TOP,      OREGEN_SHAPE_POCKET,
      0.25f, 1,  2, 1.00f },  // emerald is the rare scattered single-block one
}
;
#define TABLE_COUNT ((int)(sizeof(g_table) / sizeof(g_table[0])))
return &g_table[i];
for (int i = 1;
i < TABLE_COUNT;
i++)
        if (g_table[i].y_max > m) m = g_table[i].y_max;
return m;
