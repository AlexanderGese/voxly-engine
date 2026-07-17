#ifndef RENDER_SSAO_SAMPLE_H
#define RENDER_SSAO_SAMPLE_H

#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"

// view-space sampling math. this is the heart of ssao and the part most
// worth having on the cpu too: the gpu shader does the exact same thing but
// in glsl. keeping a c reference lets us unit-test the occlusion logic
// without spinning up a gl context, and it documents the algorithm.

// project a view-space point through the projection matrix into [0,1] uv +
// ndc depth. returns 0 if the point is behind the near plane (w <= 0).
int  ssaox_project(mat4 proj, vec3 view_pos, vec2 *out_uv, float *out_depth);

// build a tangent-basis matrix (TBN columns) from a surface normal and an
// in-plane rotation vector, gram-schmidt'd. used to orient the kernel.
// rotation is the (x,y,0) noise vector.
mat4 ssaox_tbn(vec3 normal, vec3 rotation);

// the smoothstep range check. given the radius and the depth difference
// between the fragment and the depth fetched at a sample's screen position,
// returns a 0..1 weight that fades out samples that are too far behind. this
// is what stops a near wall from being darkened by far-away geometry.
float ssaox_range_check(float radius, float frag_view_z, float sample_view_z);

// decide if a single kernel sample is occluded. given the sample's view-space
// z and the scene depth fetched at the sample's screen position (also view z),
// plus the bias, returns the occlusion contribution already multiplied by the
// range check. 0 = not occluded, up to 1 = fully occluded+in range.
float ssaox_occlusion_term(float radius, float bias,
                           float sample_view_z, float scene_view_z);

#endif
