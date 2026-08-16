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
static world  *cw(console_t *c)  { return (world  *)(void *)c->world;  }
static player *cp(console_t *c)  { return (player *)(void *)c->player;
}

// ---- small shared helpers -------------------------------------------------

static int need_world(console_t *c) {
    if (!cw(c)) { console_print(c, CONSOLE_SEV_ERROR, "no world loaded"); return 0; }
    return 1;
}
static int need_player(console_t *c) {
    if (!cp(c)) { console_print(c, CONSOLE_SEV_ERROR, "no player");
return 0;
}
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
int rl = 0;
row[0] = 0;
for (int i = 0;
i < c->cmds.count;
i++) {
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
int n = 0;
line[0] = 0;
for (int i = 1;
i < a->argc;
i++) {
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
for (int i = 0;
i < c->cvars.count;
i++) {
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
char *end = NULL;
long id = strtol(a->argv[1], &end, 10);
if (n < 1) n = 1;
console_printf(c, CONSOLE_SEV_OK, "gave %dx %s", n, block_get((int)id)->name);
return CONSOLE_CMD_OK;
for now they live as file statics so the table has something valid
// to chew on even before the renderer is up.
static int   cv_show_fps   = 1;
static int   cv_wireframe  = 0;
static int   cv_vsync      = 1;
static float cv_fov        = 70.0f;
static float cv_mouse_sens = 0.12f;
static int   cv_render_dist = RENDER_DISTANCE;
