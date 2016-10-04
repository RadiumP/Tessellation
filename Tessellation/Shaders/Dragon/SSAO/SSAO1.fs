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

vec3 getViewRay(vec2 tc) {
		// float hfar = 2.0 * tan(fov/2.0) * far;
		// float wfar = hfar * aspectRatio;    
		// vec3 ray = vec3(wfar * (tc.x - 0.5), hfar * (tc.y - 0.5), -far);    
		// return ray;  

		float hfar = 2.0 * tan(fov * PI / 180.0 /2.0) ;
		float wfar = hfar * 1920.0 / 1080.0;    
		vec3 ray = vec3(wfar * (tc.x - 0.5), hfar * (tc.y - 0.5), -1.0);    
		return ray;                     
	}       

void main(){
	vec2 screenPos = vec2(gl_FragCoord.x / 1920.0, gl_FragCoord.y / 1080.0);
	screenPos = TexCoords;
	float linearDepth = -texture(gPositionDepth,screenPos).w;


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
		    float sampleDepth = -Sample.z;// / far;
			float depthBufferValue = -texture(gPositionDepth,offset.xy).w;				              
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