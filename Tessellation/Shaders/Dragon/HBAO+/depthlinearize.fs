#version 430


uniform float near;
uniform float far;
layout(binding=0)  uniform sampler2D inputTexture;


out float out_Color;

float reconstructCSZ(float d, float near, float far) 
{
	return near * far / (far - d * (far - near));
}


void main() 
{
	 float depth = texelFetch(inputTexture, ivec2(gl_FragCoord.xy), 0).x;
	 out_Color = reconstructCSZ(depth, near, far);
}