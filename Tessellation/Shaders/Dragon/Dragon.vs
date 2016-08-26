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
    //vPosition =  (view * model * vec4(Position, 1.0f)).xyz;
    //mat3 normalMatrix = transpose(inverse(mat3(view * model)));
  	vPosition = Position;
    //vNormal = normalize( mat3(view * model)* Normal); // world what should be the normal?
    vNormal = Normal;
    
    //vTexCoord = texCoord;
}
