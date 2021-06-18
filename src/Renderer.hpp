#pragma once

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "lParser.hpp"

class Renderer {
public:
	Renderer();
	~Renderer();

	void setupPrimitivesToRender(const std::vector<lParser::Cylinder>& cylinders);

	void render(const glm::mat4& projView) const;

private:
	uint32_t mVAO;
	uint32_t mVBO;
	uint32_t mNumPrimitives;
	bool mLineRenderMode = true;

	uint32_t mLineProgram;

	static uint32_t loadShader(const char* shader, uint32_t shaderType);
};