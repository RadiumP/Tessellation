#version 400 core





layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 texCoord;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;


uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;


void main()
{
    //vPosition =  ( vec4(Position, 1.0f)).xyz;
  	vPosition = Position;
    //vNormal = normalize( ( vec4(Normal, 0.0f)).xyz); // world what should be the normal?
    vNormal = Normal;
    
    //vTexCoord = texCoord;
}
