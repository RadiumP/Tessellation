#version 440

	#extension GL_OES_standard_derivatives : enable

	#define NUM_SAMPLE_DIRECTIONS  8 

	// number of sample steps when raymarching along a direction
	#define NUM_SAMPLE_STEPS       4 
	#define PI 3.14159265

	uniform sampler2D gPositionDepth;
	uniform sampler2D gNormal;//reconstruct
	uniform sampler2D texNoise;

	uniform mat4 view;
 
	uniform vec2 FocalLen;
	uniform vec2 UVToViewA;
	uniform vec2 UVToViewB;

	uniform vec2 LinMAD;


	const float fov = 45.0f;
	const vec2 noiseScale = vec2(1920.0f/4.0f, 1080.0f/4.0f);

	//uniform float near;
	const float far = 2000.0f;            
	
	const float aspectRatio = 1920.0f / 1080.0f;    
	//uniform vec3 kernel[16];  

	//modi
	const float uAngleBias = 0.1;
	const float uIntensity = 1.2;
	
	const vec2 InvAORes = vec2(1.0f/1920.0f, 1.0f/1080.0f);
	


in vec2 vTexCoords;
//in vec2 vPosition;

//out float FragColor;
out vec2 FragColor;

	//const float PI = 3.14159265;
	

	


vec3 UVToViewSpace(vec2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;

	return (view * vec4(uv *z, z, 1.0)).xyz;
}

vec3 GetViewPos(vec2 uv)
{
	//z/(w+1)/2 ?
	//float z = ViewSpaceZFromDepth(texture(gPositionDepth, uv).z/(texture(gPositionDepth, uv).w + 1.0f) / 2.0f);
	//float z = ViewSpaceZFromDepth(-texture(gPositionDepth, uv).w );
	//float z = -texture(gPositionDepth, uv).w ; 
	
	//return UVToViewSpace(uv, z);
	return (view * vec4(texture(gPositionDepth, uv).xyz ,1.0)).xyz; 
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
  vec3 V1 = Pr - P;
  vec3 V2 = P - Pl;
  return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 ReconstructNormal(vec2 UV, vec3 P)
{
  vec3 Pr = GetViewPos(UV + vec2(InvAORes.x, 0));
  vec3 Pl = GetViewPos(UV + vec2(-InvAORes.x, 0));
  vec3 Pt = GetViewPos(UV + vec2(0, InvAORes.y));
  vec3 Pb = GetViewPos(UV + vec2(0, -InvAORes.y));
  return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}
  



	void main()
	{	
		//vec2 screenPos = vec2(gl_FragCoord.x / 1920.0, gl_FragCoord.y / 1080.0);
		vec2 screenPos = vTexCoords;
		const float TWO_PI = 2.0 * PI;
		//vec3 originVS = getPositionViewSpace(screenPos);
		vec3 originVS = GetViewPos(screenPos); 

		
		//vec3 normal = normalize(vNormal);

		 vec3 normal = texture(gNormal, screenPos).xyz;
		float radiusSS = 0.0; // radius of influence in screen space
		//float radiusWS = 0.0; // radius of influence in world space
		  
		radiusSS = 1.0;//sample radius

		 
	  	
	    const float theta = TWO_PI / float(NUM_SAMPLE_DIRECTIONS);
	    float cosTheta = cos(theta);
	    float sinTheta = sin(theta);
	    // matrix to create the sample directions
  		mat2 deltaRotationMatrix = mat2(cosTheta, -sinTheta, sinTheta, cosTheta);
  
  		// step vector in view space
  		vec2 deltaUV = vec2(1.0, 0.0) * (radiusSS / (float(NUM_SAMPLE_DIRECTIONS * NUM_SAMPLE_STEPS) + 1.0));
  
  
  		vec3 sampleNoise    = texture(texNoise, screenPos * noiseScale).xyz;
  		sampleNoise.xy      = sampleNoise.xy * 2.0 - vec2(1.0);
  		mat2 rotationMatrix = mat2(sampleNoise.x, -sampleNoise.y, 
                             sampleNoise.y,  sampleNoise.x);
	  
	  
  		deltaUV = rotationMatrix * deltaUV;
  
  		float jitter = sampleNoise.z;
  		float occlusion = 0.0;
	  		
  		for (int i = 0; i < NUM_SAMPLE_DIRECTIONS; ++i) {
    
 		   	deltaUV = deltaRotationMatrix * deltaUV;
    
   		 	vec2 sampleDirUV = deltaUV;
   		 	float oldAngle   = uAngleBias;
    
   		 	for (int j = 0; j < NUM_SAMPLE_STEPS; ++j) {
      			vec2 sampleUV     = screenPos + (jitter + float(j)) * sampleDirUV;
      			vec3 sampleVS     = GetViewPos(sampleUV);
      			vec3 sampleDirVS  = (sampleVS - originVS);
      
      // angle between fragment tangent and the sample
      			float gamma = (PI / 2.0) - acos(dot(normal, normalize(sampleDirVS)));
      
      			if (gamma > oldAngle) {
        			float value = sin(gamma) - sin(oldAngle);
        			occlusion += value;
        			oldAngle = gamma;
        			//gl_FragColor = vec4(0.5, occlusion, occlusion, 1.0);
        		}
        	}
  		}
	  
	  	occlusion = 1.0 - occlusion / float(NUM_SAMPLE_DIRECTIONS * NUM_SAMPLE_STEPS);
	 	occlusion = clamp(pow(occlusion, 1.0 + uIntensity), 0.0, 1.0);
	  	//gl_FragColor = vec4(occlusion, occlusion, occlusion, 1.0);
	  	FragColor = vec2(occlusion);
		
  
	}

