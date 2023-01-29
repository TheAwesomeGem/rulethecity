#version 450 core

in vec4 frag_tint_color;
in vec2 frag_uv_coord;
in float frag_texture_index;

out vec4 pixel_color;

uniform sampler2D u_textures[9];

void main()
{
    int text_index = int(frag_texture_index);
    vec4 potential_pixel_color = texture(u_textures[text_index], frag_uv_coord) * frag_tint_color;

    if(potential_pixel_color.a < 0.1) {
        discard;
    }

    pixel_color = potential_pixel_color;
}