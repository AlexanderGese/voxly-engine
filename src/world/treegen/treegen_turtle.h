#ifndef WORLD_TREEGEN_TURTLE_H
#define WORLD_TREEGEN_TURTLE_H

#include "../../math/vec3.h"
#include "treegen_buffer.h"

// 3d turtle used to interpret an l-system string into voxels. classic prusinkiewicz
// setup: an orthonormal frame (heading/left/up) plus a position, and a stack so
// '[' / ']' can branch. rotations are done on the frame directly with rodrigues;
// no mat4 needed and it stays numerically tidy after a re-orthonormalize.

typedef struct {
    vec3 pos;       // turtle tip, plant-local, float so partial steps accumulate
    vec3 head;      // forward (grows toward this)
    vec3 left;      // local +x
    vec3 up;        // local +y of the turtle, not the world
    float length;   // current segment length (shrinks with '!')
    float radius;   // current wood radius as a float, floored on stamp
} treegen_turtle;

#define TREEGEN_STACK_MAX  48

// the turtle plus its push/pop stack. one of these per plant.
typedef struct {
    treegen_turtle cur;
    treegen_turtle stack[TREEGEN_STACK_MAX];
    int sp;
} treegen_turtle_state;

// stand the turtle at the origin pointing straight up.
void treegen_turtle_init(treegen_turtle_state *t, float length, float radius);

int  treegen_turtle_push(treegen_turtle_state *t);   // 1 ok, 0 stack full
int  treegen_turtle_pop(treegen_turtle_state *t);    // 1 ok, 0 stack empty

// frame rotations, degrees. yaw about up, pitch about left, roll about head.
void treegen_turtle_yaw(treegen_turtle_state *t, float deg);
void treegen_turtle_pitch(treegen_turtle_state *t, float deg);
void treegen_turtle_roll(treegen_turtle_state *t, float deg);

// step forward by the current length along head. if id != AIR, lay a wood line
// from the old tip to the new one (so steep steps dont leave gaps). returns the
// integer cell the tip landed in via out_cell (may be NULL).
void treegen_turtle_forward(treegen_turtle_state *t, treegen_buffer *out,
                            block_id id, int *ox, int *oy, int *oz);

#endif
