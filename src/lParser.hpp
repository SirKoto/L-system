#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace lParser {

struct Rule {
	std::string id;
	float probability = 1.0f;
	std::string mapping;

	Rule() = default;
	Rule(const std::string& id, float p, const std::string& map) : id(id), probability(p), mapping(map) {}
	Rule(const std::string& id, const std::string& map) : id(id), mapping(map) {}
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
	int32_t rngSeed = 15312;
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