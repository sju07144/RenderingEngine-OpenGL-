#include "model.h"

void Model::Draw(const Shader& shader)
{
	for (auto& i : meshes)
		i.Draw(shader);
}
void Model::LoadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals); // aiProcess_FlipUVs if need
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}
void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (auto i = 0; i != node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (auto i = 0; i != node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}
Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumVertices);

	for (auto i = 0; i != mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		
		glm::vec3 vector;
		std::array<float, 3> position;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
		}
		else
			vertex.texCoords = glm::vec2(0.0f, 0.0f);

		vertex.cornerArea = glm::vec3(0.0f, 0.0f, 0.0f);
		vertex.pointArea = 0.0f;

		vertex.pdir1 = glm::vec3(0.0f, 0.0f, 0.0f);
		vertex.pdir2 = glm::vec3(0.0f, 0.0f, 0.0f);
		vertex.curv1 = 0.0f;
		vertex.curv2 = 0.0f;
		vertex.dcurv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

		vertex.q1 = 0.0f;
		vertex.t1 = glm::vec2(0.0f, 0.0f);
		vertex.dt1q1 = 0.0f;

		vertices.push_back(vertex);
	}

	/* for (auto i = 0; i != mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		for (auto j = 0; j != face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	} */

	std::vector<std::array<unsigned int, 3>> faces;
	faces.reserve(mesh->mNumFaces);
	for (auto i = 0; i != mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		std::array<unsigned int, 3> faceIndex;

		for (auto j = 0; j != face.mNumIndices; ++j)
		{
			faceIndex[j] = face.mIndices[j];
			indices.push_back(face.mIndices[j]);
		}
		faces.push_back(faceIndex);
	}

	std::vector<std::vector<unsigned int>> adjacentFaces;
	int nv = vertices.size(), nf = faces.size();
	std::vector<unsigned int> numAdjacentFaces(nv);
	std::array<unsigned int, 3> face;
	for (int i = 0; i < nf; i++)
	{
		face = faces[i];
		numAdjacentFaces[face[0]]++;
		numAdjacentFaces[face[1]]++;
		numAdjacentFaces[face[2]]++;
	}

	adjacentFaces.resize(vertices.size());
	for (int i = 0; i < nv; i++)
	{
		adjacentFaces[i].reserve(numAdjacentFaces[i]);
	}

	for (int i = 0; i < nf; i++)
	{
		face = faces[i];
		for (int j = 0; j < 3; j++)
		{
			adjacentFaces[face[j]].push_back(i);
		}
	}

	Material mat;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiColor3D color;

		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		mat.ka = glm::vec3(color.r, color.g, color.b);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		mat.kd = glm::vec3(color.r, color.g, color.b);
		material->Get(AI_MATKEY_COLOR_SPECULAR, color);
		mat.ks = glm::vec3(color.r, color.g, color.b);

		std::vector<Texture> diffusemaps;
		LoadMaterialTextures(diffusemaps, material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffusemaps.begin(), diffusemaps.end());
		std::vector<Texture> specularmaps;
		LoadMaterialTextures(specularmaps, material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularmaps.begin(), specularmaps.end());
	}
	else
	{
		mat.ka = glm::vec3(0.0f, 0.0f, 0.0f);
		mat.kd = glm::vec3(1.0f, 1.0f, 0.0f);
		mat.ks = glm::vec3(0.4f, 0.4f, 0.0f);
	}

	return Mesh{ vertices, faces, indices, adjacentFaces, textures, mat };
}
void Model::LoadMaterialTextures(std::vector<Texture>& textures, aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	for (auto i = 0; i != mat->GetTextureCount(type); ++i)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		bool skip = false;
		auto texturesLoadedSize = textures_loaded.size();
		for (auto j = 0; j != texturesLoadedSize; ++j)
		{
			if (std::strcmp(textures_loaded[j].GetPath().data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			Texture texture;
			texture.LoadTextureUsingDirectory(str.C_Str(), this->directory, typeName);
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
}