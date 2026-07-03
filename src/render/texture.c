#include "texture.h"
#include "../util/log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int texture_load(texture *t, const char *path) {
    int w, h, comp;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &w, &h, &comp, 4);
    if (!data) {
        LOGE("texture_load failed: %s", path);
        return 0;
    }

    glGenTextures(1, &t->id);
    glBindTexture(GL_TEXTURE_2D, t->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    t->w = w;
    t->h = h;
    LOGI("texture %s %dx%d id=%u", path, w, h, t->id);
    return 1;
}

void texture_bind(const texture *t, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, t->id);
}

void texture_destroy(texture *t) {
    if (t->id) glDeleteTextures(1, &t->id);
    t->id = 0;
}
