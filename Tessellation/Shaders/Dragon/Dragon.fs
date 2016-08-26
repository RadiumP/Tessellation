#version 440 

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;	
};

struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};




const float NEAR = 0.1f;
const float FAR = 100.0f;



uniform vec3 viewPos;
uniform Material material;
uniform Light light;


uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

in vec4 gPosition;
in vec3 gNormal;
in vec2 gTexCoord;
in vec3 gPatchDistance;
in vec3 gTriDistance;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

//in vec3 tePatchDistance;
//in vec3 teTriDistance;



//in vec2 vTexCoord;

out vec4 color;

layout(location = 0) out vec4 geoPositionDepth;
layout(location = 1) out vec3 geoNormal;
layout(location = 2) out vec4 geoAlbedoSpec;


float amplify(float d, float scale, float offset)
{
	d = scale * d + offset;
	d = clamp(d, 0, 1);
	d = 1 - exp2(-2*d*d);
	return d;
}




void main()
{

	
	//edge
	//float d2 = min(min(tePatchDistance.x, tePatchDistance.y), tePatchDistance.z);
	//float d1 = min(min(teTriDistance.x, teTriDistance.y), teTriDistance.z);
	float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
	float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);



	//vec4 tex = texture(texture1, gTexCoord);
	

	//w/o gs

	//vec4 tex = texture(texture1, teTexCoord);
	

	//color = tex;

	

	//lighting

	
	vec3 ambient = light.ambient * material.ambient;

	//vec3 norm = normalize(teNormal);
	vec3 norm = normalize(gNormal);
	
	//vec3 lightDir = normalize(light.position - tePosition);
	vec3 lightDir = normalize(light.position - gPosition.xyz);
	
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * material.diffuse);

	//vec3 viewDir = normalize(viewPos - tePosition);
	vec3 viewDir = normalize(viewPos - gPosition.xyz);

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular);

	//vec3 result = vec3(0.6f); //
	

	vec3 result = ambient + diffuse + specular;
	
	//color = vec4(result, 1.0f);
	

	/*
	if(gNormal.y <= 0.0)
	{
		result = vec3(0.5,0.5,0.5) ;
	}
	else
	{result = vec3(1.0,0.0,0.0) ;}
	*/
	//color = vec4(result * amplify(d2, 60, -0.5) * amplify(d1, 40, -0.5), 1.0);
	color = vec4(result, 1.0);

	
}