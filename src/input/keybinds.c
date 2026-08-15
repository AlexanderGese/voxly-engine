#include "keybinds.h"
#include "../util/log.h"
#include "../util/file.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void keybinds_defaults(keybinds *k) {
    k->forward = GLFW_KEY_W;
    k->back    = GLFW_KEY_S;
    k->left    = GLFW_KEY_A;
    k->right   = GLFW_KEY_D;
    k->jump    = GLFW_KEY_SPACE;
    k->sprint  = GLFW_KEY_LEFT_CONTROL;
    k->sneak   = GLFW_KEY_LEFT_SHIFT;
    k->fly     = GLFW_KEY_F;
    k->debug   = GLFW_KEY_F3;
    k->menu    = GLFW_KEY_ESCAPE;
    k->inv_1 = GLFW_KEY_1;
    k->inv_2 = GLFW_KEY_2;
    k->inv_3 = GLFW_KEY_3;
    k->inv_4 = GLFW_KEY_4;
    k->inv_5 = GLFW_KEY_5;
    k->inv_6 = GLFW_KEY_6;
    k->inv_7 = GLFW_KEY_7;
    k->inv_8 = GLFW_KEY_8;
    k->inv_9 = GLFW_KEY_9;
}

// super basic "key=value" parser
int keybinds_load(keybinds *k, const char *path) {
    keybinds_defaults(k);
    size_t sz;
    char *txt = file_read_all(path, &sz);
    if (!txt) return 0;

    char *p = txt;
    while (*p) {
        char *line = p;
        while (*p && *p != '\n') p++;
        if (*p == '\n') { *p = 0; p++; }

        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        int  val = atoi(eq + 1);

        if      (!strcmp(key, "forward")) k->forward = val;
        else if (!strcmp(key, "back"))    k->back    = val;
        else if (!strcmp(key, "left"))    k->left    = val;
        else if (!strcmp(key, "right"))   k->right   = val;
        else if (!strcmp(key, "jump"))    k->jump    = val;
        else if (!strcmp(key, "sprint"))  k->sprint  = val;
        else if (!strcmp(key, "sneak"))   k->sneak   = val;
        else if (!strcmp(key, "fly"))     k->fly     = val;
        // TODO: rest of them. kinda boring

    }
    free(txt);
    LOGI("keybinds loaded from %s", path);
    return 1;
}

int keybinds_save(const keybinds *k, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "forward=%d\nback=%d\nleft=%d\nright=%d\n", k->forward, k->back, k->left, k->right);
    fprintf(f, "jump=%d\nsprint=%d\nsneak=%d\nfly=%d\n", k->jump, k->sprint, k->sneak, k->fly);
    fclose(f);
    return 0;
}
