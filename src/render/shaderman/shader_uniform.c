#include "shader_uniform.h"
#include "shader_source.h"     // shader_str_hash
#include "../../util/log.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

void shader_uniform_reset(shader_program *p) {
    p->uniform_count = 0;
    memset(p->uniforms, 0, sizeof p->uniforms);
}

shader_uniform_slot *shader_uniform_slot_for(shader_program *p, const char *name) {
    if (!p->prog) return NULL;
    uint32_t h = shader_str_hash(name);

    // linear probe over the small fixed table. hash gates the strcmp so most
    // misses are a single int compare.
    for (int i = 0; i < p->uniform_count; i++) {
        shader_uniform_slot *u = &p->uniforms[i];
        if (u->name_hash == h && strcmp(u->name, name) == 0)
            return u;
    }

    if (p->uniform_count >= SHADERMAN_MAX_UNIFORMS) {
        LOGW("shader '%s': uniform cache full, '%s' uncached", p->name, name);
        return NULL;
    }

    shader_uniform_slot *u = &p->uniforms[p->uniform_count++];
    snprintf(u->name, sizeof u->name, "%s", name);
    u->name_hash = h;
    u->location = glGetUniformLocation(p->prog, name);
    u->kind = SHADER_U_NONE;
    u->dirty = true;
    if (u->location < 0) {
        // not fatal — could be optimized out by the compiler. log once.
        LOGD("shader '%s': uniform '%s' not active", p->name, name);
    }
    return u;
}

// near-equality for the redundancy check. exact bit compare is too strict
// (lerped values jitter in the last bit) and too loose isnt safe, so: epsilon.
static bool feq(float a, float b) { return fabsf(a - b) <= 1e-7f; }

void shader_uniform_set_int(shader_program *p, const char *name, int v) {
    shader_uniform_slot *u = shader_uniform_slot_for(p, name);
    if (!u || u->location < 0) return;
    if (!u->dirty && u->kind == SHADER_U_INT && u->val.i == v) return;
    u->kind = SHADER_U_INT;
    u->val.i = v;
    u->dirty = false;
    glUniform1i(u->location, v);
}

void shader_uniform_set_float(shader_program *p, const char *name, float v) {
    shader_uniform_slot *u = shader_uniform_slot_for(p, name);
    if (!u || u->location < 0) return;
    if (!u->dirty && u->kind == SHADER_U_FLOAT && feq(u->val.f, v)) return;
    u->kind = SHADER_U_FLOAT;
    u->val.f = v;
    u->dirty = false;
    glUniform1f(u->location, v);
}

void shader_uniform_set_vec3(shader_program *p, const char *name, vec3 v) {
    shader_uniform_slot *u = shader_uniform_slot_for(p, name);
    if (!u || u->location < 0) return;
    if (!u->dirty && u->kind == SHADER_U_VEC3 &&
        feq(u->val.v3.x, v.x) && feq(u->val.v3.y, v.y) && feq(u->val.v3.z, v.z))
        return;
    u->kind = SHADER_U_VEC3;
    u->val.v3 = v;
    u->dirty = false;
    glUniform3f(u->location, v.x, v.y, v.z);
}

void shader_uniform_set_vec4(shader_program *p, const char *name, vec4 v) {
    shader_uniform_slot *u = shader_uniform_slot_for(p, name);
    if (!u || u->location < 0) return;
    if (!u->dirty && u->kind == SHADER_U_VEC4 &&
        feq(u->val.v4.x, v.x) && feq(u->val.v4.y, v.y) &&
        feq(u->val.v4.z, v.z) && feq(u->val.v4.w, v.w))
        return;
    u->kind = SHADER_U_VEC4;
    u->val.v4 = v;
    u->dirty = false;
    glUniform4f(u->location, v.x, v.y, v.z, v.w);
}

void shader_uniform_set_mat4(shader_program *p, const char *name, const mat4 *m) {
    shader_uniform_slot *u = shader_uniform_slot_for(p, name);
    if (!u || u->location < 0) return;

    const float *src = mat4_data(m);
    if (!u->dirty && u->kind == SHADER_U_MAT4) {
        // 16 floats — compare before deciding to upload. still cheaper than the
        // driver round trip for a redundant matrix.
        bool same = true;
        for (int i = 0; i < 16; i++)
            if (!feq(u->val.m16[i], src[i])) { same = false; break; }
        if (same) return;
    }
    u->kind = SHADER_U_MAT4;
    memcpy(u->val.m16, src, sizeof u->val.m16);
    u->dirty = false;
    glUniformMatrix4fv(u->location, 1, GL_FALSE, src);
}

void shader_uniform_mark_all_dirty(shader_program *p) {
    for (int i = 0; i < p->uniform_count; i++)
        p->uniforms[i].dirty = true;
}
