#include "shader_uniform.h"
#include "shader_source.h"     // shader_str_hash
#include "../../util/log.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
uint32_t h = shader_str_hash(name);
for (int i = 0;
i < p->uniform_count;
return NULL;
}

    shader_uniform_slot *u = &p->uniforms[p->uniform_count++];
snprintf(u->name, sizeof u->name, "%s", name);
u->name_hash = h;
u->location = glGetUniformLocation(p->prog, name);
u->kind = SHADER_U_NONE;
u->dirty = true;
if (!u || u->location < 0) return;
if (!u->dirty && u->kind == SHADER_U_INT && u->val.i == v) return;
u->kind = SHADER_U_INT;
