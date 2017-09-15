#version 450

layout (binding = 0) uniform sampler2D sampler_color;
layout (binding = 1) uniform UBO 
{
	vec4 outline_color;
	float outline_width;
	float outline;
} ubo;

layout (location = 0) in float in_type;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

layout (location = 0) out vec4 out_color;

void main() 
{
	out_color = in_color;

	if (in_type == 1)
	{
		float distance = texture(sampler_color, in_uv).r;
		float smooth_width = fwidth(distance);	
		float alpha = smoothstep(0.5 - smooth_width, 0.5 + smooth_width, distance);
		vec3 rgb = vec3(alpha);
									 
		if (ubo.outline > 0.0) 
		{
			float w = 1.0 - ubo.outline_width;
			alpha = smoothstep(w - smooth_width, w + smooth_width, distance);
			rgb += mix(vec3(alpha), ubo.outline_color.rgb, alpha);
		}									 
		out_color = in_color * vec4(rgb, alpha);
	}
}