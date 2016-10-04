#version 440



#define AO_RANDOMTEX_SIZE 4.0

const float PI = 3.14159265;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;//reconstruct
uniform sampler2D texNoise;
//uniform sampler2D depthTex;


uniform mat4 projection;

uniform vec2 FocalLen;
uniform vec2 UVToViewA;
uniform vec2 UVToViewB;

const float NegInvR2 = -1.0f / (1.0f * 1.0f);
const float R = 1.0f;
const float fov = 45.0f;
const vec2 noiseScale = vec2(1920.0f/4.0f, 1080.0f/4.0f);

const float  NUM_STEPS = 16;
const float  NUM_DIRECTIONS = 16; // texRandom/g_Jitter initialization depends on this
const float  NDotVBias = 0.1;

const float AOMultiplier = 1.5;
const vec2 InvFullResolution = vec2(1.0f/1920.0f, 1.0f/1080.0f);
in vec2 vTexCoords;
in vec2 vPosition;

out vec2 FragColor;


vec3 UVToView(vec2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;
	return vec3(uv * z, z);
}


vec3 FetchViewPos(vec2 UV)
{
  //float z = -texture(gPositionDepth, UV).w ; 
  //return UVToView(UV, z);
  return texture(gPositionDepth, UV).xyz; 
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

  // (cos(Alpha),sin(Alpha),rand1,rand2)
  return texture( texNoise, (gl_FragCoord.xy / AO_RANDOMTEX_SIZE));

  //return texture(texNoise, vTexCoords.xy * noiseScale);

}


float ComputeCoarseAO(vec2 FullResUV, float RadiusPixels, vec4 Rand, vec3 ViewPosition, vec3 ViewNormal)
{
	float StepSizePixels = RadiusPixels / (NUM_STEPS + 1);

  const float Alpha = 2.0 * PI / NUM_DIRECTIONS;
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
	 //vec2 uv = vTexCoords;
   	vec2 uv = vec2 (gl_FragCoord.x/1920, gl_FragCoord.y/1080); //same thing
  	vec3 ViewPosition = FetchViewPos(uv);
    

  	//vec3 ViewNormal = ReconstructNormal(uv, ViewPosition);
    vec3 ViewNormal = texture(gNormal, uv).xyz;

  	float RadiusToScreen = R * 0.5f * 1080.0f / (tan(fov * PI/180.0f * 0.5f) * 2.0f);
  	float RadiusPixels = RadiusToScreen / (ViewPosition.z);

  	vec4 Rand = GetJitter();

  	float AO = ComputeCoarseAO(uv, RadiusPixels, Rand, ViewPosition, ViewNormal);

  	FragColor = vec2(AO, AO);
}