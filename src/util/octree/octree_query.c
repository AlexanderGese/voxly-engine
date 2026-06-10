#include "octree_query.h"
#include "../darray.h"
#include <math.h>
#include <float.h>
const octree_node *n = &t->pool.nodes[idx];
if (n->count == 0) return 0;
if (!aabb_intersects(n->bounds, region)) return 0;
