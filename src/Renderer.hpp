#pragma once

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "lParser.hpp"

class Renderer {
public:
	Renderer();
	~Renderer();

	// Send to the GPU the cylinders to render
	void setupPrimitivesToRender(const std::vector<lParser::Cylinder>& cylinders);
	// Update the scale of the cylinders
	void setCylinderScale(float scale) { mCylinderWidthMultiplier = scale; }
	// Update the color of the plant
	void setPlantColor(const glm::vec3& color) { mColor = color; }
	// mode 0 = line
	// mode 1 = cylinders
	// mode 2 = cylinders shaded witht the normal
	void render(const glm::mat4& projView, uint32_t mode) const;

private:
	uint32_t mVAO;
	uint32_t mVBO;
	uint32_t mNumPrimitives;

	float mCylinderWidthMultiplier = 1.0f;
	glm::vec3 mColor = glm::vec3(0.1f, 0.9f, 0.2f);
	uint32_t mLineProgram, mCylinderProgram, mCylinderProgramNormal;

	static uint32_t loadShader(const char* shader, uint32_t shaderType);
};