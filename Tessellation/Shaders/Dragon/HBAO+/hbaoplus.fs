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

//not layered
uniform vec2 g_Float2Offset;
uniform vec4 g_Jitter;
  

layout(binding=0) uniform sampler2D texLinearDepth;
layout(binding=1) uniform sampler2D texViewNormal;

vec2 getQuarterCoord(vec2 UV){
    return UV;
  }
  
layout(location=0,index=0) out vec4 out_Color;
  
void outputColor(vec4 color) 
{
    out_Color = color;
}


in vec2 texCoord;



vec3 UVToView(vec2 uv, float eye_z)
{
  return vec3((uv * projInfo.xy + projInfo.zw) * (eye_z), eye_z);
}

vec3 FetchQuarterResViewPos(vec2 UV)
{
  float ViewDepth = textureLod(texLinearDepth,getQuarterCoord(UV),0).x;
  return UVToView(UV, ViewDepth);
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
	 return g_Jitter;
}

float ComputeCoarseAO(vec2 FullResUV, float RadiusPixels, vec4 Rand, vec3 ViewPosition, vec3 ViewNormal)
{
	RadiusPixels /= 4.0;

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
      vec2 SnappedUV = round(RayPixels * Direction) * InvQuarterResolution + FullResUV;
      vec3 S = FetchQuarterResViewPos(SnappedUV);
      RayPixels += StepSizePixels;

      AO += ComputeAO(ViewPosition, ViewNormal, S);
    }
  }

  AO *= AOMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
  return clamp(1.0 - AO * 2.0,0,1);
}

void main()
{
  
  vec2 base = floor(gl_FragCoord.xy) * 4.0 + g_Float2Offset;
  vec2 uv = base * (InvQuarterResolution / 4.0);

  vec3 ViewPosition = FetchQuarterResViewPos(uv);
  vec4 NormalAndAO =  texelFetch( texViewNormal, ivec2(base), 0);
  vec3 ViewNormal =  -(NormalAndAO.xyz * 2.0 - 1.0);

  // Compute projection of disk of radius control.R into screen space
  float RadiusPixels = RadiusToScreen / (ViewPosition.z);

  // Get jitter vector for the current full-res pixel
  vec4 Rand = GetJitter();

  float AO = ComputeCoarseAO(uv, RadiusPixels, Rand, ViewPosition, ViewNormal);


  //normal blur
  //outputColor(vec4(pow(AO, PowExponent)));

  outputColor(vec4(AO));
  //outputColor(vec4(0.4,0.5,0.2,1.0));


}