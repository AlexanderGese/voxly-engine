#ifndef RENDER_SHADERMAN_SHADER_SOURCE_H
#define RENDER_SHADERMAN_SHADER_SOURCE_H

// shader source loading + a tiny preprocessor. glsl has no #include so we roll
// our own: lines of the form  #include "common.glsl"  get spliced in. this
// keeps the lighting + fog helpers in one file shared by block/water/skybox
// instead of being copy-pasted (which is what i did for the first two weeks).

#include "shaderman_types.h"
#include <stddef.h>
#include <stdint.h>

// result of resolving a stage's source. text is a single malloc'd buffer the
// caller frees. dep_paths/dep_count list every file that fed into it (the main
// file plus all transitive includes) so the reload watcher knows what to stat.
typedef struct {
    char    *text;
    size_t   len;
    char     dep_paths[SHADERMAN_MAX_INCLUDES][SHADERMAN_PATH_LEN];
    int      dep_count;
    bool     ok;
} shader_source;

// read `path`, recursively splice #include directives, return assembled source.
// includes are resolved relative to the including file's directory. on failure
// ok is false and text is NULL. caller frees with shader_source_free.
shader_source shader_source_load(const char *path);

void shader_source_free(shader_source *s);

// fnv-1a over a nul-terminated string. used for uniform name hashing too.
uint32_t shader_str_hash(const char *s);

// mtime in whole seconds, or 0 if the file cant be stat'd.
int64_t  shader_file_mtime(const char *path);

#endif
