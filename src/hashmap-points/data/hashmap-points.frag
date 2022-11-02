#version 330 core

#define kGray vec3(0.5)

uniform float u_width;
uniform float u_height;

in vec2 v_sprite_coord;
in vec3 v_point_pos;
in vec3 v_point_col;

out vec4 frag_col;

/* ----------------------------------------------------------------------------
 * Fragment shader main
 */
void main(void)
{
    /* Compute the normal to the sphere at the fragment position. */
    vec3 N;
    N.xy = 2.0*v_sprite_coord - 1.0;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0) {
        discard;                // discard fragments outside the sprite disc.
    }
    N.z = sqrt(1.0 - r2);

    /* Shade the fragment using a simple directional light. */
    const vec3 light_dir = normalize(vec3(0.5, 0.5, 1.5));
    float diffuse = max(0.0, dot(light_dir, N));
    float alpha = length(v_point_col - kGray) > 0.0 ? 1.0 : 0.1;
    frag_col = vec4(v_point_col*diffuse, alpha);
}
