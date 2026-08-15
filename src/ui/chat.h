#ifndef UI_CHAT_H
#define UI_CHAT_H
#include "../render/text.h"
// chat / event log. not a real chat, just a scrolling message buffer.
#define CHAT_MAX_LINES 32
#define CHAT_LINE_LEN   96
typedef struct {
    char   lines[CHAT_MAX_LINES][CHAT_LINE_LEN];
    float  ages[CHAT_MAX_LINES];
    int    head;
    int    count;
} chatlog;
void chat_init(chatlog *c);
void chat_push(chatlog *c, const char *msg);
void chat_pushf(chatlog *c, const char *fmt, ...);
void chat_update(chatlog *c, float dt);
void chat_draw(const chatlog *c, text_renderer *t, int sw, int sh);
#endif
