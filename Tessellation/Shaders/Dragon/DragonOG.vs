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
    vPosition = Position;
    //gl_Position.xyz = Position;
    //gl_Position = projection * view * model * vec4(vPosition, 1);
    vNormal = normalize(Normal);
    //vNormal = vec3(1.0,0.0,0.0);
    
    //vTexCoord = texCoord;
}
