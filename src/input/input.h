#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include "../math/vec3.h"

struct GLFWwindow;

void input_init(struct GLFWwindow *win);
void input_update(void);

// keyboard
int  input_key_held(int key);
int  input_key_pressed(int key);   // edge

// mouse
void  input_mouse_delta(float *dx, float *dy);
int   input_mouse_pressed(int button);
int   input_mouse_held(int button);
void  input_set_cursor_locked(int locked);
int   input_cursor_locked(void);

// hotbar scroll
int   input_scroll_delta(void);

#endif
