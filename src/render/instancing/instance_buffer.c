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
}

void instancing_buffer_upload(instancing_instance_buffer *b,
                              const instancing_gpu_instance *recs, int count) {
    if (!b->initialised) {
        LOGW("instancing_buffer_upload: buffer not initialised");
        return;
    }
    b->count = count;
    if (count <= 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);

    if (count > b->capacity) {
        int newcap = b->capacity;
        while (newcap < count) newcap *= 2;
        glBufferData(GL_ARRAY_BUFFER,
                     (long)newcap * sizeof(instancing_gpu_instance),
                     NULL, GL_STREAM_DRAW);
        b->capacity = newcap;
    } else {
        // orphan: hand us a fresh block so the driver doesn't make us wait on
        // the prior frame's draw still reading the old one.
        glBufferData(GL_ARRAY_BUFFER,
                     (long)b->capacity * sizeof(instancing_gpu_instance),
                     NULL, GL_STREAM_DRAW);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (long)count * sizeof(instancing_gpu_instance), recs);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int instancing_buffer_setup_attribs(int base_location) {
    const GLsizei stride = sizeof(instancing_gpu_instance);
    int loc = base_location;

    // the model matrix occupies four consecutive vec4 attribute slots — gl
    // has no mat4 vertex attribute, you feed it as four columns. each gets
    // divisor 1 so it advances once per instance, not per vertex.
    for (int col = 0; col < 4; ++col) {
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride,
                              (void*)(offsetof(instancing_gpu_instance, model)
                                      + (size_t)col * 4 * sizeof(float)));
        glVertexAttribDivisor(loc, 1);
        loc++;
    }

    // tint (vec4)
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_gpu_instance, tint));
    glVertexAttribDivisor(loc, 1);
    loc++;

    // light (float)
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_gpu_instance, light));
    glVertexAttribDivisor(loc, 1);
    loc++;

    return loc;
}
