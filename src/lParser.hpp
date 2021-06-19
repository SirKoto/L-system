#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace lParser {

struct Rule {
	std::string id;
	std::string mapping;
};

struct LParserInfo
{
	std::string axiom;
	std::vector<std::pair<std::string, float>> constants;
	std::vector<Rule> rules;
	uint32_t maxRecursionLevel = 0;
	float defaultAngle = 20.0f;
	float defaultThickness = 0.05f;
	float thicknessReductionFactor = 0.707f;
};

struct Cylinder {
	glm::vec3 init, end;
	float width;
};
struct LParserOut {
	std::vector<Cylinder> cylinders;
};

bool parse(const LParserInfo& info, LParserOut* out, std::string* outErr);

};