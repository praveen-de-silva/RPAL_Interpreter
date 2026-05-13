#ifndef FLATTENER_H
#define FLATTENER_H

#include <vector>
#include <string>
#include <iostream>
#include "../parser/ASTNode.h"

// Enum to specify the type of control element in the CSE Machine
enum class CSENodeType {
    IDENTIFIER,
    INTEGER,
    STRING,
    OPERATOR,
    LAMBDA,
    GAMMA,
    TAU,
    DELTA,
    BETA,
    ENV,
    TUPLE,
    YSTAR,
    TRUE_VAL,
    FALSE_VAL,
    NIL,
    DUMMY,
    UNKNOWN
};

// Represents a single control element in the Delta structures
class CSENode {
public:
    CSENodeType type;
    std::string value;    // Used for identifiers, operators, strings, integers
    int deltaIndex;       // Used for LAMBDA and DELTA to point to a delta structure
    int envIndex;         // Used for ENV nodes
    std::vector<std::string> boundVars; // Bound variables for LAMBDA

    // Constructors
    CSENode(CSENodeType t) : type(t), value(""), deltaIndex(-1), envIndex(-1) {}
    CSENode(CSENodeType t, std::string v) : type(t), value(v), deltaIndex(-1), envIndex(-1) {}
    CSENode(CSENodeType t, int dIndex) : type(t), value(""), deltaIndex(dIndex), envIndex(-1) {}

    // Debug print
    void print() const {
        switch (type) {
            case CSENodeType::IDENTIFIER: std::cout << "<id:" << value << ">"; break;
            case CSENodeType::INTEGER: std::cout << "<int:" << value << ">"; break;
            case CSENodeType::STRING: std::cout << "<str:'" << value << "'>"; break;
            case CSENodeType::OPERATOR: std::cout << "<op:" << value << ">"; break;
            case CSENodeType::LAMBDA:
                std::cout << "<lambda";
                if (!boundVars.empty()) {
                    std::cout << " ";
                    for (size_t i = 0; i < boundVars.size(); ++i) {
                        std::cout << boundVars[i] << (i < boundVars.size() - 1 ? "," : "");
                    }
                }
                std::cout << " : " << deltaIndex << ">";
                break;
            case CSENodeType::GAMMA: std::cout << "<gamma>"; break;
            case CSENodeType::TAU: std::cout << "<tau:" << value << ">"; break;
            case CSENodeType::DELTA: std::cout << "<delta:" << deltaIndex << ">"; break;
            case CSENodeType::BETA: std::cout << "<beta>"; break;
            case CSENodeType::ENV: std::cout << "<e" << envIndex << ">"; break;
            case CSENodeType::TUPLE: std::cout << "<tuple>"; break;
            case CSENodeType::YSTAR: std::cout << "<Y*>"; break;
            case CSENodeType::TRUE_VAL: std::cout << "<true>"; break;
            case CSENodeType::FALSE_VAL: std::cout << "<false>"; break;
            case CSENodeType::NIL: std::cout << "<nil>"; break;
            case CSENodeType::DUMMY: std::cout << "<dummy>"; break;
            default: std::cout << "<unknown>"; break;
        }
    }
};

class Flattener {
private:
    std::vector<std::vector<CSENode>> deltas;
    int nextDelta;

    void flattenNode(ASTNode* node, int currentDelta);

public:
    Flattener();
    
    // Main entry point
    void flatten(ASTNode* root);

    // Get the generated deltas
    const std::vector<std::vector<CSENode>>& getDeltas() const;

    // Print all deltas for debugging
    void print() const;
};

#endif // FLATTENER_H
