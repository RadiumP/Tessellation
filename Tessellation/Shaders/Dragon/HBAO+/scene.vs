#version 430

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

in  vec3 pos;
in  vec3 normal;
in  vec4 color;


out vec3 outPos;
out vec3 outNormal;



void main()
{
  gl_Position = projection * view * vec4(pos,1);
  outPos = pos;
  outNormal = normal;
 
}
