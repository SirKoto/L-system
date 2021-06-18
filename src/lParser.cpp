#include "lParser.hpp"

#include <map>
#include <memory>
#include <cctype>
#include <glm/gtx/quaternion.hpp>
#include <stack>

struct Node {
    std::unique_ptr<Node> brother;
    std::unique_ptr<Node> son;
    double value;
    enum class Op {
        eValue
    };
};

struct Turtle {
    glm::vec3 pos = glm::vec3(0);
    glm::quat rotation = glm::quat(1.f, glm::vec3(0.f));

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
        float sinA = std::sinf(angle * 0.5f);
        float cosA = std::cosf(angle * 0.5f);
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
    std::vector<lParser::Cylinder>* outCyls;
    std::map<char, std::string>* symbolMap;
};

bool processRule(const std::string& axiom,
    const uint32_t depth,
    ParseData* data) {
    Turtle& turtle = data->turtle;
    lParser::Cylinder cylinder;
    cylinder.width = 0.05;
    for (size_t i = 0; i < axiom.size(); ++i) {
        const char c = axiom[i];

        switch (c)
        {
        case 'F':
            cylinder.init = turtle.pos;
            turtle.advance(1.0f);
            cylinder.end = turtle.pos;
            data->outCyls->push_back(cylinder);
            break;
        case '+':
            turtle.rotateArround(data->defaultAngle, turtle.up());
            break;
        case '-':
            turtle.rotateArround(-data->defaultAngle, turtle.up());
            break;
        case '/':
            turtle.rotateArround(data->defaultAngle, turtle.forward());
            break;
        case '\\':
            turtle.rotateArround(-data->defaultAngle, turtle.forward());
            break;
        case '&':
            turtle.rotateArround(data->defaultAngle, turtle.left());
            break;
        case '^':
            turtle.rotateArround(-data->defaultAngle, turtle.left());
            break;
        case '|':
            turtle.rotateArround(glm::pi<float>(), turtle.left());
            break;
        case '[':
            data->turtleStack.push(data->turtle);
            break;
        case ']':
            if (data->turtleStack.empty()) {
                return false;
            }
            data->turtle = data->turtleStack.top();
            data->turtleStack.pop();
            break;
        default:
            break;
        }

        if (std::isalpha(c) && depth < data->maxDepth) {
            std::map<char, std::string>::const_iterator it = data->symbolMap->find(c);
            if (it != data->symbolMap->end()) {
                bool ret = processRule(it->second, depth + 1, data);
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
    
    std::map<char, std::string> symbolMap;
    for (const Rule& rule : info.rules) {
        if (rule.id.empty()) {
            *outErr = "There is a rule without ID";
            return false;
        }
        if (!std::isalpha(rule.id.front())) {
            *outErr = rule.id + " has not a char as an identifier";
            return false;
        }

        symbolMap.emplace(rule.id.front(), rule.mapping);
    }

    ParseData parseData;
    parseData.maxDepth = info.maxRecursionLevel;
    parseData.outCyls = &out->cylinders;
    parseData.symbolMap = &symbolMap;
    parseData.defaultAngle = glm::radians(info.defaultAngle);

    return processRule(info.axiom, 0, &parseData);
}
