#include "settings_menu.h"
#include "settings_panel.h"
#include <string.h>
settings_model_snapshot(&sm->model);
memcpy(sm->model.work, sm->model.live, sizeof sm->model.work);
settings_model_refresh(&sm->model);
settings_tabs_set(&sm->tabs, SETTINGS_TAB_VIDEO);
for (int i = 0;
i < SETTINGS_TAB_COUNT;
i++)
        settings_layout_reset(&sm->layouts[i]);
wg_id id = wg_gen_id_n(ctx, "settings.footer", salt);
int hov = 0, held = 0, hit = 0;
wg_draw_rect(&ctx->draw, r, bg);
wg_draw_border(&ctx->draw, r, st->widget_border, st->border_thick);
wg_rgba tc = enabled ? st->text : st->text_disabled;
float scale = st->font_scale;
float tw = (float)strlen(label) * 6.0f * scale;
float tx = r.x + (r.w - tw) * 0.5f;
