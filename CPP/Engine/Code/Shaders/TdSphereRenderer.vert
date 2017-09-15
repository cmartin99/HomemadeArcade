#version 450

layout (set = 0, binding = 0) uniform UBOType
{
	mat4 view_proj;
	vec3 cam_pos;
    float far_clip;
} ubo;

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec4 in_color;
layout (location = 0) out vec4 out_color;

void main() 
{
	gl_Position = ubo.view_proj * in_pos;
	out_color = in_color;
}
