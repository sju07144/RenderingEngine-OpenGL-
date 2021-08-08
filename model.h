#pragma once
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class Model
{
public:
	Model() = default;
	void LoadModel(const std::string& path);
	void Draw(const Shader& shader);
private:
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;

	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterialTextures(std::vector<Texture>& textures, aiMaterial* mat, aiTextureType type,
		const std::string& typeName);
};