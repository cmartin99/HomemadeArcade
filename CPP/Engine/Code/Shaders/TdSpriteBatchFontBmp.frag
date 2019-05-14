#version 450

layout (location = 0) in float in_type;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform sampler2D sampler_color;

void main() 
{
	vec4 final_color = in_color;

	if (in_type == 0)
	{
		// sprite
		if (in_uv.x >= 0) final_color *= texture(sampler_color, in_uv);
  		if (final_color.a == 0) discard; 
	}
	else
	{
		// text
		float a = texture(sampler_color, in_uv).a;
		if (a < 0.5) discard;
	}

	out_color = final_color;
}