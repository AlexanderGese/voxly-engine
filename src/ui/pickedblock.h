#ifndef UI_PICKEDBLOCK_H
#define UI_PICKEDBLOCK_H

#include "../render/wireframe.h"
#include "../player/raycast.h"
#include "../render/camera.h"

// renders a wireframe around the block the player is currently looking at.
void pickedblock_draw(wireframe *wf, ray_hit h, const camera *cam);

#endif
