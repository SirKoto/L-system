#include "lParser.hpp"

#include <map>
#include <unordered_map>
#include <memory>
#include <cctype>
#include <glm/gtx/quaternion.hpp>
#include <stack>
#include <cstdlib>
#include <cmath>
#include <random>

struct Turtle {
    glm::vec3 pos = glm::vec3(0);
    glm::quat rotation = glm::quat(1.f, glm::vec3(0.f));
    float thickness = 0.05f;

    void advance(float t) {
        pos += t * this->forward();
    }

    glm::vec3 forward() const
    {
        return glm::rotate(rotation, glm::vec3(0.f, 1.f, 0.f));
    }

    glm::vec3 left() const
    {
        return glm::rotate(rotation, glm::vec3(-1.f, 0.f, 0.f));
    }

    glm::vec3 up() const
    {
        return glm::rotate(rotation, glm::vec3(0.f, 0.f, 1.f));
    }

    void rotateArround(float angle, glm::vec3 axis)
    {
        //angle = std::fmod(angle, 
        float sinA = std::sin(angle * 0.5f);
        float cosA = std::cos(angle * 0.5f);
        rotation = glm::quat(cosA, axis * sinA) * rotation;

        // normalize if needed
        float dot = glm::dot(rotation, rotation);
        if (std::abs(dot - 1.0f) > 1e-4) {
            rotation = rotation / std::sqrt(dot);
        }
    }
};

struct ParseData {
    Turtle turtle;
    std::stack<Turtle> turtleStack;
    uint32_t maxDepth;
    float defaultAngle;
    float thicknessReductionFactor;
    std::vector<lParser::Cylinder>* outCyls;

    typedef struct StockVal {
        float probability;
        std::string rule;
    } StockVal;
    std::map<char, std::vector<StockVal>>* symbolMap;
    std::unordered_map<std::string, float>* constantsMap;

    std::mt19937 rng;
    std::uniform_real_distribution<float> distr = std::uniform_real_distribution<float>(0.0, 1.0);
};

float checkIfCustomValue(const std::string& axiom, const ParseData* data, float defaultValue, size_t* i_,
    bool* error, std::string* outErr) {
    size_t i = *i_;
    if (axiom.size() <= i + 1 || axiom[i + 1] != '(') {
        return defaultValue;
    }

    // We know that at i+1 there is an (
    size_t j = axiom.find(')', i + 2);
    if (j == std::string::npos) {
        *error |= true;
        *outErr = "Can't find closing )";
        return defaultValue;
    }

    float val;
    // is constant or value?
    if (std::isalpha(axiom[i + 2])) {
        std::string id = axiom.substr(i + 2, j - (i + 2));
        auto it = data->constantsMap->find(id);
        if (it == data->constantsMap->end()) {
            *error |= true;
            *outErr = "Can't find constant " + id;
            val = defaultValue;
        }
        else {
            val = it->second;
        }
    }
    else { // it is a value
        val = std::strtof(axiom.c_str() + i + 2, nullptr);
    }
    
    (*i_) = j;
    return val;
}

