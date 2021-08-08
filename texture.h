#pragma once
#include <fstream>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <iostream>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string>
#include <vector>

class Texture
{
public:
	Texture();
	~Texture();

	GLuint GetTextureID();
	const std::string& GetPath();
	const std::string& GetType();

	void LoadTexture(const std::string& path, const std::string& typeName, bool gammaCorrection = false);
	void LoadTextureUsingDirectory(const std::string& path, const std::string& directory, const std::string& typeName, bool gamma = false);
	void LoadCubemap(const std::vector<std::string>& faces);
private:
	GLuint textureID;
	std::string type;
	std::string path;
};

bool SaveScreenshot(const std::string& fileName);
bool SaveScreenshotWithBMP(const std::string& fileName);