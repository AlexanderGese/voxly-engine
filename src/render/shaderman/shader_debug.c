#include "shader_debug.h"
#include "../../util/log.h"

#include <string.h>
#include <stdio.h>

static int count_stages(const shader_program *p) {
    int n = 0;
    for (int k = 0; k < SHADERMAN_MAX_STAGES; k++)
        if (p->stages[k].present) n++;
    return n;
}

static int count_active(const shader_program *p) {
    int n = 0;
    for (int i = 0; i < p->uniform_count; i++)
        if (p->uniforms[i].location >= 0) n++;
    return n;
}

int shaderman_collect_info(shaderman *sm, shader_info *out, int cap) {
    int n = 0;
    for (int i = 0; i < sm->program_count && n < cap; i++) {
        shader_program *p = &sm->programs[i];
        if (!p->in_use) continue;

        shader_info *si = &out[n++];
        snprintf(si->name, sizeof si->name, "%s", p->name);
        si->gl_id          = p->prog;
        si->ok             = p->ok;
        si->stage_count    = count_stages(p);
        si->uniform_count  = p->uniform_count;
        si->active_uniforms = count_active(p);
        si->reload_gen     = p->reload_gen;
    }
    return n;
}

static const char *kind_name(shader_uniform_kind k) {
    switch (k) {
    case SHADER_U_NONE:  return "none";
    case SHADER_U_INT:   return "int";
    case SHADER_U_FLOAT: return "float";
    case SHADER_U_VEC3:  return "vec3";
    case SHADER_U_VEC4:  return "vec4";
    case SHADER_U_MAT4:  return "mat4";
    }
    return "?";
}

void shaderman_dump(shaderman *sm) {
    LOGI("=== shaderman dump (%d slots) ===", sm->program_count);
    for (int i = 0; i < sm->program_count; i++) {
        shader_program *p = &sm->programs[i];
        if (!p->in_use) continue;

        LOGI("[%d] %-16s gl=%u %s gen=%u stages=%d uniforms=%d/%d",
             i, p->name, p->prog, p->ok ? "OK" : "BROKEN",
             p->reload_gen, count_stages(p),
             count_active(p), p->uniform_count);

        for (int k = 0; k < SHADERMAN_MAX_STAGES; k++) {
            if (!p->stages[k].present) continue;
            const char *sn = (k == SHADER_STAGE_VERT) ? "vert" :
                             (k == SHADER_STAGE_FRAG) ? "frag" : "geom";
            LOGI("    %s: %s", sn, p->stages[k].path);
        }

        for (int u = 0; u < p->uniform_count; u++) {
            shader_uniform_slot *us = &p->uniforms[u];
            LOGI("    u %-20s loc=%-3d %-5s %s",
                 us->name, us->location, kind_name(us->kind),
                 us->dirty ? "(dirty)" : "");
        }
    }
    LOGI("=== %d failures total, %u reloads ===",
         sm->build_failures, sm->watcher.reload_count);
}

int shaderman_live_count(shaderman *sm) {
    int n = 0;
    for (int i = 0; i < sm->program_count; i++)
        if (sm->programs[i].in_use && sm->programs[i].prog) n++;
    return n;
}

int shaderman_gl_active_uniforms(shaderman *sm, shader_handle h) {
    shader_program *p = shaderman_get(sm, h);
    if (!p || !p->prog) return 0;
    int count = 0;
    glGetProgramiv(p->prog, GL_ACTIVE_UNIFORMS, &count);
    return count;
}

const char *shaderman_status_line(shaderman *sm, char *buf, int cap) {
    int ok = 0, broken = 0;
    for (int i = 0; i < sm->program_count; i++) {
        if (!sm->programs[i].in_use) continue;
        if (sm->programs[i].ok) ok++;
        else broken++;
    }
    snprintf(buf, cap, "shaders: %d ok, %d broken, %u reloads",
             ok, broken, sm->watcher.reload_count);
    return buf;
}
