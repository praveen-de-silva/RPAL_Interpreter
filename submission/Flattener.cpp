// ================================
// Flattener.cpp
// Flatten the ST before the CSE
// ================================

#include "Flattener.h"

// Initialise with an empty delta[0] (the top-level control sequence).
Flattener::Flattener() : nextDelta(0) {
    deltas.push_back(std::vector<CSENode>());
    nextDelta = 1;
}

// Public entry point: flatten the entire ST rooted at 'root' into delta[0].
void Flattener::flatten(ASTNode* root) {
    if (root) {
        flattenNode(root, 0);
    }
}

// Recursively translate an ST node into CSENode instructions.
// Results are appended to deltas[currentDelta].
void Flattener::flattenNode(ASTNode* node, int currentDelta) {
    if (!node) return;

    if (node->type == "lambda") {
        // Allocate a new delta for the lambda body
        int newDelta = nextDelta++;
        deltas.push_back(std::vector<CSENode>());

        // A lambda node has exactly two children in the ST:
        //   child 1 — bound variable(s): single ID, or a ',' / 'tau' list
        //   child 2 — the body expression
        ASTNode* boundVarNode = node->child;
        ASTNode* bodyNode     = boundVarNode->sibling;

        // Flatten the body into the new delta (not the current one)
        flattenNode(bodyNode, newDelta);

        // Emit a LAMBDA instruction into the current delta that points to the new one
        CSENode lambdaNode(CSENodeType::LAMBDA, newDelta);

        // Collect bound variable names
        if (boundVarNode->type == "," || boundVarNode->type == "tau") {
            // Multiple parameters: walk the sibling chain under the ',' node
            ASTNode* var = boundVarNode->child;
            while (var) {
                lambdaNode.boundVars.push_back(var->value);
                var = var->sibling;
            }
        } else {
            // Single parameter
            lambdaNode.boundVars.push_back(boundVarNode->value);
        }

        deltas[currentDelta].push_back(lambdaNode);
    }
    else if (node->type == "->") {
        // Conditional: three children — condition B, true-branch T, false-branch F
        ASTNode* bNode = node->child;
        ASTNode* tNode = bNode->sibling;
        ASTNode* fNode = tNode->sibling;

        // Each branch gets its own delta
        int tDelta = nextDelta++;
        deltas.push_back(std::vector<CSENode>());
        flattenNode(tNode, tDelta);

        int fDelta = nextDelta++;
        deltas.push_back(std::vector<CSENode>());
        flattenNode(fNode, fDelta);

        // Flatten the condition into the current delta, then append BETA + two DELTA refs.
        // The CSE Machine's rule8_beta pops BETA, reads the two DELTA nodes from control,
        // and pushes the chosen branch.
        flattenNode(bNode, currentDelta);
        deltas[currentDelta].push_back(CSENode(CSENodeType::BETA));
        deltas[currentDelta].push_back(CSENode(CSENodeType::DELTA, tDelta));
        deltas[currentDelta].push_back(CSENode(CSENodeType::DELTA, fDelta));
    }
    else {
        // All other node types: flatten children left-to-right first, then emit this node
        ASTNode* child = node->child;
        int childCount = 0;
        while (child) {
            flattenNode(child, currentDelta);
            childCount++;
            child = child->sibling;
        }

        // Map ST node type to CSENodeType
        CSENodeType cType;
        std::string val = node->value;

        if      (node->type == "IDENTIFIER")                       cType = CSENodeType::IDENTIFIER;
        else if (node->type == "INTEGER")                          cType = CSENodeType::INTEGER;
        else if (node->type == "STRING")                           cType = CSENodeType::STRING;
        else if (node->type == "gamma")                            cType = CSENodeType::GAMMA;
        else if (node->type == "tau") {
            cType = CSENodeType::TAU;
            val   = std::to_string(childCount); // TAU carries arity as its value
        }
        else if (node->type == "Ystar" || node->type == "Y*")     cType = CSENodeType::YSTAR;
        else if (node->type == "true"  || node->type == "<true>")  cType = CSENodeType::TRUE_VAL;
        else if (node->type == "false" || node->type == "<false>") cType = CSENodeType::FALSE_VAL;
        else if (node->type == "nil"   || node->type == "<nil>")   cType = CSENodeType::NIL;
        else if (node->type == "dummy" || node->type == "<dummy>") cType = CSENodeType::DUMMY;
        else {
            // Anything not matched above is an operator (+, -, *, aug, eq, ...)
            cType = CSENodeType::OPERATOR;
            val   = node->type; // operator string is stored in 'type', not 'value'
        }

        deltas[currentDelta].push_back(CSENode(cType, val));
    }
}

// Returns a const reference to the delta array for the CSE Machine.
const std::vector<std::vector<CSENode>>& Flattener::getDeltas() const {
    return deltas;
}

// Debug helper: print all deltas and their instruction sequences.
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
