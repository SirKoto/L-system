#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace lParser {

// Rule definition for the parser
struct Rule {
	std::string id;
	float probability = 1.0f;
	std::string mapping;

	Rule() = default;
	Rule(const std::string& id, float p, const std::string& map) : id(id), probability(p), mapping(map) {}
	Rule(const std::string& id, const std::string& map) : id(id), mapping(map) {}
};

// Input data for the parser
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

// Cylinder type to store output data of the parser
struct Cylinder {
	glm::vec3 init, end;
	float width;
};
struct LParserOut {
	std::vector<Cylinder> cylinders;
};

// Main function of the project. Parse some information, creating a new model.
// If returns false, an error has occurred, and string outErr contains an error message.
bool parse(const LParserInfo& info, LParserOut* out, std::string* outErr);

};