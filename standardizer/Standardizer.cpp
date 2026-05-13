// ============================================================
// Standardizer.cpp
// Implements the Standardizer (Stage 4 of RPAL Interpreter).
//
// Transforms AST → Standardized Tree (ST) by applying
// fixed transformation rules bottom-up (children first).
//
// ASTNode structure (from friend's parser):
//   node->child   = first child
//   child->sibling = next sibling
//   Children form a linked list via sibling pointers.
//
// Example: let node with 2 children [D, E]:
//   letNode->child          = D
//   D->sibling              = E
//   E->sibling              = nullptr
//
// Transformation Rules Summary:
//
//  Rule 1: let
//    AST:  let               ST:  gamma
//           ├── = (or rec)        ├── lambda
//           │    ├── x            │    ├── x
//           │    └── E            │    └── P
//           └── P                 └── E
//
//  Rule 2: where
//    AST:  where             ST:  gamma
//           ├── P                 ├── lambda
//           └── = (or rec)        │    ├── x
//                ├── x            │    └── P
//                └── E            └── E
//
//  Rule 3: fcn_form
//    AST:  fcn_form          ST:  =
//           ├── f                 ├── f
//           ├── x1                └── lambda
//           ├── x2                     ├── x1
//           └── E                      └── lambda
//                                           ├── x2
//                                           └── E
//
//  Rule 4: lambda (multiple Vb)
//    AST:  lambda            ST:  lambda
//           ├── x1                ├── x1
//           ├── x2                └── lambda
//           └── E                      ├── x2
//                                      └── E
//
//  Rule 5: and
//    AST:  and               ST:  =
//           ├── = (x1,E1)         ├── tau
//           └── = (x2,E2)         │    ├── x1
//                                  │    └── x2
//                                  └── tau
//                                       ├── E1
//                                       └── E2
//
//  Rule 6: rec
//    AST:  rec               ST:  =
//           └── = (f, E)          ├── f
//                                  └── gamma
//                                       ├── Ystar
//                                       └── lambda
//                                            ├── f
//                                            └── E
//
//  Rule 7: within
//    AST:  within            ST:  gamma
//           ├── = (x1,E1)         ├── lambda
//           └── = (x2,E2)         │    ├── x2
//                                  │    └── E2
//                                  └── gamma
//                                       ├── lambda
//                                       │    ├── x1
//                                       │    └── E1
//                                       └── E0 (optional)
//
// Author: 230123K
// ============================================================

#include "Standardizer.h"
#include <iostream>
#include <stdexcept>

// ── Helper: Create a new node ─────────────────────────────────────────────────

// Creates a fresh ASTNode with the given type label and optional value.
// The new node has no children (child = nullptr) and no sibling.
ASTNode* Standardizer::makeNode(const std::string& type,
                                 const std::string& value) {
    return new ASTNode(type, value);
}

// ── Helper: Deep copy a subtree ───────────────────────────────────────────────

// Creates a complete deep copy of the subtree rooted at 'node'.
// Needed for the 'rec' rule where the variable name is duplicated.
// Copies both child and sibling chains.
ASTNode* Standardizer::copyTree(ASTNode* node) {
    if (node == nullptr) return nullptr;

    // Copy this node
    ASTNode* copy = makeNode(node->type, node->value);

    // Recursively copy children
    copy->child = copyTree(node->child);

    // Recursively copy siblings
    copy->sibling = copyTree(node->sibling);

    return copy;
}

// ── Helper: Get Nth child (0-indexed) ────────────────────────────────────────

// Returns the Nth child of a node by traversing the sibling chain.
// Children: node->child (0), child->sibling (1), child->sibling->sibling (2) ...
// Returns nullptr if N is out of range.
ASTNode* Standardizer::getChild(ASTNode* node, int n) {
    if (node == nullptr) return nullptr;

    ASTNode* current = node->child;
    for (int i = 0; i < n; i++) {
        if (current == nullptr) return nullptr;
        current = current->sibling;
    }
    return current;
}

// ── Helper: Count children ───────────────────────────────────────────────────

// Returns the total number of children of a node.
int Standardizer::countChildren(ASTNode* node) {
    if (node == nullptr) return 0;
    int count = 0;
    ASTNode* current = node->child;
    while (current != nullptr) {
        count++;
        current = current->sibling;
    }
    return count;
}

// ── Main standardize function ─────────────────────────────────────────────────

