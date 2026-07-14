#ifndef RENDER_SHADERMAN_SHADERMAN_PRESETS_H
#define RENDER_SHADERMAN_SHADERMAN_PRESETS_H

// the engine's stock shader set, registered in one go. before this the paths
// were scattered as string literals across renderer.c, skybox.c, water_render.c
// etc. now there's one table and everyone fetches a handle by enum.
//
// the handles live on the preset struct so callers do
// presets.block / presets.water instead of re-resolving by name each frame.

#include "shaderman.h"

typedef struct {
    shader_handle block;     // opaque terrain
    shader_handle water;     // translucent fluid pass
    shader_handle skybox;    // background cube
    shader_handle sun;       // celestial billboards
    shader_handle hud;       // 2d ortho ui
    shader_handle wireframe; // debug block outline / selection
    int           loaded;    // how many of the above actually built ok
} shader_presets;

// load every stock shader into `sm` and fill `out` with their handles. paths
// are the same ones the old renderer used so existing .vert/.frag files just
// work. returns the number that built successfully (the rest stay registered
// for hot-reload rescue).
int shaderman_load_presets(shaderman *sm, shader_presets *out);

// re-fetch handles for an already-populated manager (e.g. after a reload of the
// manager itself). idempotent; wont rebuild gl programs.
void shaderman_resolve_presets(shaderman *sm, shader_presets *out);

#endif
