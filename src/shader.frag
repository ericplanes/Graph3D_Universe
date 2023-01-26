#version 330

in vec2 v_uv;
in vec3 v_pos;
out vec4 fragColor;
in vec3 v_normal;
uniform vec3 u_color;
uniform sampler2D u_texture;
uniform vec3 u_light_dir;
uniform vec3 u_light_color;
uniform vec3 u_eye;
uniform float u_glossiness;

void main(void){
	vec3 L = normalize(u_light_dir);
	vec3 N = normalize(v_normal);
	vec3 R = reflect(-L,N);
	vec3 E = normalize(u_eye - v_pos);
	float RdotE = max(0.0,dot(R,E));

	float NdotL = max(0.0,dot(N,L));

	vec3 specular_color = vec3(0.0, 0.0, 0.0);
	
	if(!(NdotL <= 0.0)) {
		specular_color = u_light_color * pow(RdotE, u_glossiness);
	} 

	vec4 texture_color = texture(u_texture,v_uv);
	vec3 ambient_color = texture_color.xyz * 0.1f;

	vec3 final_color = texture_color.xyz * NdotL;

    fragColor =  vec4(ambient_color + final_color + specular_color, 1.0);
}
