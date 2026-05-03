#include "Flattener.h"

Flattener::Flattener() : nextDelta(0) {
    // Initialize delta 0
    deltas.push_back(std::vector<CSENode>());
    nextDelta = 1;
}

void Flattener::flatten(ASTNode* root) {
    if (root) {
        flattenNode(root, 0);
    }
}

void Flattener::flattenNode(ASTNode* node, int currentDelta) {
    if (!node) return;

    if (node->type == "lambda") {
        int newDelta = nextDelta++;
        deltas.push_back(std::vector<CSENode>());

        // A lambda node in ST has 2 children:
        // 1. the bound variable (could be a single id or a comma list/tau of ids)
        // 2. the body
        ASTNode* boundVarNode = node->child;
        ASTNode* bodyNode = boundVarNode->sibling;

        // Flatten the body into the new delta
        flattenNode(bodyNode, newDelta);

        // Add LAMBDA node to current delta
        CSENode lambdaNode(CSENodeType::LAMBDA, newDelta);
        
        // Extract bound variables
        if (boundVarNode->type == "," || boundVarNode->type == "tau") {
            ASTNode* var = boundVarNode->child;
            while (var) {
                lambdaNode.boundVars.push_back(var->value);
                var = var->sibling;
            }
        } else {
            lambdaNode.boundVars.push_back(boundVarNode->value);
        }

        deltas[currentDelta].push_back(lambdaNode);
    }
    else if (node->type == "->") {
        // Condition node: 3 children -> B, T, F
        ASTNode* bNode = node->child;
        ASTNode* tNode = bNode->sibling;
        ASTNode* fNode = tNode->sibling;

        int tDelta = nextDelta++;
        deltas.push_back(std::vector<CSENode>());
        flattenNode(tNode, tDelta);

        int fDelta = nextDelta++;
        deltas.push_back(std::vector<CSENode>());
        flattenNode(fNode, fDelta);

        // Sequence: B, DELTA_T, DELTA_F, BETA
        flattenNode(bNode, currentDelta);
        
        deltas[currentDelta].push_back(CSENode(CSENodeType::DELTA, tDelta));
        deltas[currentDelta].push_back(CSENode(CSENodeType::DELTA, fDelta));
        deltas[currentDelta].push_back(CSENode(CSENodeType::BETA));
    }
    else {
        // Other nodes: process children left-to-right, then add the node
        ASTNode* child = node->child;
        int childCount = 0;
        while (child) {
            flattenNode(child, currentDelta);
            childCount++;
            child = child->sibling;
        }

        // Determine node type and add
        CSENodeType cType;
        std::string val = node->value;
        
        if (node->type == "IDENTIFIER") {
            cType = CSENodeType::IDENTIFIER;
        }
        else if (node->type == "INTEGER") {
            cType = CSENodeType::INTEGER;
        }
        else if (node->type == "STRING") {
            cType = CSENodeType::STRING;
        }
        else if (node->type == "gamma") {
            cType = CSENodeType::GAMMA;
        }
        else if (node->type == "tau") {
            cType = CSENodeType::TAU;
            val = std::to_string(childCount);
        }
        else if (node->type == "Ystar" || node->type == "Y*") {
            cType = CSENodeType::YSTAR;
        }
        else if (node->type == "true" || node->type == "<true>") {
            cType = CSENodeType::TRUE_VAL;
        }
        else if (node->type == "false" || node->type == "<false>") {
            cType = CSENodeType::FALSE_VAL;
        }
        else if (node->type == "nil" || node->type == "<nil>") {
            cType = CSENodeType::NIL;
        }
        else if (node->type == "dummy" || node->type == "<dummy>") {
            cType = CSENodeType::DUMMY;
        }
        else {
            cType = CSENodeType::OPERATOR;
            val = node->type; // Operator string (+, -, etc.)
        }

        deltas[currentDelta].push_back(CSENode(cType, val));
    }
}

const std::vector<std::vector<CSENode>>& Flattener::getDeltas() const {
    return deltas;
}

void Flattener::print() const {
    for (size_t i = 0; i < deltas.size(); ++i) {
        std::cout << "Delta " << i << ": [ ";
        for (const auto& node : deltas[i]) {
            node.print();
            std::cout << " ";
        }
        std::cout << "]\n";
    }
}
