#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	programID = 0;
	this->vertexPath = vertexPath;
	this->fragmentPath = fragmentPath;
	this->geometryPath = geometryPath;

	this->vertex = 0;
	this->fragment = 0;
	this->geometry = 0;
}
Shader::~Shader()
{
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}
void Shader::BuildShader()
{
	CompileShader(vertexPath, vertex, GL_VERTEX_SHADER);
	CompileShader(fragmentPath, fragment, GL_FRAGMENT_SHADER);
	CompileShader(geometryPath, geometry, GL_GEOMETRY_SHADER);
	LinkProgram(vertex, fragment, geometry);
}
void Shader::CompileShader(const char* shaderPath, GLuint& shader, GLenum shaderType)
{
	if (shaderPath == nullptr)
	{
		return;
	}

	std::string shaderCodeString;
	std::ifstream shaderFile;

	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		shaderFile.open(shaderPath);
		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		shaderCodeString = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const char* shaderCode = shaderCodeString.c_str();

	int success;
	char* infoLog = new char[512];

	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCode, nullptr);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		if (shaderType == GL_VERTEX_SHADER)
		{
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		else if (shaderType == GL_FRAGMENT_SHADER)
		{
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		else if (shaderType == GL_GEOMETRY_SHADER)
		{
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

	delete[] infoLog;
}
void Shader::LinkProgram(GLuint vertex, GLuint fragment, GLuint geometry)
{
	programID = glCreateProgram();
	glAttachShader(programID, vertex);
	glAttachShader(programID, fragment);
	if (geometry != 0)
	{
		glAttachShader(programID, geometry);
	}
	glLinkProgram(programID);

	int success;
	char* infoLog = new char[512];

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	delete[] infoLog;
}

GLuint Shader::GetProgramID() { return programID; }

void Shader::Use() { glUseProgram(programID); }

void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}
void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}
void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::SetVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
}
void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::SetVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
}
void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
}
void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
	glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::SetUniformBlockBinding(const std::string& name, GLuint uniformBlockBinding) const
{
	glUniformBlockBinding(programID, glGetUniformBlockIndex(programID, name.c_str()), uniformBlockBinding);
}