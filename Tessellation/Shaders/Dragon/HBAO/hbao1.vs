#version 440 

out vec2 vTexCoords;

void main()
{
  uint idx = gl_VertexID % 3; // allows rendering multiple fullscreen triangles
  vec4 pos =  vec4(
      (float( idx     &1U)) * 4.0 - 1.0,
      (float((idx>>1U)&1U)) * 4.0 - 1.0,
      0, 1.0);
  gl_Position = pos;
  vTexCoords = pos.xy * 0.5 + 0.5;
}