#version 430



layout(binding=0)  uniform sampler2DArray texResultsArray;
layout(binding=1)  uniform sampler2D test;


layout(location=0,index=0) out vec4 out_Color;

//----------------------------------------------------------------------------------

void main() {
  ivec2 FullResPos = ivec2(gl_FragCoord.xy);
  ivec2 Offset = FullResPos & 3;
  int SliceId = Offset.y * 4 + Offset.x;
  ivec2 QuarterResPos = FullResPos >> 2;
  

//  out_Color = vec4(texelFetch( texResultsArray, ivec3(QuarterResPos, SliceId), 0).xy,0,0);

  out_Color = vec4(texelFetch( texResultsArray, ivec3(QuarterResPos, SliceId), 0).x);

  //out_Color = vec4(1.0, 0.2,0.0,0.0);
  out_Color = vec4(texelFetch(test, QuarterResPos, 0).x);
  
}
