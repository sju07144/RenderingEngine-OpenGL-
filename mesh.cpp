#include "mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<std::array<unsigned int, 3>> faces, std::vector<unsigned int> indices, 
	std::vector<std::vector<unsigned int>> adjacentFaces, std::vector<Texture> textures, Material mat)
{
	// geometry primitive 초기화
	this->vertices = vertices;
	this->faces = faces;
	this->indices = indices;
	this->adjacentFaces = adjacentFaces;
	this->textures = textures;
	this->mat = mat;

	int nv = vertices.size();

	// principal curvature, derivative of principal curvature 계산
	CalculatePointAreas();
	CalculatePrincipalCurvatures();
	CalculateDerivativeCurvature();

	// adjacent face, count texture 초기화
	adjacentFaceCountID = CreateAdjacentFaceCountTexture();
	// adjacentFaceID = CreateAdjacentFaceTexture();
	SetupMesh();
}
void Mesh::Draw(const Shader& shader)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;

	auto textureSize = textures.size();
	for (auto i = 0; i != textureSize; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		std::string name = textures[i].GetType();
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);

		shader.SetInt(name + number, i);
		glBindTexture(GL_TEXTURE_2D, textures[i].GetTextureID());
	}

	shader.SetUniformBlockBinding("Mat", 0);
	glBindVertexArray(vertexArrayID);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr); // simple rendering: GL_TRIANGLES, silhouette: GL_TRIANGLES_ADJACENCY
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}
void Mesh::SetupMesh()
{
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &elementBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uniformBlockIndexID);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBlockIndexID);
	glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::vec3), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformBlockIndexID, 0, 3 * sizeof(glm::vec3));

	glBindBuffer(GL_UNIFORM_BUFFER, uniformBlockIndexID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(mat.ka));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBlockIndexID);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec3), sizeof(glm::vec3), glm::value_ptr(mat.kd));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBlockIndexID);
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::vec3), sizeof(glm::vec3), glm::value_ptr(mat.ks));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pdir1));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pdir2));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, curv1));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, curv2));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, dcurv));

	glBindVertexArray(0);
}
void Mesh::CalculatePointAreas()
{
	int nf = faces.size(), nv = vertices.size();
	std::array<unsigned int, 3> face;
	cornerAreas.reserve(nf);

	for (int i = 0; i < nf; i++)
	{
		// Edges
		face = faces[i];
		glm::vec3 e[3] = { vertices[face[2]].position - vertices[face[1]].position,
			vertices[face[0]].position - vertices[face[2]].position,
			vertices[face[1]].position - vertices[face[0]].position };

		// Compute corner weights
		float area = 0.5f * glm::length(glm::cross(e[0], e[1]));
		float l2[3] = { glm::length2(e[0]), glm::length2(e[1]), glm::length2(e[2]) };

		// Barycentric weights of circumcenter
		float bcw[3] = { l2[0] * (l2[1] + l2[2] - l2[0]),
						 l2[1] * (l2[2] + l2[0] - l2[1]),
						 l2[2] * (l2[0] + l2[1] - l2[2]) };
		glm::vec3 cornerArea;
		if (bcw[0] <= 0.0f)
		{
			cornerArea.y = -0.25f * l2[2] * area /
				glm::dot(e[0], e[2]);
			cornerArea.z = -0.25f * l2[1] * area /
				glm::dot(e[0], e[1]);
			cornerArea.x = area - cornerArea.y - cornerArea.z;
		}
		else if (bcw[1] <= 0.0f)
		{
			cornerArea.z = -0.25f * l2[0] * area /
				glm::dot(e[1], e[0]);
			cornerArea.x = -0.25f * l2[2] * area /
				glm::dot(e[1], e[2]);
			cornerArea.y = area - cornerArea.z - cornerArea.x;
		}
		else if (bcw[2] <= 0.0f)
		{
			cornerArea.x = -0.25f * l2[1] * area /
				glm::dot(e[2], e[1]);
			cornerArea.y = -0.25f * l2[0] * area /
				glm::dot(e[2], e[0]);
			cornerArea.z = area - cornerArea.x - cornerArea.y;
		}
		else
		{
			float scale = 0.5f * area / (bcw[0] + bcw[1] + bcw[2]);
			cornerArea.x = scale * (bcw[1] + bcw[2]);
			cornerArea.y = scale * (bcw[2] + bcw[0]);
			cornerArea.z = scale * (bcw[0] + bcw[1]);
		}
		cornerAreas.push_back(cornerArea);
	}
	for (int i = 0; i < nf; i++)
	{
		face = faces[i];
		vertices[face[0]].pointArea += cornerAreas[i].x;
		vertices[face[1]].pointArea += cornerAreas[i].y;
		vertices[face[2]].pointArea += cornerAreas[i].z;
	}
}
void Mesh::CalculatePrincipalCurvatures()
{
	// Resize the arrays we'll be using
	int nv = vertices.size(), nf = faces.size();
	std::vector<float> curv12;
	curv12.reserve(nv);
	std::array<unsigned int, 3> face;

	for (int i = 0; i < nv; i++)
	{
		curv12.push_back(0.0f);
	}

	// Set up an initial coordinate system per vertex
	for (int i = 0; i < nf; i++) 
	{
		face = faces[i];
		vertices[face[0]].pdir1 = vertices[face[1]].position - vertices[face[0]].position;
		vertices[face[1]].pdir1 = vertices[face[2]].position - vertices[face[1]].position;
		vertices[face[2]].pdir1 = vertices[face[0]].position - vertices[face[2]].position;
	}

	for (int i = 0; i < nv; i++)
	{
		vertices[i].pdir1 = glm::cross(vertices[i].pdir1, vertices[i].normal);
		vertices[i].pdir1 = glm::normalize(vertices[i].pdir1);
		vertices[i].pdir2 = glm::cross(vertices[i].normal, vertices[i].pdir1);
	}

	// Compute curvature per-face
	for (int i = 0; i < nf; i++) 
	{
		// Edges
		face = faces[i];
		glm::vec3 e[3] = { vertices[face[2]].position - vertices[face[1]].position,
			vertices[face[0]].position - vertices[face[2]].position,
			vertices[face[1]].position - vertices[face[0]].position };

		// N-T-B coordinate system per face
		glm::vec3 t = e[0];
		t = glm::normalize(t);
		glm::vec3 n = glm::cross(e[0], e[1]);
		glm::vec3 b = glm::cross(n, t);
		b = glm::normalize(b);

		// Estimate curvature based on variation of normals
		// along edges
		float m[3] = { 0, 0, 0 };
		float w[3][3] = { {0,0,0}, {0,0,0}, {0,0,0} };
		for (int j = 0; j < 3; j++)
		{
			float u = glm::dot(e[j], t);
			float v = glm::dot(e[j], b);
			w[0][0] += u * u;
			w[0][1] += u * v;
			w[2][2] += v * v;
			// The below are computed once at the end of the loop
			// w[1][1] += v*v + u*u;
			// w[1][2] += u*v;
			unsigned int faceAddress0 = static_cast<unsigned int>((j + 2) % 3);
			unsigned int faceAddress1 = static_cast<unsigned int>((j + 1) % 3);
			glm::vec3 dn = vertices[face[faceAddress0]].normal - vertices[face[faceAddress1]].normal; // PREV_MOD3: (j - 1) % 3, NEXT_MOD3: (j + 1) % 3
			float dnu = glm::dot(dn, t);
			float dnv = glm::dot(dn, b);
			m[0] += dnu * u;
			m[1] += dnu * v + dnv * u;
			m[2] += dnv * v;
		}
		w[1][1] = w[0][0] + w[2][2];
		w[1][2] = w[0][1];

		// Least squares solution
		float diag[3];
		if (!ldltdc<float, 3>(w, diag)) 
		{
			//dprintf("ldltdc failed!\n");
			continue;
		}
		ldltsl<float, 3>(w, diag, m, m);

		// Push it back out to the vertices
		for (int j = 0; j < 3; j++)
		{
			int vj = face[j];
			float c1, c12, c2;
			proj_curv(t, b, m[0], m[1], m[2],
				vertices[vj].pdir1, vertices[vj].pdir2, c1, c12, c2);
			float wt = cornerAreas[i][j] / vertices[vj].pointArea;
			vertices[vj].curv1 += wt * c1;
			curv12[vj] += wt * c12;
			vertices[vj].curv2 += wt * c2;
		}
	}

	// Compute principal directions and curvatures at each vertex
	for (int i = 0; i < nv; i++)
	{
		diagonalize_curv(vertices[i].pdir1, vertices[i].pdir2,
			vertices[i].curv1, curv12[i], vertices[i].curv2,
			vertices[i].normal, vertices[i].pdir1, vertices[i].pdir2,
			vertices[i].curv1, vertices[i].curv2);
	} 
}

