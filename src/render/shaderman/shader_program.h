#ifndef RENDER_SHADERMAN_SHADER_PROGRAM_H
#define RENDER_SHADERMAN_SHADER_PROGRAM_H

// build / rebuild a single managed program from its stage paths. this is the
// glue between source loading, compiling, linking and the uniform cache. the
// manager owns the shader_program structs; this file just operates on one.

#include "shaderman_types.h"

// set up an empty program slot with a name + stage paths. geom_path may be
// NULL. does not touch gl — call shader_program_build for that.
void shader_program_config(shader_program *p, const char *name,
                           const char *vert_path, const char *frag_path,
                           const char *geom_path);

// (re)compile + link from the configured stage paths. on success swaps in the
// new gl program (deleting the old one) and resets the uniform cache. on
// failure the OLD program is kept live so a typo while hot-reloading doesnt
// black-screen the game — it just logs and you fix the file. returns ok.
bool shader_program_build(shader_program *p);

// refresh the stored mtimes from disk for every stage + its include deps,
// without rebuilding. used after a successful build so the watcher has a clean
// baseline. returns the number of stage files it could stat.
int  shader_program_restamp(shader_program *p);

// free the gl program and clear the slot.
void shader_program_destroy(shader_program *p);

#endif
