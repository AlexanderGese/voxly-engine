#version 330 core

// debug draw fragment shader. passes the vertex color straight through.
// for GL_POINTS we round the corners off so points look like dots instead
// of squares — cheap discard against the point-sprite coord.

in  vec4 v_col;
out vec4 frag;

void main() {
    // gl_PointCoord is (0,0)..(1,1) inside a point sprite, and ends up
    // (0,0) for line/tri frags which the length test below tolerates.
    vec2 d = gl_PointCoord - vec2(0.5);
    if (dot(d, d) > 0.25) discard;   // outside the inscribed circle

    frag = v_col;
    if (frag.a < 0.01) discard;
}