void Mesh::CalculateDerivativeCurvature()
{
	CalculatePrincipalCurvatures();

	// Resize the arrays we'll be using
	int nv = vertices.size(), nf = faces.size();
	std::array<unsigned int, 3> face;

	// Compute dcurv per-face
	for (int i = 0; i < nf; i++)
	{
		face = faces[i];
		// Edges
		glm::vec3 e[3] = { vertices[face[2]].position - vertices[face[1]].position,
			vertices[face[0]].position - vertices[face[2]].position,
			vertices[face[1]].position - vertices[face[0]].position };

		// N-T-B coordinate system per face
		glm::vec3 t = e[0];
		t = glm::normalize(t);
		glm::vec3 n = glm::cross(e[0], e[1]);
		glm::vec3 b = glm::cross(n, t);
		b = glm::normalize(b);

		// Project curvature tensor from each vertex into this
		// face's coordinate system
		glm::vec3 fcurv[3];
		for (int j = 0; j < 3; j++)
		{
			int vj = face[j];
			proj_curv(vertices[vj].pdir1, vertices[vj].pdir2, vertices[vj].curv1, 0, vertices[vj].curv2,
				t, b, fcurv[j].x, fcurv[j].y, fcurv[j].z);
		}

		// Estimate dcurv based on variation of curvature along edges
		float m[4] = { 0, 0, 0, 0 };
		float w[4][4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };
		for (int j = 0; j < 3; j++)
		{
			// Variation of curvature along each edge
			unsigned int faceAddress0 = static_cast<unsigned int>((j + 2) % 3);
			unsigned int faceAddress1 = static_cast<unsigned int>((j + 1) % 3);
			glm::vec3 dfcurv = fcurv[faceAddress0] - fcurv[faceAddress1];
			float u = glm::dot(e[j], t);
			float v = glm::dot(e[j], b);
			// std::cout << u << ' ' << v << '\n';
			float u2 = u * u, v2 = v * v, uv = u * v;
			w[0][0] += u2;
			w[0][1] += uv;
			w[3][3] += v2;
			// All the below are computed at the end of the loop
			// w[1][1] += 2.0f*u2 + v2;
			// w[1][2] += 2.0f*uv;
			// w[2][2] += u2 + 2.0f*v2;
			// w[2][3] += uv;
			m[0] += u * dfcurv.x;
			m[1] += v * dfcurv.x + 2.0f * u * dfcurv.y;
			m[2] += 2.0f * v * dfcurv.y + u * dfcurv.z;
			m[3] += v * dfcurv.z;
		}
		w[1][1] = 2.0f * w[0][0] + w[3][3];
		w[1][2] = 2.0f * w[0][1];
		w[2][2] = w[0][0] + 2.0f * w[3][3];
		w[2][3] = w[0][1];

		// Least squares solution
		float d[4];
		if (!ldltdc<float, 4>(w, d))
		{
			//dprintf("ldltdc failed!\n");
			continue;
		}
		ldltsl<float, 4>(w, d, m, m);

		glm::vec4 face_dcurv = glm::vec4(m[0], m[1], m[2], m[3]);

		// Push it back out to each vertex
		for (int j = 0; j < 3; j++)
		{
			int vj = face[j];
			glm::vec4 this_vert_dcurv;
			proj_dcurv(t, b, face_dcurv,
				vertices[vj].pdir1, vertices[vj].pdir2, this_vert_dcurv);
			float wt = cornerAreas[i][j] / vertices[vj].pointArea;
			vertices[vj].dcurv += wt * this_vert_dcurv;
		}
	}
}

