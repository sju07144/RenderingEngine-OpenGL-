#include "renderer.h"

Renderer::Renderer()
{
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	object = new Model();
	object->LoadModel("teapot/teapot.obj");
	currentShader = new Shader("teapot.vshader", "teapot.fshader");
	currentShader->BuildShader();
	lightDir = glm::vec3(1.0f, glm::sqrt(3.0f), -glm::sqrt(3.0f));
}
Renderer::~Renderer()
{
	delete currentShader;
	delete camera;
	delete object;
}
Camera* Renderer::GetCamera() { return camera; }
void Renderer::Render(float aspect)
{
	SetMatrix(aspect);
	SetUniformVariables();
	object->Draw(*currentShader);
}
void Renderer::SetMatrix(float aspect)
{
	float zoom = camera->zoom;
	projection = glm::perspective(camera->zoom, aspect, 0.1f, 100.0f);
	view = camera->GetViewMatrix();
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
}
void Renderer::SetUniformVariables()
{
	currentShader->Use();
	currentShader->SetMat4("projection", projection);
	currentShader->SetMat4("view", view);
	currentShader->SetMat4("model", model);
	currentShader->SetVec3("viewPos", camera->position);
	currentShader->SetVec3("light.direction", lightDir);
	currentShader->SetVec3("light.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
	currentShader->SetVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	currentShader->SetVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	currentShader->SetFloat("material.shininess", 32.0f);
}