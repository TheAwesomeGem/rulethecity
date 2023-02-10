#version 450 core

in vec4 frag_tint_color;
in vec2 frag_uv_coord;
in float frag_texture_index;
in float frag_radius;

out vec4 pixel_color;

uniform sampler2D u_textures[9];

void main()
{
    float dist = distance(pointPos, gl_FragCoord.xy);
    if (dist > aRadius)
    discard;

    float d = dist / aRadius;
    vec3 color = mix(aColor.rgb, vec3(0.0), step(1.0-threshold, d));

    gl_FragColor = vec4(color, 1.0);
}