#ifndef RENDER_SHADERMAN_SHADER_DEBUG_H
#define RENDER_SHADERMAN_SHADER_DEBUG_H
// introspection helpers for the shader manager. these feed the F3 debug overlay
// and the `shaders` console command. nothing here is on a hot path — its all
// querying state for humans to look at.
#include "shaderman.h"
// a flat snapshot of one program's state for the overlay. no pointers into the
// manager so the overlay can keep it around for a frame without aliasing.
typedef struct {
    char     name[SHADERMAN_NAME_LEN];
    glid     gl_id;
    bool     ok;
    int      stage_count;
    int      uniform_count;
    int      active_uniforms;     // ones with a real location
    uint32_t reload_gen;
} shader_info;
// fill `out` (caller array of `cap`) with one shader_info per in-use program.
// returns the number written.
int  shaderman_collect_info(shaderman *sm, shader_info *out, int cap);
// human-readable dump of every program + its cached uniforms to the log. bound
// to a console command. verbose on purpose.
void shaderman_dump(shaderman *sm);
// total live gl programs the manager owns. for the overlay line.
int  shaderman_live_count(shaderman *sm);
int  shaderman_gl_active_uniforms(shaderman *sm, shader_handle h);
const char *shaderman_status_line(shaderman *sm, char *buf, int cap);
#endif
