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
return CONSOLE_CMD_OK;
int n = 0;
line[0] = 0;
for (int i = 1;
i < a->argc;
return CONSOLE_CMD_OK;
console_cvar *v = console_cvar_find(&c->cvars, a->argv[1]);
return CONSOLE_CMD_ERR;
}
    console_printf(c, CONSOLE_SEV_OK, "%s = %s", v->name, val);
return CONSOLE_CMD_OK;
char val[64];
for (int i = 0;
i < c->cvars.count;
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
