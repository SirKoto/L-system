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

static const char* VERTEX_SHADER_C =
	"#version 330 core\n"
	"layout(location = 0) in vec3 aPos;\n"
	"layout(location = 1) in float aWidth;\n"
	"out VS_OUT {\n"
	"	float width;\n"
	"} vs_out;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(aPos, 1.0);\n"
	"	vs_out.width = aWidth;\n"
	"}\n";
// Extracted from https://github.com/torbjoern/polydraw_scripts/blob/master/geometry/drawcone_geoshader.pss
static const char* GEOMETRY_SHADER_C =
	"#version 330 core\n"
	"#extension GL_ARB_explicit_uniform_location : enable\n"
	"layout(lines) in;\n"
	"layout(triangle_strip, max_vertices = 32) out;\n"
	"layout(location = 0) uniform mat4 MVP;\n"
	"layout(location = 1) uniform float widthScale;\n"
	"in VS_OUT {\n"
	"	float width;\n"
	"} gs_in[];\n"
	"vec3 createPerp(vec3 p1, vec3 p2)"
	"{"
	"	vec3 invec = normalize(p2 - p1);"
	"	vec3 ret = cross(invec, vec3(0.0, 0.0, 1.0));"
	"	if (length(ret) == 0.0)"
	"	{"
	"		ret = cross(invec, vec3(0.0, 1.0, 0.0));"
	"	}"
	"	return ret;"
	"}"
	"void main()\n"
	"{\n"
	"float r1 = widthScale * gs_in[0].width;"
	"float r2 = widthScale * gs_in[1].width;"
	"vec3 axis = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;"
	"vec3 perpx = createPerp(gl_in[1].gl_Position.xyz, gl_in[0].gl_Position.xyz);"
	"vec3 perpy = cross(normalize(axis), perpx);"
	"int segs = 16;"
	"for (int i = 0; i < segs; i++) {"
	"	float a = i / float(segs - 1) * 2.0 * 3.14159;"
	"	float ca = cos(a); float sa = sin(a);"
	"	vec3 normal = vec3(ca * perpx.x + sa * perpy.x,"
	"		ca * perpx.y + sa * perpy.y,"
	"		ca * perpx.z + sa * perpy.z);"
	"	vec3 p1 = gl_in[0].gl_Position.xyz + r1 * normal;"
	"	vec3 p2 = gl_in[1].gl_Position.xyz + r2 * normal;"
	"	gl_Position = MVP * vec4(p1, 1.0); EmitVertex();"
	"	gl_Position = MVP * vec4(p2, 1.0); EmitVertex();"
	"}"
	"EndPrimitive();"
	"}\n";

static const char* FRAGMENT_SHADER_C =
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
	uint32_t vertexC = loadShader(VERTEX_SHADER_C, GL_VERTEX_SHADER);
	uint32_t geometryC = loadShader(GEOMETRY_SHADER_C, GL_GEOMETRY_SHADER);
	uint32_t fragmentC = loadShader(FRAGMENT_SHADER_C, GL_FRAGMENT_SHADER);
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
	{
		mCylinderProgram = glCreateProgram();
		glAttachShader(mCylinderProgram, vertexC);
		glAttachShader(mCylinderProgram, geometryC);
		glAttachShader(mCylinderProgram, fragmentC);

		glLinkProgram(mCylinderProgram);
		//there should not be linking errors....
		int32_t success;
		glGetProgramiv(mCylinderProgram, GL_LINK_STATUS, &success);
		assert(success);
	}
	CheckGLError();
	glDeleteShader(vertexS);
	glDeleteShader(fragmentS);
	glDeleteShader(vertexC);
	glDeleteShader(fragmentC);
	glDeleteShader(geometryC);
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

void Renderer::render(const glm::mat4& projView, uint32_t mode) const
{
	if (mNumPrimitives == 0) {
		return;
	}

	if (mode == 0) {
		glUseProgram(mLineProgram);
		glLineWidth(2);
		CheckGLError();
	}
	else {
		glUseProgram(mCylinderProgram);
		CheckGLError();
		glUniform1f(1, mCylinderWidthMultiplier);
		CheckGLError();
	}

	glUniformMatrix4fv(0, 1, GL_FALSE, &projView[0][0]);
	CheckGLError();


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
