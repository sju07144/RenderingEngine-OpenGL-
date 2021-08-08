#pragma once
#include <array>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <string>
#include <vector>
#include "shader.h"
#include "texture.h"

// vertex 요소(위치, normal vector, UV coordinates)
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	glm::vec3 cornerArea;
	float pointArea;

	glm::vec3 pdir1, pdir2;
	float curv1, curv2;
	glm::vec4 dcurv;

	float q1;
	glm::vec2 t1;
	float dt1q1;
};

// mtl파일에 쓰여있는 ka(ambient color), kd(diffuse color), ks(specular color)
struct Material
{
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
};

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<std::array<unsigned int, 3>> faces, std::vector<unsigned int> indices, 
		std::vector<std::vector<unsigned int>> adjacentFaces, std::vector<Texture> textures, Material mat);
	void Draw(const Shader& shader);
private:
	std::vector<Vertex> vertices; // vertex 집합
	std::vector<std::array<unsigned int, 3>> faces; // face 집합
	std::vector<std::vector<unsigned int>> adjacentFaces;
	std::vector<glm::vec3> cornerAreas;
	std::vector<unsigned int> indices; // index 집합
	std::vector<Texture> textures; // texture 집합
	Material mat; // mtl 파일 요소 집합
	
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint elementBufferID;
	GLuint uniformBlockIndexID;

	GLuint adjacentFaceCountID;
	GLuint adjacentFaceID;

	void SetupMesh(); // Mesh를 구성함
	void CalculatePointAreas();
	void CalculatePrincipalCurvatures(); // principal curvatures 계산
	void CalculateDerivativeCurvature();
	GLuint CreateAdjacentFaceCountTexture();
	// GLuint CreateAdjacentFaceTexture(); // adjacent face texture 생성
};

// Principal Curvatures

// Rotate a coordinate system to be perpendicular to the given normal
static void rot_coord_sys(const glm::vec3& old_u, const glm::vec3& old_v,
	const glm::vec3& new_norm,
	glm::vec3& new_u, glm::vec3& new_v);
// Reproject a curvature tensor from the basis spanned by old_u and old_v
// (which are assumed to be unit-length and perpendicular) to the
// new_u, new_v basis.
void proj_curv(const glm::vec3& old_u, const glm::vec3& old_v,
	float old_ku, float old_kuv, float old_kv,
	const glm::vec3& new_u, const glm::vec3& new_v,
	float& new_ku, float& new_kuv, float& new_kv);
// Like the above, but for dcurv
void proj_dcurv(const glm::vec3& old_u, const glm::vec3& old_v,
	const glm::vec4& old_dcurv,
	const glm::vec3& new_u, const glm::vec3& new_v,
	glm::vec4& new_dcurv);
// Given a curvature tensor, find principal directions and curvatures
// Makes sure that pdir1 and pdir2 are perpendicular to normal
void diagonalize_curv(const glm::vec3& old_u, const glm::vec3& old_v,
	float ku, float kuv, float kv,
	const glm::vec3& new_norm,
	glm::vec3& pdir1, glm::vec3& pdir2, float& k1, float& k2);

// LDL^T decomposition of a symmetric positive definite matrix (and some
// other symmetric matrices, but fragile since we don't do pivoting).
// Like Cholesky, but no square roots, which is important for small N.
// Reads diagonal and upper triangle of matrix A.
// On output, lower triangle of A holds LD, while rdiag holds D^-1.
// Algorithm from Golub and van Loan.
template <class T, int N>
static inline bool ldltdc(T(&A)[N][N], T rdiag[N]);

// Solve Ax=b after ldltdc.  x is allowed to be the same as b.
template <class T, int N>
static inline void ldltsl(const T(&A)[N][N],
	const T rdiag[N],
	const T b[N],
	T x[N]);