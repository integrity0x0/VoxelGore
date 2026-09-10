#version 450

vec2 positions[4] = vec2[](
    vec2(-0.02, 0.0),
    vec2( 0.02, 0.0),
    vec2( 0.0, -0.02),
    vec2( 0.0,  0.02)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}