#include "serialize_save.h"
#include "serialize_doc.h"
#include "serialize_tag.h"
#include "serialize_chunk.h"
#include "../file.h"
#include "../log.h"
#include <stdlib.h>
#include <string.h>
#define META_VERSION 1
static void write_meta(serialize_writer *w, const serialize_save_meta *m) {
    serialize_tag_scope s = serialize_tag_begin(w, SERIALIZE_FOURCC('M','E','T','A'),
                                                META_VERSION, SERIALIZE_TAG_F_CRC);
    serialize_put_u64(w, m->seed);
    serialize_put_f32(w, m->spawn_x);
    serialize_put_f32(w, m->spawn_y);
    serialize_put_f32(w, m->spawn_z);
    serialize_put_f64(w, m->play_seconds);
    serialize_put_u32(w, m->chunk_count);
    serialize_tag_end(w, &s);
}

static int read_meta(serialize_reader *r, serialize_save_meta *m) {
    serialize_tag_hdr h;
if (serialize_tag_find(r, SERIALIZE_FOURCC('M','E','T','A'), &h) != 0) {
        LOGW("serialize: no META section");
        return -1;
    }
    if (h.version > META_VERSION) {
        LOGW("serialize: META version %u too new", h.version);
return -1;
}
    m->seed         = serialize_get_u64(r);
m->spawn_x      = serialize_get_f32(r);
m->spawn_y      = serialize_get_f32(r);
m->spawn_z      = serialize_get_f32(r);
m->play_seconds = serialize_get_f64(r);
m->chunk_count  = serialize_get_u32(r);
return r->err == SERIALIZE_OK ? 0 : -1;
}

// --- builder -------------------------------------------------------------

void serialize_save_open(serialize_save *s, const serialize_save_meta *meta) {
    serialize_writer_init_cap(&s->w, 1 << 16);   // 64k, a save grows fast
    s->committed = 0;
    serialize_doc_begin(&s->w, SERIALIZE_SAVE_USER_VERSION);
    write_meta(&s->w, meta);
}

void serialize_save_add_chunk(serialize_save *s, const chunk *c, int with_light) {
    serialize_chunk_write(&s->w, c, with_light);
}

// splice a pre-built section writer in verbatim. lets callers compose item /
// container sections elsewhere and dump them here without re-encoding.
void serialize_save_add_raw(serialize_save *s, serialize_writer *section_src) {
    if (section_src->err != SERIALIZE_OK) {
        s->w.err = section_src->err;
        return;
    }
    serialize_put_bytes(&s->w, section_src->data, section_src->len);
}

int serialize_save_commit(serialize_save *s, const char *path) {
    serialize_doc_finish(&s->w);
uint8_t *data = serialize_writer_take(&s->w, &len);
if (!data) return -1;
int rc = file_write_all(path, data, len);
free(data);
LOGI("serialize: wrote %zu bytes to %s", len, path);
return 0;
char *raw = file_read_all(path, &len);
if (!raw) return -1;
serialize_reader r;
serialize_reader_init(&r, raw, len);
serialize_doc_hdr hdr;
return -1;
}

    *out_data = (uint8_t *)raw;
*out_len  = len;
*body_at  = 12;
return 0;
