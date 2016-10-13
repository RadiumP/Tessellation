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

const float near = 1.0f; // Projection matrix's near plane distance
const float far = 2000.0f; // Projection matrix's far plane distance
const vec2 noiseScale = vec2(1920.0f/4.0f, 1080.0f/4.0f);
const float fov = 45.0;
const float PI = 3.14159265;

		float LinearizeDepth(float depth)
{
   // float z = depth * 2.0 - 1.0; // Back to NDC 
   // return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR)); ??
    float z = depth;// * 2.0 - 1.0; // Back to NDC 
    return (near * far) / (far  - z * (far - near));    	
}


vec3 getViewRay(vec2 tc) {
		 float hfar = 2.0 * tan(fov/2.0) * far;
		 float wfar = hfar * (1920.0 / 1080.0);    
		 vec3 ray = vec3(wfar * (tc.x - 0.5), hfar * (tc.y - 0.5), -far);    
		 return ray;  
              
	} 

	float unpackDepth(const in vec4 rgba_depth) {
		const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
		float depth = dot(rgba_depth, bit_shift);
		return depth;
	}    

	float getDepth(vec2 coord) {                          
		
		return unpackDepth(texture2D(gPositionDepth, coord.xy));
		
	}    
	      

void main(){
	vec2 screenPos = vec2(gl_FragCoord.x / 1920.0, gl_FragCoord.y / 1080.0);
	//screenPos = TexCoords;
	float linearDepth = LinearizeDepth(getDepth(screenPos));


	vec3 origin = getViewRay(screenPos) * linearDepth ;   
		
		vec3 normal = texture(gNormal, screenPos).xyz;   
				
		vec3 rvec = texture2D(texNoise, screenPos.xy * noiseScale).xyz * 2.0 - 1.0;
	    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
	    vec3 bitangent = cross(normal, tangent);
		mat3 tbn = mat3(tangent, bitangent, normal);        
		
		float occlusion = 0.0;
		for(int i = 0; i < kernelSize; ++i) {    	 
			vec3 Sample = origin + (tbn * samples[i]) * radius;
		    vec4 offset = projection * vec4(Sample, 1.0);		
			offset.xy /= offset.w;
			offset.xy = offset.xy * 0.5 + 0.5;        
		    float sampleDepth = -Sample.z / far;
			float depthBufferValue = LinearizeDepth(getDepth(offset.xy)) / far;				              
			float range_check = abs(linearDepth - depthBufferValue);
			if (range_check < radius && depthBufferValue <= sampleDepth) {
				occlusion +=  1.0;
			}
			
		}         
		   
		occlusion = 1.0 - occlusion / float(kernelSize);
                                   
		
	    //gl_FragColor.rgb = vec3((diffuse*0.2 + ambient*0.8) * occlusion);
		//gl_FragColor.rgb = vec3(occlusion);
	    //gl_FragColor.a = 1.0; 

	    FragColor = occlusion;
  

}