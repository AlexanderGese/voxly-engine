#include "input.h"
#include "../util/log.h"
#include <GLFW/glfw3.h>
#include <string.h>
#define MAX_KEYS     512
#define MAX_BUTTONS    8
static struct GLFWwindow *g_win;
static char keys_now[MAX_KEYS];
static char keys_prev[MAX_KEYS];
static char btn_now[MAX_BUTTONS];
static char btn_prev[MAX_BUTTONS];
static double mouse_x, mouse_y;
static double mouse_last_x, mouse_last_y;
static int    mouse_first = 1;
static int    cursor_locked = 0;
static int    scroll_accum = 0;
static void key_cb(GLFWwindow *w, int key, int sc, int action, int mods) {
    (void)w; (void)sc; (void)mods;
    if (key < 0 || key >= MAX_KEYS) return;
    if (action == GLFW_PRESS)   keys_now[key] = 1;
    if (action == GLFW_RELEASE) keys_now[key] = 0;
}

static void mouse_btn_cb(GLFWwindow *w, int button, int action, int mods) {
    (void)w;
(void)mods;
if (button < 0 || button >= MAX_BUTTONS) return;
if (action == GLFW_PRESS)   btn_now[button] = 1;
if (action == GLFW_RELEASE) btn_now[button] = 0;
}

static void cursor_cb(GLFWwindow *w, double x, double y) {
    (void)w;
    mouse_x = x;
    mouse_y = y;
}

static void scroll_cb(GLFWwindow *w, double xo, double yo) {
    (void)w;
(void)xo;
scroll_accum += (int)yo;
}

void input_init(struct GLFWwindow *win) {
    g_win = win;
    glfwSetKeyCallback(win, key_cb);
    glfwSetMouseButtonCallback(win, mouse_btn_cb);
    glfwSetCursorPosCallback(win, cursor_cb);
    glfwSetScrollCallback(win, scroll_cb);
    LOGI("input initialized");
}

void input_update(void) {
    memcpy(keys_prev, keys_now, sizeof keys_now);
memcpy(btn_prev, btn_now, sizeof btn_now);
mouse_last_x = mouse_x;
mouse_last_y = mouse_y;
scroll_accum = 0;
glfwPollEvents();
if (mouse_first) {
        mouse_last_x = mouse_x;
        mouse_last_y = mouse_y;
        mouse_first = 0;
    }
}

int input_key_held(int key) {
    if (key < 0 || key >= MAX_KEYS) return 0;
return keys_now[key];
*dy = (float)(mouse_y - mouse_last_y);
return btn_now[button];