// Recursively standardizes the entire tree bottom-up.
// Children are standardized BEFORE the parent node is transformed.
// This ensures nested transformations are handled correctly.
ASTNode* Standardizer::standardize(ASTNode* node) {
    if (node == nullptr) return nullptr;

    // ── Step 1: Standardize all children FIRST (bottom-up) ───
    ASTNode* child = node->child;
    while (child != nullptr) {
        standardize(child);
        child = child->sibling;
    }

    // ── Step 2: Apply transformation rule for THIS node ──────
    if (node->type == "let") {
        standardizeLet(node);
    }
    else if (node->type == "where") {
        standardizeWhere(node);
    }
    else if (node->type == "fcn_form") {
        standardizeFcnForm(node);
    }
    else if (node->type == "lambda") {
        standardizeLambda(node);
    }
    else if (node->type == "and") {
        standardizeAnd(node);
    }
    else if (node->type == "rec") {
        standardizeRec(node);
    }
    else if (node->type == "within") {
        standardizeWithin(node);
    }
    // All other nodes (gamma, +, -, *, etc.) need no transformation

    return node;
}

// ── Rule 1: let ───────────────────────────────────────────────────────────────
//
// AST:                      ST:
//   let                       gamma
//    ├── = (or fcn_form)        ├── lambda
//    │    ├── x                 │    ├── x
//    │    └── E                 │    └── P
//    └── P                      └── E
//
// The let node is transformed IN-PLACE into a gamma node.
void Standardizer::standardizeLet(ASTNode* node) {
    // node is the 'let' node
    // child[0] = the '=' or 'fcn_form' definition node
    // child[1] = the body expression P

    ASTNode* defNode = node->child;          // '=' node: has children [x, E]
    ASTNode* P       = defNode->sibling;     // body expression

    if (defNode == nullptr || P == nullptr) {
        std::cerr << "Standardizer Error: malformed 'let' node" << std::endl;
        return;
    }

    // Extract x and E from the definition node
    ASTNode* x = defNode->child;            // variable (first child of '=')
    ASTNode* E = x->sibling;               // expression (second child of '=')

    // Detach x and E from defNode
    x->sibling = nullptr;
    E->sibling = nullptr;

    // Detach P from defNode
    defNode->sibling = nullptr;

    // Build: lambda
    //          ├── x
    //          └── P
    ASTNode* lambdaNode = makeNode("lambda");
    lambdaNode->child   = x;
    x->sibling          = P;
    P->sibling          = nullptr;

    // Build: gamma
    //          ├── lambda
    //          └── E
    // We reuse 'node' as the gamma node (transform in place)
    node->type          = "gamma";
    node->child         = lambdaNode;
    lambdaNode->sibling = E;
    E->sibling          = nullptr;

    // Delete the now-unused definition node
    defNode->child   = nullptr;  // prevent recursive delete of x and E
    defNode->sibling = nullptr;
    delete defNode;
}

// ── Rule 2: where ─────────────────────────────────────────────────────────────
//
// AST:                      ST:
//   where                     gamma
//    ├── P                      ├── lambda
//    └── =                      │    ├── x
//         ├── x                 │    └── P
//         └── E                 └── E
//
// Same result as 'let' — just different child order in AST.
void Standardizer::standardizeWhere(ASTNode* node) {
    // node is the 'where' node
    // child[0] = body expression P
    // child[1] = the '=' definition node

    ASTNode* P       = node->child;          // body expression
    ASTNode* defNode = P->sibling;           // '=' definition node

    if (P == nullptr || defNode == nullptr) {
        std::cerr << "Standardizer Error: malformed 'where' node" << std::endl;
        return;
    }

    // Extract x and E from definition
    ASTNode* x = defNode->child;
    ASTNode* E = x->sibling;

    // Detach everything cleanly
    P->sibling       = nullptr;
    x->sibling       = nullptr;
    E->sibling       = nullptr;
    defNode->sibling = nullptr;

    // Build: lambda
    //          ├── x
    //          └── P
    ASTNode* lambdaNode = makeNode("lambda");
    lambdaNode->child   = x;
    x->sibling          = P;
    P->sibling          = nullptr;

    // Transform node in place to gamma
    node->type          = "gamma";
    node->child         = lambdaNode;
    lambdaNode->sibling = E;
    E->sibling          = nullptr;

    // Clean up definition node
    defNode->child   = nullptr;
    defNode->sibling = nullptr;
    delete defNode;
}

