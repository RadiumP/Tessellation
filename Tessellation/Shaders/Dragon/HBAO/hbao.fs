// http://www.nvidia.co.uk/object/siggraph-2008-HBAO.html

#version 440

const float PI = 3.14159265;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;


uniform mat4 projection;

uniform vec2 FocalLen;
uniform vec2 UVToViewA;
uniform vec2 UVToViewB;

uniform vec2 LinMAD;

const vec2 noiseScale = vec2(800.0f/4.0f, 600.0f/4.0f);
const vec2 AORes = vec2(800.0,600.0);
const vec2 InvAORes = vec2(1.0f/800.0f, 1.0f/600.0f);

const float AOStrength = 1.9;
const float R = 0.3;
const float R2 = 0.3 * 0.3;
const float NegInvR2 = -1.0 / (0.3 * 0.3);
const float TanBias = tan (45.0 * PI / 180.0);
const float MaxRadiusPixels = 100.0;

const int NumDirections = 6;
const int NumSamples = 4;
const float far = 50.0f;
const float near = 0.1f;

in vec2 vTexCoords;
//in vec2 vPosition;

//out float FragColor;
out vec2 FragColor;

float ViewSpaceZFromDepth(float d)
{
	d = (d * 2.0 - 1.0) ;
	
	return -1.0 / (LinMAD.x * d + LinMAD.y);
}

vec3 UVToViewSpace(vec2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;
	return vec3(uv *z, z);
}

vec3 GetViewPos(vec2 uv)
{
	//z/(w+1)/2 ?
	//float z = ViewSpaceZFromDepth(texture(gPositionDepth, uv).z/(texture(gPositionDepth, uv).w + 1.0f) / 2.0f);
	//float z = ViewSpaceZFromDepth(-texture(gPositionDepth, uv).w );
	float z = -texture(gPositionDepth, uv).w ; 
	//float z = texture(gPositionDepth, uv).z/(texture(gPositionDepth, uv).w + 1.0f) / 2.0f;
	return UVToViewSpace(uv, z);
}



float TanToSin(float x)
{
	return x * inversesqrt(x * x + 1.0);
}

float InvLength(vec2 V)
{
	return inversesqrt(dot(V,V));
}

float Tangent(vec3 V)
{
	return V.z * InvLength(V.xy);
}

float BiasedTangent(vec3 V)
{
	return V.z * InvLength(V.xy) + TanBias;
}

float Tangent(vec3 P, vec3 S)
{
	return -(P.z - S.z) * InvLength(S.xy - P.xy);
}

float Length2(vec3 V)
{
	return dot(V,V);
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
	vec3 V1 = Pr - P;
	vec3 V2 = P - Pl;
	return (Length2(V1) < Length2(V2)) ? V1 : V2;
}

vec2 SnapUVOffset(vec2 uv)
{
	return round(uv * AORes) * InvAORes;
}

float Falloff(float d2)
{
	return d2 * NegInvR2 + 1.0f;
}

float HorizonOcclusion(vec2 deltaUV, vec3 P, vec3 dPdu, vec3 dPdv, float randstep,  float numSamples)
{
	float ao = 0;

	vec2 uv = vTexCoords + SnapUVOffset(randstep * deltaUV);
	deltaUV = SnapUVOffset(deltaUV);

	vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

	float tanH = BiasedTangent(T);
	float sinH = TanToSin(tanH);

	float tanS;
	float d2;
	vec3 S;

	for(float s = 1; s <= numSamples; ++s)
	{
		uv += deltaUV;
		S = GetViewPos(uv);
		tanS = Tangent(P, S);
		d2 = Length2(S - P);

		if(d2 < R2 && tanS > tanH)
		{
			float sinS = TanToSin(tanS);
			ao += Falloff(d2) * (sinS - sinH);

			tanH = tanS;
			sinH = sinS;
		}
	}

	return ao;

}

vec2 RotateDirections(vec2 Dir, vec2 CosSin)
{
	return vec2(Dir.x * CosSin.x - Dir.y * CosSin.y, Dir.x * CosSin.y + Dir.y * CosSin.x);
}

void ComputeSteps(inout vec2 stepSizeUv, inout float numSteps, float rayRadiusPix, float rand)
{
	numSteps = min(NumSamples, rayRadiusPix);
	float stepSizePix = rayRadiusPix / (numSteps + 1);
	float maxNumSteps = MaxRadiusPixels / stepSizePix;
	if(maxNumSteps < numSteps)
	{
		numSteps = floor(maxNumSteps + rand);
		numSteps = max(numSteps, 1);
		stepSizePix = MaxRadiusPixels / numSteps;
	}

	stepSizeUv = stepSizePix * InvAORes;
}

void main()
{
	float numDirections = NumDirections;

	vec3 P, Pr, Pl, Pt, Pb;
	P = GetViewPos(vTexCoords);

	Pr = GetViewPos(vTexCoords + vec2(InvAORes.x, 0));
	Pl = GetViewPos(vTexCoords + vec2(-InvAORes.x, 0));
	Pt = GetViewPos(vTexCoords + vec2(0, InvAORes.y));
	Pb = GetViewPos(vTexCoords + vec2(0, -InvAORes.y));


	vec3 dPdu = MinDiff(P, Pr, Pl);
    vec3 dPdv = MinDiff(P, Pt, Pb) * (AORes.y * InvAORes.x);

    vec3 random = texture(texNoise, vTexCoords.xy * noiseScale).rgb;

    vec2 rayRadiusUV = 0.5 * R * FocalLen / -P.z;
    float rayRadiusPix = rayRadiusUV.x * AORes.x;

    float ao = 1.0;

    if(rayRadiusPix > 1.0f)
    {
    	ao = 0.0;
    	float numSteps;
    	vec2 stepSizeUV;

    	// Compute the number of steps
    	ComputeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z);

		float alpha = 2.0 * PI / numDirections;

		// Calculate the horizon occlusion of each direction
		for(float d = 0; d < numDirections; ++d)
		{
			float theta = alpha * d;

			// Apply noise to the direction
			vec2 dir = RotateDirections(vec2(cos(theta), sin(theta)), random.xy);
			vec2 deltaUV = dir * stepSizeUV;

			// Sample the pixels along the direction
			ao += HorizonOcclusion(	deltaUV,
									P,
									dPdu,
									dPdv,
									random.z,
									numSteps);
		}

		// Average the results and produce the final AO
		ao = 1.0 - ao / numDirections * AOStrength;
		//ao = 0.0;
		
	}

	//out_frag0 = vec2(ao, 30.0 * P.z);
	//FragColor = ao;
	//FragColor = texture(gPositionDepth, vTexCoords).xy;
	FragColor = vec2(ao, 30.0 * P.z);
}








