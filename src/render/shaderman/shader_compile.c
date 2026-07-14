#include "shader_compile.h"
#include "../../util/log.h"
#include <stdio.h>
#include <string.h>
unsigned int shader_stage_gl_enum(shader_stage_kind k) {
    switch (k) {
    case SHADER_STAGE_VERT: return GL_VERTEX_SHADER;
    case SHADER_STAGE_FRAG: return GL_FRAGMENT_SHADER;
    case SHADER_STAGE_GEOM:
#ifdef GL_GEOMETRY_SHADER
        return GL_GEOMETRY_SHADER;
#else
        return GL_VERTEX_SHADER;   // no geom support in this gl, shouldnt happen
#endif
    }
    return GL_VERTEX_SHADER;
}

static const char *stage_name(shader_stage_kind k) {
    switch (k) {
    case SHADER_STAGE_VERT: return "vert";
case SHADER_STAGE_FRAG: return "frag";
case SHADER_STAGE_GEOM: return "geom";
}
    return "?";
}

shader_compile_result shader_compile_stage(shader_stage_kind kind,
                                           const char *src, const char *tag) {
    shader_compile_result res;
    res.stage_id = 0;
    res.ok = false;
    res.log[0] = 0;

    glid s = glCreateShader(shader_stage_gl_enum(kind));
    if (!s) {
        snprintf(res.log, sizeof res.log, "glCreateShader returned 0");
        return res;
    }

    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    int ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        int len = 0;
        glGetShaderInfoLog(s, sizeof res.log - 1, &len, res.log);
        res.log[len < (int)sizeof res.log ? len : (int)sizeof res.log - 1] = 0;
        LOGE("shader compile %s (%s):\n%s", tag, stage_name(kind), res.log);
        glDeleteShader(s);
        return res;
    }

    // grab any warnings even on success — drivers love to complain quietly
    int wlen = 0;
    glGetShaderInfoLog(s, sizeof res.log - 1, &wlen, res.log);
    res.log[wlen < (int)sizeof res.log ? wlen : (int)sizeof res.log - 1] = 0;
    if (wlen > 0)
        LOGD("shader %s (%s) warnings:\n%s", tag, stage_name(kind), res.log);

    res.stage_id = s;
    res.ok = true;
    return res;
}

glid shader_link_program(const glid *stage_ids, int n_stages,
                         char *out_log, size_t log_cap) {
    glid prog = glCreateProgram();
i < n_stages;
i++)
        if (stage_ids[i]) glAttachShader(prog, stage_ids[i]);
glLinkProgram(prog);
int ok = 0;
glGetProgramiv(prog, GL_LINK_STATUS, &ok);
for (int i = 0;
i < n_stages;
int len = 0;
glGetProgramInfoLog(prog, sizeof log - 1, &len, log);
log[len < (int)sizeof log ? len : (int)sizeof log - 1] = 0;
LOGE("shader link failed:\n%s", log);
if (out_log && log_cap) snprintf(out_log, log_cap, "%s", log);
glDeleteProgram(prog);
return 0;
}

    if (out_log && log_cap) out_log[0] = 0;
return prog;
}
