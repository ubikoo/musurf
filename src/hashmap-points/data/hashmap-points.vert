#version 330 core

uniform mat4 u_view;
uniform mat4 u_persp;
uniform float u_scale;

layout (location = 0) in vec2 a_sprite_coord;
layout (location = 1) in vec3 a_point_pos;      /* x, y, z */
layout (location = 2) in vec3 a_point_col;      /* r, g, b */
layout (location = 3) in float a_point_radius;

out vec2 v_sprite_coord;
out vec3 v_point_pos;
out vec3 v_point_col;

/*
 * vertex shader main
 */
void main(void)
{
    /* Pass through sprite uv-coordinates and point vertex attributes */
    v_sprite_coord = a_sprite_coord;
    v_point_pos = a_point_pos;
    v_point_col = a_point_col;

    /* Compute the vertex from the sprite and point positions */
    float radius = u_scale * a_point_radius;
    vec2 sprite_xy = radius * (2.0 * a_sprite_coord - 1.0);
    vec4 sprite_pos = inverse(u_view) * vec4(sprite_xy, 0.0, 1.0);
    vec4 point_pos = vec4(a_point_pos, 1.0);

    gl_Position = u_persp * (u_view * (point_pos +  sprite_pos));
}
