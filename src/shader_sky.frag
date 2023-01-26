#version 330

in vec2 v_uv;
in vec3 v_normal;

out vec4 fragColor;

uniform vec3 u_color;
uniform sampler2D u_texture_sky;
uniform vec3 u_light_dir;

void main(void){
	vec4 texture_color = texture(u_texture_sky, v_uv);
    fragColor = vec4(texture_color.xyz, 1.0);
}