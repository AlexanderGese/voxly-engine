#ifndef INPUT_KEYBINDS_H
#define INPUT_KEYBINDS_H

// user-configurable keybinds. loaded from assets/keybinds.txt at startup,
// defaults used if missing.

typedef struct {
    int forward;
    int back;
    int left;
    int right;
    int jump;
    int sprint;
    int sneak;
    int fly;
    int debug;
    int menu;
    int inv_1, inv_2, inv_3, inv_4, inv_5, inv_6, inv_7, inv_8, inv_9;
} keybinds;

void keybinds_defaults(keybinds *k);
int  keybinds_load(keybinds *k, const char *path);
int  keybinds_save(const keybinds *k, const char *path);

#endif
