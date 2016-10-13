#version 440

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform mat4 projection;


uniform vec3 samples[64];

int kernelSize = 64;
float radius = 1.0;

const vec2 noiseScale = vec2(1920.0f/4.0f, 1080.0f/4.0f);
const float fov = 45.0;
const float PI = 3.14159265;

uniform vec2 UVToViewA;
uniform vec2 UVToViewB;





vec3 ReconstructNormal(vec2 UV, vec3 P)
  {
    return -normalize(cross(dFdy(P), dFdx(P)));
  }


void main()
{
	vec3 fragPos = texture(gPositionDepth, TexCoords).xyz;
	
	vec3 normal = texture(gNormal, TexCoords).rgb ;
	//vec3 normal = ReconstructNormal(TexCoords, fragPos);
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz ;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 samp = TBN * samples[i];
		samp = fragPos + samp * radius;

		vec4 offset = vec4(samp, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = -texture(gPositionDepth, offset.xy).w ;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

		occlusion += (sampleDepth >= samp.z ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;
	
	
}