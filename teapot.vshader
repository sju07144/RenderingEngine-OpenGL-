#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

layout(std140) uniform Mat
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec3 viewDir;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPos;

void main()
{
	vec4 fragPos = model * vec4(aPos, 1.0);
	vs_out.fragPos = fragPos.xyz;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vs_out.normal = normalMatrix * aNormal;
	vs_out.texCoords = aTexCoords;
	vs_out.viewDir = viewPos - vs_out.fragPos;

	vs_out.ambientColor = ambient;
	vs_out.diffuseColor = diffuse;
	vs_out.specularColor = specular;

	gl_Position = projection * view * fragPos;
}