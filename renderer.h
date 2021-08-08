#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "model.h"
#include "shader.h"

class Renderer
{
public:
	Renderer();
	Renderer(const Renderer&) = delete;
	~Renderer();

	Camera* GetCamera();
	void Render(float aspect);
private:
	// Shader 변수
	Shader* currentShader;

	// matrix 변수
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;

	// Camera 변수
	Camera* camera;

	// model 변수
	Model* object;

	// Light Direction 변수
	glm::vec3 lightDir;

	void SetMatrix(float aspect); // Parameter: float aspect => aspect를 window에서 가져옴. => 일반화??
	void SetUniformVariables();
};