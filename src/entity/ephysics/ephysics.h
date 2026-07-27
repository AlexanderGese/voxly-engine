#ifndef ENTITY_EPHYSICS_H
#define ENTITY_EPHYSICS_H

// entity physics module umbrella header. include this and you get the whole
// pipeline: swept-aabb collision, step-up, buoyancy, friction.
//
// the engine ticks entities at a fixed rate; call ephysics_tick_entity() (or
// the lower-level ephysics_step_body) once per fixed step. the player uses the
// same path with an ephys_input filled from the controller.
//
// layout, roughly bottom-up:
// types      - plain data shared by everyone
// aabb       - box math the sweep needs
// material   - per-body motion profiles + entity glue
// broadphase - world -> candidate block boxes
// sweep      - swept-aabb + slide resolve
// step       - ledge step-up
// fluid      - buoyancy, drag, currents
// friction   - surface + medium drag
// integrate  - ties it all together per tick
// debug      - introspection for the f3-style overlay

#include "ephysics_types.h"
#include "ephysics_aabb.h"
#include "ephysics_material.h"
#include "ephysics_broadphase.h"
#include "ephysics_sweep.h"
#include "ephysics_step.h"
#include "ephysics_fluid.h"
#include "ephysics_friction.h"
#include "ephysics_integrate.h"
#include "ephysics_debug.h"

#endif
