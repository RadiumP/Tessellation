#version 440 




layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;



out vec2 TexCoords;

void main()
{
    gl_Position = vec4(Position, 1.0f);
    TexCoords = texCoord;
}