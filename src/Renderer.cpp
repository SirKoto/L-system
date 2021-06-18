#include "Renderer.hpp"

#include <iostream>
#include <glad/glad.h>



const char* GetGLErrorStr(GLenum err)
{
	switch (err)
	{
	case GL_NO_ERROR:          return "No error";
	case GL_INVALID_ENUM:      return "Invalid enum";
	case GL_INVALID_VALUE:     return "Invalid value";
	case GL_INVALID_OPERATION: return "Invalid operation";
	//case GL_STACK_OVERFLOW:    return "Stack overflow";
	//case GL_STACK_UNDERFLOW:   return "Stack underflow";
	case GL_OUT_OF_MEMORY:     return "Out of memory";
	default:                   return "Unknown error";
	}
}

void CheckGLError()
{
	while (true)
	{
		const GLenum err = glGetError();
		if (GL_NO_ERROR == err)
			break;

		std::cout << "GL Error: " << GetGLErrorStr(err) << std::endl;
	}
}


static const char* VERTEX_SHADER = 
	"#version 330 core\n"
	"#extension GL_ARB_explicit_uniform_location : enable\n"
	"layout(location = 0) in vec3 aPos;\n"
	"layout(location = 1) in float aWidth;\n"
	"layout(location = 0) uniform mat4 MVP;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = MVP * vec4(aPos, 1.0);\n"
	"}\n";
static const char* FRAGMENT_SHADER =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"	FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";


Renderer::Renderer() : mNumPrimitives(0)
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	uint32_t vertexS = loadShader(VERTEX_SHADER, GL_VERTEX_SHADER);
	uint32_t fragmentS = loadShader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
	CheckGLError();
	{
		mLineProgram = glCreateProgram();
		glAttachShader(mLineProgram, vertexS);
		glAttachShader(mLineProgram, fragmentS);

		glLinkProgram(mLineProgram);
		//there should not be linking errors....
		int32_t success;
		glGetProgramiv(mLineProgram, GL_LINK_STATUS, &success);
		assert(success);
	}
	CheckGLError();
	glDeleteShader(vertexS);
	glDeleteShader(fragmentS);
	CheckGLError();
}

Renderer::~Renderer()
{
	glDeleteProgram(mLineProgram);

	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void Renderer::setupPrimitivesToRender(const std::vector<lParser::Cylinder>& cylinders)
{
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	struct Data {
		glm::vec3 pos;
		float width;
	};
	std::vector<Data> vertData;
	vertData.reserve(2 * cylinders.size());
	for (const lParser::Cylinder& c : cylinders) {
		vertData.push_back(Data{ c.init, c.width });
		vertData.push_back(Data{ c.end, c.width });
	}
	mNumPrimitives = (uint32_t)vertData.size();
	glBufferData(GL_ARRAY_BUFFER,
		vertData.size() * sizeof(Data),
		vertData.data(),
		GL_STATIC_DRAW);
	CheckGLError();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Data), (void*)offsetof(Data, pos));
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Data), (void*)offsetof(Data, width));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	CheckGLError();
}

void Renderer::render(const glm::mat4& projView) const
{
	if (mNumPrimitives == 0) {
		return;
	}

	if (mLineRenderMode) {
		glUseProgram(mLineProgram);
		glLineWidth(20);
		CheckGLError();
	}

	glUniformMatrix4fv(0, 1, GL_FALSE, &projView[0][0]);

	glBindVertexArray(mVAO);

	glDrawArrays(GL_LINES, 0, mNumPrimitives);
	CheckGLError();

	glBindVertexArray(0);
}

uint32_t Renderer::loadShader(const char* c_str, uint32_t shaderType)
{
	uint32_t shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &c_str, nullptr);
	glCompileShader(shader);
	// check for errors
	int32_t  success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		int32_t len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> errMessage(len, 0);

		glGetShaderInfoLog(shader, len, NULL, errMessage.data());
		std::cerr << "Compilation error:\n" << errMessage.data() << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	CheckGLError();
	return shader;
}
