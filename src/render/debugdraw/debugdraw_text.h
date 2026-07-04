#ifndef RENDER_DEBUGDRAW_TEXT_H
#define RENDER_DEBUGDRAW_TEXT_H

#include "debugdraw.h"
#include "../text.h"

// world-space text labels. you anchor a string at a world point and it gets
// projected to screen coords at flush time and drawn through the engine's
// hud text renderer. labels behind the camera are culled.

// hand debugdraw the text renderer + the current screen size. without this
// labels are silently dropped (geometry still works). call once at startup
// and on resize. screen size is needed because text_draw wants pixels.
void debugdraw_text_attach(debugdraw *dd, text_renderer *tr, int sw, int sh);

// queue a label. truncated to the fixed buffer. center offsets the anchor.
void debugdraw_label(debugdraw *dd, vec3 world, ddcolor c, const char *str);

// printf flavor. same truncation rules.
void debugdraw_labelf(debugdraw *dd, vec3 world, ddcolor c,
                      const char *fmt, ...);

// project + draw all queued labels. called by debugdraw_flush; you usually
// dont invoke this yourself.
void debugdraw_flush_labels(debugdraw *dd, const camera *cam);

#endif
