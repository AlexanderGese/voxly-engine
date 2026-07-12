#include "ao_sampler.h"
#include "face_dir.h"

static int solid(const mb_ctx *c, int x, int y, int z) {
    block_id b = c->sample(c->ctx, c->base_x + x, y, c->base_z + z);
    return block_is_opaque(b);
}

int mb_ao_corner(int s1, int s2, int c) {
    // both sides closed => the diagonal cant let any light in. 0fps trick.
    if (s1 && s2) return 0;
    return MB_AO_MAX - (s1 + s2 + c);
}

// sample the 8 blocks ringing the face in the plane one step out along the
// normal. we name them by compass on the (u,v) plane: m=minus, p=plus.
// layout:   mm  vm  pm
// mu  []  pu
// mp  vp  pp
void mb_ao_quad(const mb_ctx *ctx, int x, int y, int z, int face, int ao[4]) {
    int nx, ny, nz;
    mb_face_normal(face, &nx, &ny, &nz);

    int axis = (face >> 1);             // 0,0,1,1,2,2 -> x,x,y,y,z,z
    int ua, va;
    mb_axis_plane(axis, &ua, &va);

    // step one cell out along the normal: thats the plane we sample.
    int ox = x + nx, oy = y + ny, oz = z + nz;

    // build the in-plane unit steps for u and v as xyz deltas.
    int du[3] = {0,0,0}, dv[3] = {0,0,0};
    du[ua] = 1; dv[va] = 1;

#define S(su, sv) solid(ctx, ox + su*du[0] + sv*dv[0], \
                              oy + su*du[1] + sv*dv[1], \
                              oz + su*du[2] + sv*dv[2])
    int mu = S(-1, 0), pu = S(1, 0);
    int vm = S(0, -1), vp = S(0, 1);
    int mm = S(-1,-1), pm = S(1,-1);
    int mp = S(-1, 1), pp = S(1, 1);
#undef S

    // corners in winding order: (u0,v0),(u1,v0),(u1,v1),(u0,v1).
    ao[0] = mb_ao_corner(mu, vm, mm);
    ao[1] = mb_ao_corner(pu, vm, pm);
    ao[2] = mb_ao_corner(pu, vp, pp);
    ao[3] = mb_ao_corner(mu, vp, mp);

    // negative faces are wound the other way (we look at them from -normal),
    // so mirror u to keep the AO landing on the right corners.
    if (!(face & 1)) return;            // even faces are the positive ones
    int t;
    t = ao[0]; ao[0] = ao[1]; ao[1] = t;
    t = ao[3]; ao[3] = ao[2]; ao[2] = t;
}

int mb_ao_flip(const int ao[4]) {
    // the seam runs along whichever diagonal has the larger occlusion contrast.
    // standard fix: flip when ao[0]+ao[2] != ao[1]+ao[3]; specifically when the
    // 0-2 diagonal is darker, splitting along 1-3 hides the discontinuity.
    return (ao[0] + ao[2] > ao[1] + ao[3]);
}
