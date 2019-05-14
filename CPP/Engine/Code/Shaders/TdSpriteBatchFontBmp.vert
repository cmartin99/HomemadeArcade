#version 450

layout (location = 0) in float in_type;
layout (location = 1) in vec3 in_pos;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in vec4 in_color;

layout (location = 0) out float out_type;
layout (location = 1) out vec2 out_uv;
layout (location = 2) out vec4 out_color;

void main() 
{
	out_type = in_type;
	out_uv = in_uv;
	out_color = in_color;
	gl_Position = vec4(in_pos.x, in_pos.y, in_pos.z, 1);
}