static void rot_coord_sys(const glm::vec3& old_u, const glm::vec3& old_v,
	const glm::vec3& new_norm,
	glm::vec3& new_u, glm::vec3& new_v)
{
	new_u = old_u;
	new_v = old_v;
	glm::vec3 old_norm = glm::cross(old_u, old_v);
	float ndot = glm::dot(old_norm, new_norm);
	if (ndot <= -1.0f)
	{
		new_u = -new_u;
		new_v = -new_v;
		return;
	}

	// Perpendicular to old_norm and in the plane of old_norm and new_norm
	glm::vec3 perp_old = new_norm - ndot * old_norm;

	// Perpendicular to new_norm and in the plane of old_norm and new_norm
	// vec perp_new = ndot * new_norm - old_norm;

	// perp_old - perp_new, with normalization constants folded in
	glm::vec3 dperp = 1.0f / (1 + ndot) * (old_norm + new_norm);

	// Subtracts component along perp_old, and adds the same amount along
	// perp_new.  Leaves unchanged the component perpendicular to the
	// plane containing old_norm and new_norm.
	new_u -= dperp * glm::dot(new_u, perp_old);
	new_v -= dperp * glm::dot(new_v, perp_old);
}

void proj_curv(const glm::vec3& old_u, const glm::vec3& old_v,
	float old_ku, float old_kuv, float old_kv,
	const glm::vec3& new_u, const glm::vec3& new_v,
	float& new_ku, float& new_kuv, float& new_kv)
{
	glm::vec3 r_new_u, r_new_v;
	rot_coord_sys(new_u, new_v, glm::cross(old_u, old_v), r_new_u, r_new_v);

	float u1 = glm::dot(r_new_u, old_u);
	float v1 = glm::dot(r_new_u, old_v);
	float u2 = glm::dot(r_new_v, old_u);
	float v2 = glm::dot(r_new_v, old_v);

	new_ku = old_ku * u1 * u1 + old_kuv * (2.0f * u1 * v1) + old_kv * v1 * v1;
	new_kuv = old_ku * u1 * u2 + old_kuv * (u1 * v2 + u2 * v1) + old_kv * v1 * v2;
	new_kv = old_ku * u2 * u2 + old_kuv * (2.0f * u2 * v2) + old_kv * v2 * v2;
}

