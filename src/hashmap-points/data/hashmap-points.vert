#version 330 core

uniform float u_scale;
uniform mat4 u_view;
uniform mat4 u_persp;

layout (location = 0) in vec2 a_sprite_coord;
layout (location = 1) in vec3 a_point_pos;
layout (location = 2) in vec3 a_point_col;

out vec3 v_point_pos;
out vec3 v_point_col;
out vec2 v_sprite_coord;

/*
 * vertex shader main
 */
void main(void)
{
    /* Pass through the sprite uv-coordinates */
    v_sprite_coord = a_sprite_coord;

    /* Pass through the point position */
    v_point_pos = a_point_pos;

    /* Compute the sprite color */
    v_point_col = a_point_col;

    /* Compute the vertex from the sprite and point positions */
    vec3 sprite_pos = u_scale * vec3(2.0 * a_sprite_coord - 1.0, 0.0);
    vec4 point_pos = vec4(a_point_pos, 1.0) + inverse(u_view) * vec4(sprite_pos, 1.0);
    gl_Position = u_persp * (u_view * point_pos);
}
