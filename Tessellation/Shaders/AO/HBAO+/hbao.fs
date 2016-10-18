#version 430

#define M_PI 3.14159265f
#define AO_RANDOMTEX_SIZE 4

const float  NUM_STEPS = 4;
const float  NUM_DIRECTIONS = 8;


uniform vec4 projInfo;
uniform vec2 InvFullResolution;
uniform vec2 InvQuarterResolution;
uniform float NegInvR2;
uniform float NDotVBias;
uniform float AOMultiplier;
uniform float RadiusToScreen;
uniform float PowExponent;

layout(binding=0) uniform sampler2D texLinearDepth;
layout(binding=1) uniform sampler2D texRandom;
  
layout(location=0,index=0) out vec4 out_Color;


void outputColor(vec4 color) {
    out_Color = color;
}

in vec2 texCoord;


vec3 UVToView(vec2 uv, float eye_z)
{
  return vec3((uv * projInfo.xy + projInfo.zw) * (eye_z), eye_z);
}


vec3 FetchViewPos(vec2 UV)
{
  float ViewDepth = textureLod(texLinearDepth,UV,0).x;
  return UVToView(UV, ViewDepth);
}


vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
  vec3 V1 = Pr - P;
  vec3 V2 = P - Pl;
  return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 ReconstructNormal(vec2 UV, vec3 P)
{
  vec3 Pr = FetchViewPos(UV + vec2(InvFullResolution.x, 0));
  vec3 Pl = FetchViewPos(UV + vec2(-InvFullResolution.x, 0));
  vec3 Pt = FetchViewPos(UV + vec2(0, InvFullResolution.y));
  vec3 Pb = FetchViewPos(UV + vec2(0, -InvFullResolution.y));
  return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}


float Falloff(float DistanceSquare)
{
  // 1 scalar mad instruction
  return DistanceSquare * NegInvR2 + 1.0;
}


float ComputeAO(vec3 P, vec3 N, vec3 S)
{
  vec3 V = S - P;
  float VdotV = dot(V, V);
  float NdotV = dot(N, V) * 1.0/sqrt(VdotV);

  // Use saturate(x) instead of max(x,0.f) because that is faster on Kepler
  return clamp(NdotV - NDotVBias,0,1) * clamp(Falloff(VdotV),0,1);
}

vec2 RotateDirection(vec2 Dir, vec2 CosSin)
{
  return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y,
              Dir.x*CosSin.y + Dir.y*CosSin.x);
}

vec4 GetJitter()
{
	 return textureLod( texRandom, (gl_FragCoord.xy / AO_RANDOMTEX_SIZE), 0);
}

float ComputeCoarseAO(vec2 FullResUV, float RadiusPixels, vec4 Rand, vec3 ViewPosition, vec3 ViewNormal)
{
	 // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
  float StepSizePixels = RadiusPixels / (NUM_STEPS + 1);

  const float Alpha = 2.0 * M_PI / NUM_DIRECTIONS;
  float AO = 0;

  for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
  {
    float Angle = Alpha * DirectionIndex;

    // Compute normalized 2D direction
    vec2 Direction = RotateDirection(vec2(cos(Angle), sin(Angle)), Rand.xy);

    // Jitter starting sample within the first step
    float RayPixels = (Rand.z * StepSizePixels + 1.0);

    for (float StepIndex = 0; StepIndex < NUM_STEPS; ++StepIndex)
    {	
     vec2 SnappedUV = round(RayPixels * Direction) * InvFullResolution + FullResUV;
      vec3 S = FetchViewPos(SnappedUV);
       RayPixels += StepSizePixels;

      AO += ComputeAO(ViewPosition, ViewNormal, S);
    }
    }

  AO *= AOMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
  return clamp(1.0 - AO * 2.0,0,1);
}

void main()
{
  vec2 uv = texCoord;
  //vec2 uv = vec2 (gl_FragCoord.x/1920, gl_FragCoord.y/1080);
  vec3 ViewPosition = FetchViewPos(uv);


  // Reconstruct view-space normal from nearest neighbors
  vec3 ViewNormal = -ReconstructNormal(uv, ViewPosition);

  float RadiusPixels = RadiusToScreen / (ViewPosition.z);

  vec4 Rand = GetJitter();

  float AO = ComputeCoarseAO(uv, RadiusPixels, Rand, ViewPosition, ViewNormal);


  //normal blur
  outputColor(vec4(pow(AO, PowExponent)));
  //outputColor(vec4(ViewNormal, 1.0));
}
