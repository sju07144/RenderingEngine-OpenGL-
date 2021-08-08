#version 330 core
out vec4 fragColor;

in VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec3 viewDir;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
} fs_in;

struct Material
{
	float shininess;
};

struct Light
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;
uniform Material material;

vec3 CalculateDirectionalLight(vec3 normal, vec3 lightDir, vec3 viewDir);

void main()
{
	vec3 normal = normalize(fs_in.normal);
	vec3 lightDir = normalize(-light.direction);
	vec3 viewDir = normalize(fs_in.viewDir);
	vec3 color;

	color = CalculateDirectionalLight(normal, lightDir, viewDir);
	fragColor = vec4(color, 1.0);	
}
vec3 CalculateDirectionalLight(vec3 normal, vec3 lightDir, vec3 viewDir)
{
	vec3 diffuseColor = fs_in.diffuseColor;
	vec3 specularColor = fs_in.specularColor;
	
	vec3 ambient = light.ambient * diffuseColor;
	
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * diffuseColor; 

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * specularColor;

	return (ambient + diffuse + specular);
}