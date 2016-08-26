#version 400 core

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





uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;


uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

in vec3 gPosition;
in vec3 gNormal;
in vec2 gTexCoord;
in vec3 gPatchDistance;
in vec3 gTriDistance;

in vec3 tePosition;
in vec3 teNormal;
in vec2 teTexCoord;

//in vec3 tePatchDistance;
//in vec3 teTriDistance;



//in vec2 vTexCoord;

out vec4 color;


float amplify(float d, float scale, float offset)
{
	d = scale * d + offset;
	d = clamp(d, 0, 1);
	d = 1 - exp2(-2*d*d);
	return d;
}

void main()
{

	//vec3 L = normalize(vec3(1.0));
	//edge
	//float d2 = min(min(tePatchDistance.x, tePatchDistance.y), tePatchDistance.z);
	//float d1 = min(min(teTriDistance.x, teTriDistance.y), teTriDistance.z);
	float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
	float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);



	//vec4 tex = texture(texture1, gTexCoord);
	

	//w/o gs

	//vec4 tex = texture(texture1, teTexCoord);
	

	//color = tex;

	/*
	if(gNormal.x <= 0.5)
	{
		color = vec4(0.5,0.5,0.5,1.0) ;
	}
	else
	{color = vec4(1.0,0.0,0.0,1.0) ;}
	*/

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

	vec3 result = vec3(0.6f); //
	//vec3 result = ambient + diffuse + specular;
	
	//color = vec4(result, 1.0f);
	


	color = vec4(result * amplify(d2, 100, -0.5) * amplify(d1, 40, -0.5), 1.0);
}