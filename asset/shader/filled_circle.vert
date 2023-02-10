#version 450 core

layout (location = 0) in vec3 cpu_vertex_point;
layout (location = 1) in vec4 cpu_tint_color;
layout (location = 2) in vec2 cpu_uv;
layout (location = 3) in float cpu_texture_index;
layout (location = 4) in float cpu_radius;

out vec4 frag_tint_color;
out vec2 frag_uv_coord;
out float frag_texture_index;
out float frag_radius;

uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * vec4(cpu_vertex_point, 1.0);
    frag_tint_color = cpu_tint_color;
    frag_uv_coord = cpu_uv;
    frag_texture_index = cpu_texture_index;
    frag_radius = cpu_radius;
}