bool processRule(const std::string& axiom,
    const uint32_t depth,
    ParseData* data,
    std::string* outErr) {
    Turtle& turtle = data->turtle;
    lParser::Cylinder cylinder;
    bool error = false;
    float value;
    for (size_t i = 0; i < axiom.size(); ++i) {
        const char c = axiom[i];

        switch (c)
        {
        case 'F':
            cylinder.width = turtle.thickness;
            cylinder.init = turtle.pos;
            turtle.advance(1.0f);
            cylinder.end = turtle.pos;
            data->outCyls->push_back(cylinder);
            break;
        case '+':
            value = checkIfCustomValue(axiom, data, data->defaultAngle, &i, &error, outErr);
            turtle.rotateArround(glm::radians(value), turtle.up());
            break;
        case '-':
            value = checkIfCustomValue(axiom, data, data->defaultAngle, &i, &error, outErr);
            turtle.rotateArround(-glm::radians(value), turtle.up());
            break;
        case '/':
            value = checkIfCustomValue(axiom, data, data->defaultAngle, &i, &error, outErr);
            turtle.rotateArround(glm::radians(value), turtle.forward());
            break;
        case '\\':
            value = checkIfCustomValue(axiom, data, data->defaultAngle, &i, &error, outErr);
            turtle.rotateArround(-glm::radians(value), turtle.forward());
            break;
        case '&':
            value = checkIfCustomValue(axiom, data, data->defaultAngle, &i, &error, outErr);
            turtle.rotateArround(glm::radians(value), turtle.left());
            break;
        case '^':
            value = checkIfCustomValue(axiom, data, data->defaultAngle, &i, &error, outErr);
            turtle.rotateArround(-glm::radians(value), turtle.left());
            break;
        case '|':
            turtle.rotateArround(glm::pi<float>(), turtle.left());
            break;
        case '[':
            data->turtleStack.push(data->turtle);
            break;
        case ']':
            if (data->turtleStack.empty()) {
                *outErr = "Can't find closing ]";
                return false;
            }
            data->turtle = data->turtleStack.top();
            data->turtleStack.pop();
            break;
        case '<':
            value = checkIfCustomValue(axiom, data, data->thicknessReductionFactor, &i, &error, outErr);
            data->turtle.thickness /= value;
            break;
        case '>':
            value = checkIfCustomValue(axiom, data, data->thicknessReductionFactor, &i, &error, outErr);
            data->turtle.thickness *= value;
            break;
        default:
            break;
        }

        if (error) {
            return false;
        }

        if (std::isalpha(c) && depth < data->maxDepth) {
            std::map<char, std::vector<ParseData::StockVal>>::const_iterator it = data->symbolMap->find(c);
            if (it != data->symbolMap->end()) {
                const std::string* nextAxiom = &it->second.back().rule;

                if (it->second.size() > 1) {
                    // get random value and check
                    float val = data->distr(data->rng);
                    for (const ParseData::StockVal& e : it->second) {
                        if (e.probability >= val) {
                            nextAxiom = &e.rule;
                            break;
                        }
                    }
                }
                bool ret = processRule(*nextAxiom, depth + 1, data, outErr);
                if (!ret) return false;
            }
        }
    }

    return true;
}

bool lParser::parse(const LParserInfo& info, LParserOut* out, std::string* outErr)
{
    assert(out != nullptr && outErr != nullptr);

    std::vector<Cylinder>& accum = out->cylinders;
    accum.clear();
    
    std::map<char, std::vector<ParseData::StockVal>> symbolMap;
    for (const Rule& rule : info.rules) {
        if (rule.id.empty()) {
            *outErr = "There is a rule without ID";
            return false;
        }
        if (!std::isalpha(rule.id.front())) {
            *outErr = rule.id + " has not a char as an identifier";
            return false;
        }

        decltype(symbolMap)::iterator it = symbolMap.find(rule.id.front());
        // if not exists... insert new
        if (it == symbolMap.end()) {
            symbolMap.insert({ rule.id.front(), { {rule.probability, rule.mapping} } });
        }
        else {
            it->second.push_back({rule.probability + it->second.back().probability, rule.mapping});
        }

    }

    // fix probabilities and check correct distributions
    for (auto& it : symbolMap) {
        if (std::abs(it.second.back().probability - 1.0f) > 1e-2) {
            *outErr = "Probabilites do not add up to 1";
            return false;
        }
    }

    std::unordered_map<std::string, float> constantsMap;
    for (const auto& c : info.constants) {
        constantsMap.emplace(c.first, c.second);
    }

    ParseData parseData;
    parseData.maxDepth = info.maxRecursionLevel;
    parseData.outCyls = &out->cylinders;
    parseData.symbolMap = &symbolMap;
    parseData.defaultAngle = info.defaultAngle;
    parseData.constantsMap = &constantsMap;
    parseData.thicknessReductionFactor = info.thicknessReductionFactor;
    parseData.turtle.thickness = info.defaultThickness;
    parseData.rng = std::mt19937(info.rngSeed); // set seed
    return processRule(info.axiom, 0, &parseData, outErr);
}
