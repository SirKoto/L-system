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
	void setCylinderScale(float scale) { mCylinderWidthMultiplier = scale; }
	// mode 0 = line
	// mode 1 = cylinders
	void render(const glm::mat4& projView, uint32_t mode) const;

private:
	uint32_t mVAO;
	uint32_t mVBO;
	uint32_t mNumPrimitives;

	float mCylinderWidthMultiplier = 1.0f;

	uint32_t mLineProgram, mCylinderProgram;

	static uint32_t loadShader(const char* shader, uint32_t shaderType);
};