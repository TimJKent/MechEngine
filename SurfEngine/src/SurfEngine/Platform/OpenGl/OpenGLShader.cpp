#include "sepch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SurfEngine {

	static GLenum ShaderTypeFromString(const std::string& type) {
		if (type == "vertex") return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

		SE_CORE_ASSERT(false, "Unknown Shader Type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath) {
		std::string shaderSource = ReadFile(filepath);
		auto shaderSources = PreProcess(shaderSource);
		Compile(shaderSources);
		
		//Extract Name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash+1;
		auto lastDot = filepath.rfind(".");
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash,lastDot);
	}


	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) :
		m_Name(name)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources);
	}
	
	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath) {
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {

			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else {
			SE_CORE_ERROR("Could Not Open File: '{0}'", filepath);
		}
		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) {
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			SE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			SE_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size()-1 : nextLinePos ));
		}
		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources) {
		GLuint program = glCreateProgram();
		SE_CORE_ASSERT(shaderSources.size() <= 2, "Shader files must contain 2 shaders");
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources) {
			GLenum type = kv.first;
			const std::string source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = (const GLchar*)source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			//Compile the vertex shader 
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE) {
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				//We Don't need the shader anymore
				glDeleteShader(shader);

				//Use the infoLog as you see fit
				SE_CORE_ERROR("{0}", infoLog.data());
				SE_CORE_ASSERT(false, "Shader Compilation Failure!");
				break;
			}
			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}


		//Link our program
		glLinkProgram(program);

		//Notice the different functinos here: glGetProgram instead of glGetShader
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			//We don't need the program anymore.
			glDeleteProgram(program);
			//Dont leak shaders either
			for (auto id : glShaderIDs) {
				glDeleteShader(id);
			}
	
			//Use infoLog as you see fit
			SE_CORE_ERROR("{0}", infoLog.data());
			SE_CORE_ASSERT(false, "Shader Linking Failure!");
			return;
		}

		//Always detach shaders after a successful link.
		for (auto id : glShaderIDs) {
			glDetachShader(program, id);
		}

		m_RendererID = program;
	}


	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

	void OpenGLShader::SetFloat(const std::string& name, const float value) {
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value) {
		UploadUniformVec2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) {
		UploadUniformVec3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value){
		UploadUniformVec4(name, value);
	}
	
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) {
		UploadUniformMat4(name, value);
	}
	
	void OpenGLShader::SetInt(const std::string& name, const int value) {
		UploadUniformInt(name, value);
	}


	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4 matrix) {
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float num) {
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, num);
	}

	void OpenGLShader::UploadUniformVec2(const std::string& name, const glm::vec2 num) {
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, num.x, num.y);
	}

	void OpenGLShader::UploadUniformVec3(const std::string& name, const glm::vec3 num) {
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, num.x, num.y, num.z);
	}

	void OpenGLShader::UploadUniformVec4(const std::string& name, const glm::vec4 vector) {
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, vector.r, vector.g, vector.b, vector.a);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const uint32_t num) {
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, num);
	}
}