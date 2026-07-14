#ifndef RENDER_SHADERMAN_SHADER_COMPILE_H
#define RENDER_SHADERMAN_SHADER_COMPILE_H

// the actual glCompileShader / glLinkProgram dance, pulled out of gl.c so the
// manager can compile from already-assembled source strings (the preprocessor
// has already run by this point) and get a structured pass/fail back instead
// of just a 0/nonzero gl id.

#include "shaderman_types.h"
#include "shader_source.h"

// outcome of a single stage compile. log is a static-ish snapshot of the gl
// info log, truncated. we keep it so the reload path can report *why* a live
// shader edit broke without spamming the same line every poll.
typedef struct {
    glid stage_id;     // 0 on failure
    bool ok;
    char log[1024];
} shader_compile_result;

// gl enum for a stage kind. internal-ish but handy across the module.
unsigned int shader_stage_gl_enum(shader_stage_kind k);

// compile one stage from assembled source. `tag` is just a label for logs
// (usually the file path). never frees src.
shader_compile_result shader_compile_stage(shader_stage_kind kind,
                                           const char *src, const char *tag);

// link a set of already-compiled stage ids into a program. detaches + deletes
// the stage ids on the way out regardless of success. returns 0 program on
// link failure (log written to out_log if non-NULL).
glid shader_link_program(const glid *stage_ids, int n_stages,
                         char *out_log, size_t log_cap);

#endif