// ── Rule 3: fcn_form ─────────────────────────────────────────────────────────
//
// AST:                       ST:
//   fcn_form                   =
//    ├── f (function name)      ├── f
//    ├── x1 (param 1)           └── lambda
//    ├── x2 (param 2)                ├── x1
//    └── E  (body)                   └── lambda
//                                         ├── x2
//                                         └── E
//
// Multiple Vb parameters are nested as chained lambdas (right to left).
void Standardizer::standardizeFcnForm(ASTNode* node) {
    // node is 'fcn_form'
    // child[0] = function name identifier (f)
    // child[1]...child[n-1] = parameter Vbs
    // child[n] = body expression E

    ASTNode* f = node->child;   // function name

    // Collect all Vb parameters and the body into a list
    // Parameters: from f->sibling up to (but not including) the last child
    // Body: last child

    // Find all children starting from f->sibling
    // Last child is the body E; everything before it is a Vb parameter

    // Count parameters (all children except f and E)
    std::vector<ASTNode*> params;
    ASTNode* current = f->sibling;
    ASTNode* body    = nullptr;

    // Traverse to find all params and body
    while (current != nullptr) {
        if (current->sibling == nullptr) {
            // Last child = body expression E
            body = current;
        } else {
            params.push_back(current);
        }
        current = current->sibling;
    }

    if (body == nullptr) {
        std::cerr << "Standardizer Error: malformed 'fcn_form' node" << std::endl;
        return;
    }

    // Detach f from siblings
    f->sibling = nullptr;

    // Detach body from params chain
    if (!params.empty()) {
        params.back()->sibling = nullptr;
    }
    body->sibling = nullptr;

    // Build nested lambdas from RIGHT to LEFT
    // Last lambda wraps the body
    // lambda x_n E
    ASTNode* innermost = makeNode("lambda");
    innermost->child   = params.back();
    params.back()->sibling = body;

    // Wrap each additional param (right to left)
    for (int i = (int)params.size() - 2; i >= 0; i--) {
        ASTNode* outer = makeNode("lambda");
        outer->child   = params[i];
        params[i]->sibling = innermost;
        innermost = outer;
    }

    // Transform node in place to '='
    // =
    //  ├── f
    //  └── lambda(...)
    node->type       = "=";
    node->child      = f;
    f->sibling       = innermost;
    innermost->sibling = nullptr;
}

// ── Rule 4: lambda (multiple Vb params) ──────────────────────────────────────
//
// AST:                      ST:
//   lambda                    lambda
//    ├── x1                    ├── x1
//    ├── x2                    └── lambda
//    └── E                          ├── x2
//                                   └── E
//
// A lambda with multiple parameters is converted to
// nested single-parameter lambdas.
// If only one parameter, no transformation needed.
void Standardizer::standardizeLambda(ASTNode* node) {
    // node is 'lambda'
    // Count children: if only 2 (one param + body), nothing to do
    int numChildren = countChildren(node);
    if (numChildren <= 2) {
        // Single param lambda — already in standard form
        return;
    }

    // Multiple params: x1, x2, ..., xn, E
    // Collect all params and the body
    std::vector<ASTNode*> params;
    ASTNode* current = node->child;
    ASTNode* body    = nullptr;

    while (current != nullptr) {
        if (current->sibling == nullptr) {
            body = current;
        } else {
            params.push_back(current);
        }
        current = current->sibling;
    }

    if (body == nullptr || params.empty()) {
        return;
    }

    // Detach all from siblings
    for (auto& p : params) p->sibling = nullptr;
    body->sibling = nullptr;

    // Build nested lambdas from RIGHT to LEFT
    ASTNode* innermost = makeNode("lambda");
    innermost->child   = params.back();
    params.back()->sibling = body;

    for (int i = (int)params.size() - 2; i >= 0; i--) {
        ASTNode* outer = makeNode("lambda");
        outer->child   = params[i];
        params[i]->sibling = innermost;
        innermost = outer;
    }

    // Keep node as lambda but update its children
    // node becomes: lambda x1 (lambda x2 (...))
    node->child              = params[0];
    params[0]->sibling       = innermost;
    innermost->sibling       = nullptr;
}

// ── Rule 5: and ───────────────────────────────────────────────────────────────
//
// AST:                      ST:
//   and                       =
//    ├── = (x1, E1)            ├── tau
//    ├── = (x2, E2)            │    ├── x1
//    └── = (x3, E3)            │    └── x2 ...
//                               └── tau
//                                    ├── E1
//                                    ├── E2 ...
//
// All variable names go into first tau.
// All expressions go into second tau.
void Standardizer::standardizeAnd(ASTNode* node) {
    // node is 'and'
    // Each child is a '=' node with [xi, Ei]

    ASTNode* tauVars = makeNode("tau");  // tau of variable names
    ASTNode* tauVals = makeNode("tau");  // tau of expressions

    ASTNode* lastVar = nullptr;
    ASTNode* lastVal = nullptr;

    // Walk through each '=' child of 'and'
    ASTNode* current = node->child;
    while (current != nullptr) {
        ASTNode* nextDef = current->sibling;  // save next before we modify

        // Extract variable name (xi) and expression (Ei) from '=' node
        ASTNode* xi = current->child;
        ASTNode* Ei = xi->sibling;

        // Detach
        xi->sibling       = nullptr;
        Ei->sibling       = nullptr;
        current->sibling  = nullptr;
        current->child    = nullptr;

        // Add xi to tauVars chain
        if (tauVars->child == nullptr) {
            tauVars->child = xi;
            lastVar = xi;
        } else {
            lastVar->sibling = xi;
            lastVar = xi;
        }

        // Add Ei to tauVals chain
        if (tauVals->child == nullptr) {
            tauVals->child = Ei;
            lastVal = Ei;
        } else {
            lastVal->sibling = Ei;
            lastVal = Ei;
        }

        // Delete the now-empty '=' node
        delete current;

        current = nextDef;
    }

    // Transform node to '='
    // =
    //  ├── tau (vars)
    //  └── tau (vals)
    node->type          = "=";
    node->child         = tauVars;
    tauVars->sibling    = tauVals;
    tauVals->sibling    = nullptr;
}