void proj_dcurv(const glm::vec3& old_u, const glm::vec3& old_v,
	const glm::vec4& old_dcurv,
	const glm::vec3& new_u, const glm::vec3& new_v,
	glm::vec4& new_dcurv)
{
	glm::vec3 r_new_u, r_new_v;
	rot_coord_sys(new_u, new_v, glm::cross(old_u, old_v), r_new_u, r_new_v);

	float u1 = glm::dot(r_new_u, old_u);
	float v1 = glm::dot(r_new_u, old_v);
	float u2 = glm::dot(r_new_v, old_u);
	float v2 = glm::dot(r_new_v, old_v);

	new_dcurv.x = old_dcurv.x * u1 * u1 * u1 +
		old_dcurv.y * 3.0f * u1 * u1 * v1 +
		old_dcurv.z * 3.0f * u1 * v1 * v1 +
		old_dcurv.w * v1 * v1 * v1;
	new_dcurv.y = old_dcurv.x * u1 * u1 * u2 +
		old_dcurv.y * (u1 * u1 * v2 + 2.0f * u2 * u1 * v1) +
		old_dcurv.z * (u2 * v1 * v1 + 2.0f * u1 * v1 * v2) +
		old_dcurv.w * v1 * v1 * v2;
	new_dcurv.z = old_dcurv.x * u1 * u2 * u2 +
		old_dcurv.y * (u2 * u2 * v1 + 2.0f * u1 * u2 * v2) +
		old_dcurv.z * (u1 * v2 * v2 + 2.0f * u2 * v2 * v1) +
		old_dcurv.w * v1 * v2 * v2;
	new_dcurv.w = old_dcurv.x * u2 * u2 * u2 +
		old_dcurv.y * 3.0f * u2 * u2 * v2 +
		old_dcurv.z * 3.0f * u2 * v2 * v2 +
		old_dcurv.w * v2 * v2 * v2;
}

