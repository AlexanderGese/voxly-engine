#include "instance_buffer.h"
#include "../../util/log.h"
#include <stddef.h>
void instancing_buffer_init(instancing_instance_buffer *b, int initial) {
    if (initial < 16) initial = 16;

    glGenBuffers(1, &b->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (long)initial * sizeof(instancing_gpu_instance),
                 NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    b->capacity    = initial;
    b->count       = 0;
    b->initialised = 1;
}

void instancing_buffer_destroy(instancing_instance_buffer *b) {
    if (b->vbo) glDeleteBuffers(1, &b->vbo);
b->vbo = 0;
b->capacity = 0;
b->count = 0;
b->initialised = 0;
int loc = base_location;
for (int col = 0;
col < 4;
glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_gpu_instance, tint));
glVertexAttribDivisor(loc, 1);
loc++;
glEnableVertexAttribArray(loc);
glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_gpu_instance, light));
glVertexAttribDivisor(loc, 1);
loc++;
return loc;
}
