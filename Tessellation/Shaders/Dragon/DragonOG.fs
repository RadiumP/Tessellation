#version 440 core

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

in vec3 tePosition;
in vec3 teNormal;
in vec2 teTexCoord;

out vec4 color;

void main()
{
	

	

	//lighting
	
	vec3 ambient = light.ambient * material.ambient;

	vec3 norm = normalize(teNormal);
	vec3 lightDir = normalize(light.position - tePosition);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * material.diffuse);

	vec3 viewDir = normalize(viewPos - tePosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular);

	vec3 result = ambient + diffuse + specular;
	
	color = vec4(result, 1.0f);
}
