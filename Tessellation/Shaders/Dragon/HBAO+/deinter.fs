#version 430

layout(location=0) uniform vec4 info; // xy
vec2 uvOffset = info.xy;
vec2 invResolution = info.zw;

layout(binding=0)  uniform sampler2D gPositionDepth;

layout(location=0,index=0) out float out_Color[8];

//----------------------------------------------------------------------------------



void main() {
  vec2 uv = floor(gl_FragCoord.xy) * 4.0 + uvOffset;
  ivec2 tc = ivec2(uv);

  out_Color[0] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(0,0)).a;
  out_Color[1] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(1,0)).a;
  out_Color[2] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(2,0)).a;
  out_Color[3] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(3,0)).a;
  out_Color[4] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(0,1)).a;
  out_Color[5] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(1,1)).a;
  out_Color[6] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(2,1)).a;
  out_Color[7] = -texelFetchOffset(gPositionDepth, tc, 0, ivec2(3,1)).a;
}

