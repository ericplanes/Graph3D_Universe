#version 330

in vec3 a_vertex;
in vec2 a_uv;
in vec3 a_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 v_uv;
out vec3 v_normal;
out vec3 v_pos;

void main()
{
	v_uv = a_uv;
	v_normal = a_normal;
	v_pos = ( u_model * vec4(a_vertex,1.0)).xyz;
	gl_Position = u_projection * u_view* vec4( v_pos , 1.0 );
}



