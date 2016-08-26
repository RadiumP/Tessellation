#version 440 core



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
    vPosition =  (view * model * vec4(Position, 1.0)).xyz;
    //gl_Position.xyz = vPosition;
    
    //vNormal = normalize(Normal);
    vNormal = normalize(view * model * vec4(Normal, 0.0)).xyz; // normal matrix???
    vTexCoord = texCoord;
}
