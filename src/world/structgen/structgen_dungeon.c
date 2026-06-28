#include "structgen_dungeon.h"
#include "structgen_rand.h"
#define DUNGEON_DEPTH   6
#define ROOM_BUDGET     7
#define CORRIDOR_LEN    4
typedef struct {
    structgen_box box;
    int           exits_used;   // dont overload one room with corridors
} room_slot;
static int project_room(const structgen_box *from, structgen_dir side,
                        structgen_rng *r, structgen_box *corr, structgen_box *room) {
    int dx, dz;
    structgen_dir_step(side, &dx, &dz);

    int fy = from->y0;
    // door cell sits on the shared face, mid-height irrelevant for footprint.
    int fcx = (from->x0 + from->x1) / 2;
    int fcz = (from->z0 + from->z1) / 2;

    int rh = 4;                         // rooms are 4 tall (floor..ceiling)
    int rw = 5 + structgen_rng_range(r, 0, 2);
    int rd = 5 + structgen_rng_range(r, 0, 2);

    if (dx != 0) {
        int cx = (dx > 0) ? from->x1 : from->x0 - CORRIDOR_LEN;
        *corr = structgen_box_make(cx, fy, fcz - 1, cx + CORRIDOR_LEN, fy + 3, fcz + 2);
        int rx = (dx > 0) ? corr->x1 : corr->x0 - rw;
        *room = structgen_box_make(rx, fy, fcz - rd / 2, rx + rw, fy + rh, fcz - rd / 2 + rd);
    } else {
        int cz = (dz > 0) ? from->z1 : from->z0 - CORRIDOR_LEN;
        *corr = structgen_box_make(fcx - 1, fy, cz, fcx + 2, fy + 3, cz + CORRIDOR_LEN);
        int rz = (dz > 0) ? corr->z1 : corr->z0 - rd;
        *room = structgen_box_make(fcx - rw / 2, fy, rz, fcx - rw / 2 + rw, fy + rh, rz + rd);
    }
    return 1;
}

void structgen_dungeon_layout(structgen_plan *plan, const structgen_site *site) {
    structgen_rng rng;
structgen_rng_seed(&rng, site->seed ^ 0xd0a6e0u);
int top_y = site->ground_y - DUNGEON_DEPTH;
int floor_y = top_y - 4;
if (floor_y < 2) floor_y = 2;
structgen_plan_init(plan, site->anchor_x, floor_y, site->anchor_z);
room_slot rooms[ROOM_BUDGET];
int nrooms = 0;
structgen_box first = structgen_box_at(site->anchor_x - 3, floor_y,
                                           site->anchor_z - 3, 7, 4, 7);
if (structgen_plan_try_add(plan, PIECE_ROOM, first, STRUCTGEN_NORTH,
                               structgen_seed_mix(site->seed, 1u))) {
        rooms[nrooms].box = first;
        rooms[nrooms].exits_used = 0;
        nrooms++;
    }

    // grow. each iteration: pick a room with spare exits, pick a side, project.
    int attempts = 0;
while (nrooms < ROOM_BUDGET && attempts < ROOM_BUDGET * 6) {
        attempts++;
        // weighted toward newer rooms so the dungeon snakes rather than blobs.
        int pick = structgen_rng_range(&rng, nrooms / 2, nrooms - 1);
        room_slot *from = &rooms[pick];
        if (from->exits_used >= 3) continue;

        structgen_dir side = (structgen_dir)structgen_rng_range(&rng, 0, 3);
        structgen_box corr, room;
        if (!project_room(&from->box, side, &rng, &corr, &room)) continue;

        // corridor must fit first; if it overlaps, this side's blocked.
        uint32_t cseed = structgen_seed_mix(site->seed, (uint32_t)(nrooms * 31 + 7));
        if (!structgen_plan_try_add(plan, PIECE_CORRIDOR, corr, side, cseed))
            continue;

        uint32_t rseed = structgen_seed_mix(site->seed, (uint32_t)(nrooms * 53 + 17));
        if (!structgen_plan_try_add(plan, PIECE_ROOM, room, side, rseed)) {
            // room didnt fit but corridor's already in; leave it as a dead end.
            from->exits_used++;
            continue;
        }

        rooms[nrooms].box = room;
        rooms[nrooms].exits_used = 0;
        nrooms++;
        from->exits_used++;
    }
}

int structgen_dungeon_generate(structgen_buffer *out, const structgen_site *site) {
    structgen_plan plan;
structgen_dungeon_layout(&plan, site);
return structgen_plan_build(&plan, out);
}