void diagonalize_curv(const glm::vec3& old_u, const glm::vec3& old_v,
	float ku, float kuv, float kv,
	const glm::vec3& new_norm,
	glm::vec3& pdir1, glm::vec3& pdir2, float& k1, float& k2)
{
	glm::vec3 r_old_u, r_old_v;
	rot_coord_sys(old_u, old_v, new_norm, r_old_u, r_old_v);

	float c = 1, s = 0, tt = 0;
	if (kuv != 0.0f)
	{
		// Jacobi rotation to diagonalize
		float h = 0.5f * (kv - ku) / kuv;
		tt = (h < 0.0f) ?
			1.0f / (h - sqrt(1.0f + h * h)) :
			1.0f / (h + sqrt(1.0f + h * h));
		c = 1.0f / sqrt(1.0f + tt * tt);
		s = tt * c;
	}

	k1 = ku - tt * kuv;
	k2 = kv + tt * kuv;

	if (fabs(k1) >= fabs(k2))
	{
		pdir1 = c * r_old_u - s * r_old_v;
	}
	else
	{
		std::swap(k1, k2);
		pdir1 = s * r_old_u + c * r_old_v;
	}
	pdir2 = glm::cross(new_norm, pdir1);
}

template <class T, int N>
static inline bool ldltdc(T(&A)[N][N], T rdiag[N])
{
	// Special case for small N
	if (N < 1) {
		return false;
	}
	else if (N <= 3) {
		T d0 = A[0][0];
		rdiag[0] = 1 / d0;
		if (N == 1)
			return (d0 != 0);
		A[1][0] = A[0][1];
		T l10 = rdiag[0] * A[1][0];
		T d1 = A[1][1] - l10 * A[1][0];
		rdiag[1] = 1 / d1;
		if (N == 2)
			return (d0 != 0 && d1 != 0);
		T d2 = A[2][2] - rdiag[0] * glm::pow(A[2][0], 2.0) - rdiag[1] * glm::pow(A[2][1], 2.0);
		rdiag[2] = 1 / d2;
		A[2][0] = A[0][2];
		A[2][1] = A[1][2] - l10 * A[2][0];
		return (d0 != 0 && d1 != 0 && d2 != 0);
	}

	T v[N - 1];
	for (int i = 0; i < N; i++) {
		for (int k = 0; k < i; k++)
			v[k] = A[i][k] * rdiag[k];
		for (int j = i; j < N; j++) {
			T sum = A[i][j];
			for (int k = 0; k < i; k++)
				sum -= v[k] * A[j][k];
			if (i == j) {
				if (sum == 0)
					return false;
				rdiag[i] = 1 / sum;
			}
			else {
				A[j][i] = sum;
			}
		}
	}

	return true;
}

template <class T, int N>
static inline void ldltsl(const T(&A)[N][N],
	const T rdiag[N],
	const T b[N],
	T x[N])
{
	for (int i = 0; i < N; i++) {
		T sum = b[i];
		for (int k = 0; k < i; k++)
			sum -= A[i][k] * x[k];
		x[i] = sum * rdiag[i];
	}
	for (int i = N - 1; i >= 0; i--) {
		T sum = 0;
		for (int k = i + 1; k < N; k++)
			sum += A[k][i] * x[k];
		x[i] -= sum * rdiag[i];
	}
}

GLuint Mesh::CreateAdjacentFaceCountTexture()
{
	unsigned char* adjacentFaceCounts = new unsigned char[256 * 256];
	int verticesCount = vertices.size();
	for (int i = 0; i < verticesCount; i++)
	{
		adjacentFaceCounts[i] = adjacentFaces[i].size();
	}
	for (int i = verticesCount; i < 256 * 256; i++)
	{
		adjacentFaceCounts[i] = 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, adjacentFaceCounts);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	delete[] adjacentFaceCounts;

	return textureID;
}