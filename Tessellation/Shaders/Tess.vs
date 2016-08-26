#version 410 core

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;


in vec4 gl_Vertex;
in vec3 gl_Color;

out vec4 color_vs;

void main(void)
{
	color_vs.xyz = gl_Color.xyz;
	gl_Position = projectionMatrix * modelViewMatrix * vec4(gl_Vertex.xyz, 1.0);
}
