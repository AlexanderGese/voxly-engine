#ifndef RENDER_DOF_PASS_H
#define RENDER_DOF_PASS_H
#include "../gl.h"
#include "dof_programs.h"
#include "dof_target.h"
#include "dof_quad.h"
#include "dof_coc.h"
void dof_pass_coc(const dof_programs *prog,
                  const dof_quad *quad,
                  glid depth_tex,
                  dof_target *coc_dst,
                  const dof_lens *lens,
                  float znear, float zfar,
                  float texel_scale);
#endif
