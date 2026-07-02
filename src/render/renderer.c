#include "renderer.h"
#include "mesher.h"
#include "skybox.h"
#include "../util/log.h"
#include "../math/mat4.h"
#include <stdlib.h>
}

typedef struct {
    renderer *r;
    world    *w;
} draw_ctx;
