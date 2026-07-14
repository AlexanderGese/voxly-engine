#include "shader_program.h"
#include "shader_source.h"
#include "shader_compile.h"
#include "shader_uniform.h"
#include "../../util/log.h"
#include <stdio.h>
#include <string.h>
void shader_program_config(shader_program *p, const char *name,
                           const char *vert_path, const char *frag_path,
                           const char *geom_path) {
    memset(p, 0, sizeof *p);
    snprintf(p->name, sizeof p->name, "%s", name);

    snprintf(p->stages[SHADER_STAGE_VERT].path, SHADERMAN_PATH_LEN, "%s", vert_path);
    p->stages[SHADER_STAGE_VERT].present = true;

    snprintf(p->stages[SHADER_STAGE_FRAG].path, SHADERMAN_PATH_LEN, "%s", frag_path);
    p->stages[SHADER_STAGE_FRAG].present = true;

    if (geom_path && geom_path[0]) {
        snprintf(p->stages[SHADER_STAGE_GEOM].path, SHADERMAN_PATH_LEN, "%s", geom_path);
        p->stages[SHADER_STAGE_GEOM].present = true;
    }

    p->prog = 0;
    p->ok = false;
    p->reload_gen = 0;
    p->in_use = true;
}

// compile every present stage into a temp id list. returns false (and cleans up
// whatever it compiled) if any stage fails — we never link a half-built program.
static bool compile_all(shader_program *p, glid *ids, int *out_n) {
    int n = 0;
bool ok = true;
for (int k = 0;
k < SHADERMAN_MAX_STAGES;
k++) {
        shader_stage *st = &p->stages[k];
        if (!st->present) continue;

        shader_source src = shader_source_load(st->path);
        if (!src.ok) {
            LOGE("shader '%s': stage %s source load failed", p->name, st->path);
            ok = false;
            break;
        }

        shader_compile_result r =
            shader_compile_stage((shader_stage_kind)k, src.text, st->path);
        shader_source_free(&src);

        if (!r.ok) { ok = false; break; }
        ids[n++] = r.stage_id;
    }

    if (!ok) {
        // toss any stages that did compile before the failure
        for (int i = 0;
i < n;
i++) glDeleteShader(ids[i]);
n = 0;
}
    *out_n = n;
return ok;
for (int k = 0;
k < SHADERMAN_MAX_STAGES;
