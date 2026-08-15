#ifndef PLAYER_XP_XP_SAVE_H
#define PLAYER_XP_XP_SAVE_H

// (de)serialize the xp subsystem into the save stream. we persist the player
// progression (total xp + perks) but NOT the in-flight orbs — orbs are
// transient and re-derived from total on load doesn't make sense, so they
// just vanish, same as everything else's particles. nobody will miss them.

#include "../../util/serialize/serialize_writer.h"
#include "../../util/serialize/serialize_reader.h"
#include "xp.h"

#define XP_SAVE_VERSION 1

// write the xp 'XPRG' body: version, total xp, perk count + perks.
void serialize_xp_write(serialize_writer *w, const xp_system *x);

// read it back into an already-created xp_system. recomputes derived state
// from total and rebuilds the perk set. returns 0 on success, nonzero on a
// version/format mismatch (in which case x is left at defaults).
int  serialize_xp_read(serialize_reader *r, xp_system *x);

#endif
