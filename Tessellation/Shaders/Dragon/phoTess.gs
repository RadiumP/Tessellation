#version 440 core
#extension GL_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable

layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 3) out;


uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;


in vec3 tePosition[3];
in vec3 teNormal[3];
in vec2 teTexCoord[3];
in vec3 tePatchDistance[3];

out vec3 gPosition;
out vec3 gNormal;
out vec2 gTexCoord;
out vec3 gPatchDistance;
out vec3 gTriDistance;



void main(void)
{

	
	

	
	

	gTexCoord = teTexCoord[0];
	gNormal = teNormal[0];
	gPatchDistance = tePatchDistance[0];
	gTriDistance = vec3(1, 0, 0);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gTexCoord = teTexCoord[1];
	gNormal = teNormal[1];
	gPatchDistance = tePatchDistance[1];
	gTriDistance = vec3(0, 1, 0);
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	gTexCoord = teTexCoord[2];
	gNormal = teNormal[2];
	gPatchDistance = tePatchDistance[2];
	gTriDistance = vec3(0, 0, 1);
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();


	
	

	EndPrimitive();


}