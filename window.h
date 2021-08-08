#pragma once
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <iostream>
#include "renderer.h"
#include "texture.h"

class Window
{
public:
	Window(unsigned int width, unsigned int height, const char* windowTitle);
	Window(const Window&) = delete;
	~Window();

	void Initialize();
	void Run();
	void Shutdown();

	unsigned int GetWidth();
	unsigned int GetHeight();

	void FramebufferSize(GLFWwindow* window, int width, int height);
	void Mouse(GLFWwindow* window, double xPos, double yPos);
	void Scroll(GLFWwindow* window, double xOffset, double yOffset);
private:
	// window 변수
	GLFWwindow* window;
	const char* windowTitle;
	GLuint width;
	GLuint height;

	// mouse 변수
	float lastX;
	float lastY;
	bool firstMouse;

	// frame 변수
	float deltaTime;
	float lastFrame;

	// Renderer 변수
	Renderer* renderer;

	bool GLFWInitialize();
	bool CreateWindow();
	bool GLEWInitialize(); // If you call this method, you must call GLFWInitialize before.
	void ProcessInput();
};

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
static void MouseCallback(GLFWwindow* window, double xPos, double yPos);
static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

static Window* windowHandle = nullptr;