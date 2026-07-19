#ifndef RENDER_TONEMAP_H
#define RENDER_TONEMAP_H
#include "../gl.h"
#include "tonemap_programs.h"
#include "tonemap_params.h"
#include "tonemap_quad.h"
#include "tonemap_exposure.h"
#include "tonemap_lut.h"
#include "tonemap_target.h"
// public face of the hdr tonemap + color grade stage. this is the last thing
// that touches the scene before it (or fxaa, or the ui) hits the screen.
//
// usage:
// tonemap tm;
// tonemap_init(&tm, win_w, win_h);
// ... per frame, scene already rendered to an rgba16f scene_tex ...
// tonemap_feed_luma(&tm, measured_avg_luma);   // optional, drives auto-exp
// tonemap_tick(&tm, dt);                        // advance exposure adaption
// tonemap_run(&tm, scene_tex, /*dst fbo*/ 0, win_w, win_h);
// ... on resize ...
// tonemap_resize(&tm, w, h);
// tonemap_destroy(&tm);
typedef struct {
    tonemap_programs prog;
    tonemap_params   params;
    tonemap_quad     quad;
    tonemap_exposure exposure;
    tonemap_lut      lut;
    int  ready;          // 1 if the grade shader loaded
    int  w, h;
    unsigned frames;     // frames run, for debug
} tonemap;
// build everything. returns 1 on success; on failure the struct is still safe
// to destroy and run() becomes a no-op.
int  tonemap_init(tonemap *tm, int w, int h);
void tonemap_destroy(tonemap *tm);
void tonemap_resize(tonemap *tm, int w, int h);
void tonemap_feed_luma(tonemap *tm, float avg_luma);
void tonemap_tick(tonemap *tm, float dt);
void tonemap_run(tonemap *tm, glid scene_tex, glid dst, int dst_w, int dst_h);
int  tonemap_load_lut(tonemap *tm, const char *cube_path);
tonemap_params   *tonemap_get_params(tonemap *tm);
tonemap_exposure *tonemap_get_exposure(tonemap *tm);
#endif
