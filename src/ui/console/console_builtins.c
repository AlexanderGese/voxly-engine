#include "console_builtins.h"
#include "console_command.h"
#include "console_cvar.h"

#include "../../config.h"
#include "../../world/world.h"
#include "../../world/block.h"
#include "../../player/player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the console_t holds its game hooks as forward-declared struct pointers
// to keep the header light. the real engine types are anonymous-typedef
// structs, so we launder them back through void* right here. ugly but it
// keeps console core ignorant of player.h dragging the whole GL stack in.
static world  *cw(console_t *c)  { return (world  *)(void *)c->world;  }
static player *cp(console_t *c)  { return (player *)(void *)c->player; }

// ---- small shared helpers -------------------------------------------------

static int need_world(console_t *c) {
    if (!cw(c)) { console_print(c, CONSOLE_SEV_ERROR, "no world loaded"); return 0; }
    return 1;
}
static int need_player(console_t *c) {
    if (!cp(c)) { console_print(c, CONSOLE_SEV_ERROR, "no player"); return 0; }
    return 1;
}

// parse a float arg, complaining on garbage. returns 0 ok.
static int arg_float(console_t *c, const char *s, float *out) {
    char *end = NULL;
    float v = strtof(s, &end);
    if (end == s || *end != 0) {
        console_printf(c, CONSOLE_SEV_ERROR, "not a number: '%s'", s);
        return -1;
    }
    *out = v;
    return 0;
}

// ---- commands -------------------------------------------------------------