// ── Rule 6: rec ───────────────────────────────────────────────────────────────
//
// AST:                      ST:
//   rec                       =
//    └── = (f, E)              ├── f
//                               └── gamma
//                                    ├── Ystar
//                                    └── lambda
//                                         ├── f (copy)
//                                         └── E
//
// Y* is the fixed-point combinator for recursion.
void Standardizer::standardizeRec(ASTNode* node) {
    // node is 'rec'
    // child[0] = '=' node with children [f, E]

    ASTNode* defNode = node->child;  // '=' node

    if (defNode == nullptr || defNode->child == nullptr) {
        std::cerr << "Standardizer Error: malformed 'rec' node" << std::endl;
        return;
    }

    ASTNode* f = defNode->child;        // function name identifier
    ASTNode* E = f->sibling;            // body expression

    // Detach cleanly
    f->sibling       = nullptr;
    E->sibling       = nullptr;
    defNode->child   = nullptr;
    defNode->sibling = nullptr;

    // We need a copy of f for the lambda parameter
    ASTNode* fCopy = copyTree(f);

    // Build: lambda
    //          ├── fCopy
    //          └── E
    ASTNode* lambdaNode  = makeNode("lambda");
    lambdaNode->child    = fCopy;
    fCopy->sibling       = E;
    E->sibling           = nullptr;

    // Build: Ystar node (Y* fixed point combinator)
    ASTNode* ystar = makeNode("Ystar");

    // Build: gamma
    //          ├── Ystar
    //          └── lambda
    ASTNode* gammaNode   = makeNode("gamma");
    gammaNode->child     = ystar;
    ystar->sibling       = lambdaNode;
    lambdaNode->sibling  = nullptr;

    // Transform node to '='
    // =
    //  ├── f
    //  └── gamma
    node->type       = "=";
    node->child      = f;
    f->sibling       = gammaNode;
    gammaNode->sibling = nullptr;

    // Clean up unused '=' definition node
    delete defNode;
}

// ── Rule 7: within ────────────────────────────────────────────────────────────
//
// AST:                             ST:
//   within                          gamma
// AST:                      ST:
//   within                    =
//    ├── = (x1, E1)                  ├── x2
//    └── = (x2, E2)                  └── gamma
//                                          ├── lambda
//                                          │    ├── x1
//                                          │    └── E2
//                                          └── E1
//
// within is like a nested let.
void Standardizer::standardizeWithin(ASTNode* node) {
    // node is 'within'
    // child[0] = first '=' node  (x1, E1)
    // child[1] = second '=' node (x2, E2)

    ASTNode* def1 = node->child;          // first '='
    ASTNode* def2 = def1->sibling;        // second '='

    if (def1 == nullptr || def2 == nullptr) {
        std::cerr << "Standardizer Error: malformed 'within' node" << std::endl;
        return;
    }

    // Extract from first definition: x1 and E1
    ASTNode* x1 = def1->child;
    ASTNode* E1 = x1->sibling;

    // Extract from second definition: x2 and E2
    ASTNode* x2 = def2->child;
    ASTNode* E2 = x2->sibling;

    // Detach everything cleanly
    x1->sibling  = nullptr;
    E1->sibling  = nullptr;
    x2->sibling  = nullptr;
    E2->sibling  = nullptr;
    def1->sibling = nullptr;
    def2->sibling = nullptr;
    def1->child  = nullptr;
    def2->child  = nullptr;

    // Build lambda:
    // lambda
    //  ├── x1
    //  └── E2
    ASTNode* lambdaNode  = makeNode("lambda");
    lambdaNode->child    = x1;
    x1->sibling          = E2;

    // Build gamma:
    // gamma
    //  ├── lambda
    //  └── E1
    ASTNode* gammaNode   = makeNode("gamma");
    gammaNode->child     = lambdaNode;
    lambdaNode->sibling  = E1;

    // Transform node to '=':
    // =
    //  ├── x2
    //  └── gamma
    node->type           = "=";
    node->child          = x2;
    x2->sibling          = gammaNode;

    // Clean up now-empty definition nodes
    delete def1;
    delete def2;
}