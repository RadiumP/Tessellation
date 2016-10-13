#version 440 




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
   
  	vPosition = Position;
   
    vNormal = normalize(Normal);
    
   
}
