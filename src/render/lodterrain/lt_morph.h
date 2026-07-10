#ifndef RENDER_LODTERRAIN_LT_MORPH_H
#define RENDER_LODTERRAIN_LT_MORPH_H

// geomorphing: the little fade that hides the level pop. when a chunk crosses a
// band it doesn't snap from one mesh to the next; it spends a few frames blending
// so the silhouette eases over instead of jumping. we can't morph vertex
// positions cheaply across two separately-built meshes, so we do the pragmatic
// thing the shader can act on: hand it a 0..1 weight and let it cross-dim, plus
// a tiny vertical nudge that pulls the just-popped coarse mesh down to meet the
// finer one's average height. nobody's ever noticed the seam since.

// per-chunk morph state, parked alongside the cache entry by the manager.
typedef struct {
    int   from_level;   // level we're fading out of
    int   to_level;     // level we're fading into (== entry->level once done)
    float t;            // 0 = fully `from`, 1 = fully `to`
    float speed;        // t per second; tuned so a transition takes ~0.3s
    int   active;       // a morph is in flight
} lt_morph_state;

// init a morph state to "settled at `level`", no transition in flight.
void  lt_morph_init(lt_morph_state *ms, int level, float speed);

// call when the selector picks a new level for a chunk. kicks off (or retargets)
// a fade from the current level to `new_level`. a no-op if already heading there.
void  lt_morph_retarget(lt_morph_state *ms, int new_level);

// advance the fade by dt seconds. returns 1 while a morph is still running (so
// the manager keeps the chunk's draw flagged), 0 once it settles.
int   lt_morph_tick(lt_morph_state *ms, float dt);

// the 0..1 blend weight to hand the shader this frame. eased with smoothstep so
// the fade has no hard start/stop.
float lt_morph_weight(const lt_morph_state *ms);

// vertical offset (in blocks) to bias the coarse mesh by during the fade, so a
// just-appeared coarse cell sinks to roughly where the finer surface was. eases
// to zero as the morph completes. `cell_delta` is the coarse cell size diff.
float lt_morph_y_bias(const lt_morph_state *ms, float cell_delta);

#endif
