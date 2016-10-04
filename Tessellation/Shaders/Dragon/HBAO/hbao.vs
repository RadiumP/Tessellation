#version 440 




layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 texCoord;

out vec2 vTexCoords;
out vec2 vPosition;
void main()
{
    gl_Position = vec4(Position, 1.0f);
    vTexCoords = texCoord;
    //vPosition = texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    //gl_Position = vec4(vPostition, 0.0, 1.0);
}