static console_cmd_result cmd_help(console_t *c, const console_args *a) {
    if (a->argc >= 2) {
        // help <cmd>: detail one command.
        console_command *cmd = console_cmd_find(&c->cmds, a->argv[1]);
        if (!cmd) {
            console_printf(c, CONSOLE_SEV_ERROR, "no such command '%s'", a->argv[1]);
            return CONSOLE_CMD_ERR;
        }
        console_printf(c, CONSOLE_SEV_INFO, "%s -- %s", cmd->name,
                       cmd->help ? cmd->help : "(no help)");
        if (cmd->usage)
            console_printf(c, CONSOLE_SEV_INFO, "  usage: %s", cmd->usage);
        return CONSOLE_CMD_OK;
    }
    // bare help: list every command on packed rows.
    console_print(c, CONSOLE_SEV_INFO, "commands:");
    char row[CONSOLE_TEXT_LEN];
    int rl = 0; row[0] = 0;
    for (int i = 0; i < c->cmds.count; i++) {
        int need = (int)strlen(c->cmds.cmds[i].name) + 2;
        if (rl + need >= CONSOLE_TEXT_LEN - 1) {
            console_print(c, CONSOLE_SEV_INFO, row);
            rl = 0; row[0] = 0;
        }
        rl += snprintf(row + rl, sizeof row - rl, "%s  ", c->cmds.cmds[i].name);
    }
    if (rl) console_print(c, CONSOLE_SEV_INFO, row);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_clear(console_t *c, const console_args *a) {
    (void)a;
    console_buffer_clear(&c->buf);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_echo(console_t *c, const console_args *a) {
    // glue the args back together with single spaces.
    char line[CONSOLE_TEXT_LEN];
    int n = 0; line[0] = 0;
    for (int i = 1; i < a->argc; i++) {
        n += snprintf(line + n, sizeof line - n, "%s%s",
                      i > 1 ? " " : "", a->argv[i]);
        if (n >= (int)sizeof line - 1) break;
    }
    console_print(c, CONSOLE_SEV_INFO, line);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_history(console_t *c, const console_args *a) {
    (void)a;
    int n = console_history_count(&c->hist);
    if (n == 0) { console_print(c, CONSOLE_SEV_INFO, "(history empty)"); return CONSOLE_CMD_OK; }
    console_printf(c, CONSOLE_SEV_INFO, "%d entries in history", n);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_set(console_t *c, const console_args *a) {
    if (a->argc < 2) return CONSOLE_CMD_USAGE;

    console_cvar *v = console_cvar_find(&c->cvars, a->argv[1]);
    if (!v) {
        console_printf(c, CONSOLE_SEV_ERROR, "unknown cvar '%s'", a->argv[1]);
        return CONSOLE_CMD_ERR;
    }
    char val[64];
    if (a->argc == 2) {
        // set with no value just prints it, like a lazy 'get'.
        console_printf(c, CONSOLE_SEV_INFO, "%s = %s", v->name,
                       console_cvar_format(v, val, sizeof val));
        return CONSOLE_CMD_OK;
    }
    if (console_cvar_set(v, a->argv[2], val, sizeof val) != 0) {
        console_printf(c, CONSOLE_SEV_ERROR, "bad value for %s: '%s'",
                       v->name, a->argv[2]);
        return CONSOLE_CMD_ERR;
    }
    console_printf(c, CONSOLE_SEV_OK, "%s = %s", v->name, val);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_get(console_t *c, const console_args *a) {
    if (a->argc < 2) return CONSOLE_CMD_USAGE;
    console_cvar *v = console_cvar_find(&c->cvars, a->argv[1]);
    if (!v) {
        console_printf(c, CONSOLE_SEV_ERROR, "unknown cvar '%s'", a->argv[1]);
        return CONSOLE_CMD_ERR;
    }
    char val[64];
    console_printf(c, CONSOLE_SEV_INFO, "%s = %s   (%s)", v->name,
                   console_cvar_format(v, val, sizeof val),
                   v->help ? v->help : "");
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_cvars(console_t *c, const console_args *a) {
    (void)a;
    char val[64];
    for (int i = 0; i < c->cvars.count; i++) {
        console_cvar *v = &c->cvars.vars[i];
        console_printf(c, CONSOLE_SEV_INFO, "%-14s %s", v->name,
                       console_cvar_format(v, val, sizeof val));
    }
    return CONSOLE_CMD_OK;
}

// ---- game commands (need the bound pointers) ------------------------------

static console_cmd_result cmd_tp(console_t *c, const console_args *a) {
    if (a->argc < 4) return CONSOLE_CMD_USAGE;
    if (!need_player(c)) return CONSOLE_CMD_ERR;

    float x, y, z;
    if (arg_float(c, a->argv[1], &x) || arg_float(c, a->argv[2], &y) ||
        arg_float(c, a->argv[3], &z)) return CONSOLE_CMD_ERR;

    player *p = cp(c);
    p->pos = (vec3){ x, y, z };
    p->vel = (vec3){ 0, 0, 0 };   // kill momentum or you yeet off the new spot
    console_printf(c, CONSOLE_SEV_OK, "teleported to %.1f %.1f %.1f", x, y, z);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_pos(console_t *c, const console_args *a) {
    (void)a;
    if (!need_player(c)) return CONSOLE_CMD_ERR;
    player *p = cp(c);
    console_printf(c, CONSOLE_SEV_INFO, "pos %.2f %.2f %.2f  yaw %.1f pitch %.1f",
                   p->pos.x, p->pos.y, p->pos.z, p->yaw, p->pitch);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_fly(console_t *c, const console_args *a) {
    if (!need_player(c)) return CONSOLE_CMD_ERR;
    player *p = cp(c);
    if (a->argc >= 2) {
        // explicit on/off so scripts/binds are deterministic.
        if (!strcmp(a->argv[1], "on"))  p->flying = 1;
        else if (!strcmp(a->argv[1], "off")) p->flying = 0;
        else return CONSOLE_CMD_USAGE;
    } else {
        p->flying = !p->flying;
    }
    console_printf(c, CONSOLE_SEV_OK, "fly %s", p->flying ? "on" : "off");
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_give(console_t *c, const console_args *a) {
    if (a->argc < 2) return CONSOLE_CMD_USAGE;
    char *end = NULL;
    long id = strtol(a->argv[1], &end, 10);
    if (end == a->argv[1] || *end != 0 || id < 0 || id >= BLOCK_COUNT) {
        console_printf(c, CONSOLE_SEV_ERROR, "bad block id '%s' (0..%d)",
                       a->argv[1], BLOCK_COUNT - 1);
        return CONSOLE_CMD_ERR;
    }
    int n = (a->argc >= 3) ? atoi(a->argv[2]) : 1;
    if (n < 1) n = 1;
    console_printf(c, CONSOLE_SEV_OK, "gave %dx %s", n, block_get((int)id)->name);
    return CONSOLE_CMD_OK;
}

static console_cmd_result cmd_seed(console_t *c, const console_args *a) {
    (void)a;
    if (!need_world(c)) return CONSOLE_CMD_ERR;
    console_printf(c, CONSOLE_SEV_INFO, "seed = %u", cw(c)->seed);
    return CONSOLE_CMD_OK;
}

// ---- registration ---------------------------------------------------------

// these back the engine cvars. real engine globals get pointed at here at
// boot; for now they live as file statics so the table has something valid
// to chew on even before the renderer is up.
static int   cv_show_fps   = 1;
static int   cv_wireframe  = 0;
static int   cv_vsync      = 1;
static float cv_fov        = 70.0f;
static float cv_mouse_sens = 0.12f;
static int   cv_render_dist = RENDER_DISTANCE;

void console_register_builtins(console_t *c) {
    console_cmd_register(&c->cmds, "help",    cmd_help,    "help [cmd]",       "list commands or detail one");
    console_cmd_register(&c->cmds, "clear",   cmd_clear,   "clear",            "wipe the scrollback");
    console_cmd_register(&c->cmds, "echo",    cmd_echo,    "echo <text...>",   "print text back");
    console_cmd_register(&c->cmds, "history", cmd_history, "history",          "how many lines recalled");
    console_cmd_register(&c->cmds, "set",     cmd_set,     "set <cvar> [val]", "read/write a cvar");
    console_cmd_register(&c->cmds, "get",     cmd_get,     "get <cvar>",       "read a cvar + its help");
    console_cmd_register(&c->cmds, "cvars",   cmd_cvars,   "cvars",            "dump all cvars");

    console_cmd_register(&c->cmds, "tp",   cmd_tp,   "tp <x> <y> <z>", "teleport the player");
    console_cmd_register(&c->cmds, "pos",  cmd_pos,  "pos",            "print player position");
    console_cmd_register(&c->cmds, "fly",  cmd_fly,  "fly [on|off]",   "toggle noclip flight");
    console_cmd_register(&c->cmds, "give", cmd_give, "give <id> [n]",  "spawn blocks into the hotbar");
    console_cmd_register(&c->cmds, "seed", cmd_seed, "seed",           "print the world seed");

    console_cvar_register(&c->cvars, "r_show_fps",    CVAR_BOOL,  &cv_show_fps,    0, 0,   "draw the fps counter");
    console_cvar_register(&c->cvars, "r_wireframe",   CVAR_BOOL,  &cv_wireframe,   0, 0,   "wireframe terrain");
    console_cvar_register(&c->cvars, "r_vsync",       CVAR_BOOL,  &cv_vsync,       0, 0,   "vertical sync");
    console_cvar_register(&c->cvars, "r_fov",         CVAR_FLOAT, &cv_fov,        30, 110, "vertical field of view");
    console_cvar_register(&c->cvars, "in_mouse_sens", CVAR_FLOAT, &cv_mouse_sens, 0.01f, 2.0f, "mouse sensitivity");
    console_cvar_register(&c->cvars, "r_render_dist", CVAR_INT,   &cv_render_dist, 2, LOAD_DISTANCE, "chunk render radius");
}
