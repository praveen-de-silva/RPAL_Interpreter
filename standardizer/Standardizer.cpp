// Standardizer.cpp
// Transforms the AST into a Standardized Tree (ST) using 9 rules.
// Rules are applied bottom-up: children are standardized before the parent.
// Author: 230123K

#include "Standardizer.h"
#include <iostream>
#include <stdexcept>

// Create a new AST node with given type and optional value
ASTNode* Standardizer::makeNode(const std::string& type, const std::string& value) {
    return new ASTNode(type, value);
}

// Deep copy a subtree — needed when the same node must appear in two places (e.g. rec rule)
ASTNode* Standardizer::copyTree(ASTNode* node) {
    if (node == nullptr) return nullptr;
    ASTNode* copy = makeNode(node->type, node->value);
    copy->child   = copyTree(node->child);
    copy->sibling = copyTree(node->sibling);
    return copy;
}

// Get the nth child of a node (0-indexed via sibling chain)
ASTNode* Standardizer::getChild(ASTNode* node, int n) {
    if (node == nullptr) return nullptr;
    ASTNode* current = node->child;
    for (int i = 0; i < n; i++) {
        if (current == nullptr) return nullptr;
        current = current->sibling;
    }
    return current;
}

// Count how many children a node has
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

// Main entry point — walks the tree bottom-up and applies each rule
ASTNode* Standardizer::standardize(ASTNode* node) {
    if (node == nullptr) return nullptr;

    // Standardize children first before transforming parent
    ASTNode* child = node->child;
    while (child != nullptr) {
        standardize(child);
        child = child->sibling;
    }

    if (node->type == "let")       standardizeLet(node);
    else if (node->type == "where")     standardizeWhere(node);
    else if (node->type == "fcn_form")  standardizeFcnForm(node);
    else if (node->type == "lambda")    standardizeLambda(node);
    else if (node->type == "and")       standardizeAnd(node);
    else if (node->type == "rec")       standardizeRec(node);
    else if (node->type == "within")    standardizeWithin(node);
    else if (node->type == "@")         standardizeAt(node);
    else if (node->type == "()")        standardizeEmptyParam(node);
    // operators, tau, -> are left as-is for the CSE machine to handle directly

    return node;
}

// Rule 1: let x = E in P  =>  (lambda x . P) E
void Standardizer::standardizeLet(ASTNode* node) {
    ASTNode* defNode = node->child;
    ASTNode* P       = defNode->sibling;

    if (defNode == nullptr || P == nullptr) {
        std::cerr << "Error: malformed let node" << std::endl;
        return;
    }

    ASTNode* x = defNode->child;
    ASTNode* E = x->sibling;

    x->sibling       = nullptr;
    E->sibling       = nullptr;
    defNode->sibling = nullptr;

    ASTNode* lambdaNode  = makeNode("lambda");
    lambdaNode->child    = x;
    x->sibling           = P;
    P->sibling           = nullptr;

    node->type           = "gamma";
    node->child          = lambdaNode;
    lambdaNode->sibling  = E;
    E->sibling           = nullptr;

    defNode->child   = nullptr;
    defNode->sibling = nullptr;
    delete defNode;
}

// Rule 2: P where x = E  =>  same as let (gamma(lambda x P, E))
void Standardizer::standardizeWhere(ASTNode* node) {
    ASTNode* P       = node->child;
    ASTNode* defNode = P->sibling;

    if (P == nullptr || defNode == nullptr) {
        std::cerr << "Error: malformed where node" << std::endl;
        return;
    }

    ASTNode* x = defNode->child;
    ASTNode* E = x->sibling;

    P->sibling       = nullptr;
    x->sibling       = nullptr;
    E->sibling       = nullptr;
    defNode->sibling = nullptr;

    ASTNode* lambdaNode  = makeNode("lambda");
    lambdaNode->child    = x;
    x->sibling           = P;
    P->sibling           = nullptr;

    node->type           = "gamma";
    node->child          = lambdaNode;
    lambdaNode->sibling  = E;
    E->sibling           = nullptr;

    defNode->child   = nullptr;
    defNode->sibling = nullptr;
    delete defNode;
}

// Rule 3: fcn_form f x1 x2 E  =>  = f (lambda x1 . lambda x2 . E)
// Multiple parameters become nested lambdas (curried form)
void Standardizer::standardizeFcnForm(ASTNode* node) {
    ASTNode* f = node->child;

    std::vector<ASTNode*> params;
    ASTNode* current = f->sibling;
    ASTNode* body    = nullptr;

    while (current != nullptr) {
        if (current->sibling == nullptr)
            body = current;
        else
            params.push_back(current);
        current = current->sibling;
    }

    if (body == nullptr) {
        std::cerr << "Error: malformed fcn_form node" << std::endl;
        return;
    }

    f->sibling = nullptr;
    if (!params.empty()) params.back()->sibling = nullptr;
    body->sibling = nullptr;

    // Build nested lambdas from right to left
    ASTNode* innermost = makeNode("lambda");
    innermost->child   = params.back();
    params.back()->sibling = body;

    for (int i = (int)params.size() - 2; i >= 0; i--) {
        ASTNode* outer = makeNode("lambda");
        outer->child   = params[i];
        params[i]->sibling = innermost;
        innermost = outer;
    }

    node->type         = "=";
    node->child        = f;
    f->sibling         = innermost;
    innermost->sibling = nullptr;
}

// Rule 4: lambda with multiple params  =>  nested single-param lambdas
void Standardizer::standardizeLambda(ASTNode* node) {
    if (countChildren(node) <= 2) return; // already single-param, nothing to do

    std::vector<ASTNode*> params;
    ASTNode* current = node->child;
    ASTNode* body    = nullptr;

    while (current != nullptr) {
        if (current->sibling == nullptr)
            body = current;
        else
            params.push_back(current);
        current = current->sibling;
    }

    if (body == nullptr || params.empty()) return;

    for (auto& p : params) p->sibling = nullptr;
    body->sibling = nullptr;

    ASTNode* innermost = makeNode("lambda");
    innermost->child   = params.back();
    params.back()->sibling = body;

    for (int i = (int)params.size() - 2; i >= 0; i--) {
        ASTNode* outer = makeNode("lambda");
        outer->child   = params[i];
        params[i]->sibling = innermost;
        innermost = outer;
    }

    node->child              = params[0];
    params[0]->sibling       = innermost;
    innermost->sibling       = nullptr;
}

// Rule 5: and (simultaneous definitions)  =>  = tau(vars) tau(exprs)
void Standardizer::standardizeAnd(ASTNode* node) {
    ASTNode* tauVars = makeNode("tau");
    ASTNode* tauVals = makeNode("tau");

    ASTNode* lastVar = nullptr;
    ASTNode* lastVal = nullptr;

    ASTNode* current = node->child;
    while (current != nullptr) {
        ASTNode* nextDef = current->sibling;

        ASTNode* xi = current->child;
        ASTNode* Ei = xi->sibling;

        xi->sibling      = nullptr;
        Ei->sibling      = nullptr;
        current->sibling = nullptr;
        current->child   = nullptr;

        if (tauVars->child == nullptr) { tauVars->child = xi; lastVar = xi; }
        else { lastVar->sibling = xi; lastVar = xi; }

        if (tauVals->child == nullptr) { tauVals->child = Ei; lastVal = Ei; }
        else { lastVal->sibling = Ei; lastVal = Ei; }

        delete current;
        current = nextDef;
    }

    node->type       = "=";
    node->child      = tauVars;
    tauVars->sibling = tauVals;
    tauVals->sibling = nullptr;
}

// Rule 6: rec f = E  =>  = f (Y* (lambda f . E))
// Y* is the fixed-point combinator that enables recursion
void Standardizer::standardizeRec(ASTNode* node) {
    ASTNode* defNode = node->child;

    if (defNode == nullptr || defNode->child == nullptr) {
        std::cerr << "Error: malformed rec node" << std::endl;
        return;
    }

    ASTNode* f = defNode->child;
    ASTNode* E = f->sibling;

    f->sibling       = nullptr;
    E->sibling       = nullptr;
    defNode->child   = nullptr;
    defNode->sibling = nullptr;

    ASTNode* fCopy = copyTree(f); // need f in two places

    ASTNode* lambdaNode = makeNode("lambda");
    lambdaNode->child   = fCopy;
    fCopy->sibling      = E;
    E->sibling          = nullptr;

    ASTNode* ystar      = makeNode("Ystar");
    ASTNode* gammaNode  = makeNode("gamma");
    gammaNode->child    = ystar;
    ystar->sibling      = lambdaNode;
    lambdaNode->sibling = nullptr;

    node->type          = "=";
    node->child         = f;
    f->sibling          = gammaNode;
    gammaNode->sibling  = nullptr;

    delete defNode;
}

// Rule 7: within  =>  = x2 (gamma (lambda x1 . E2) E1)
// Similar to a nested let binding
void Standardizer::standardizeWithin(ASTNode* node) {
    ASTNode* def1 = node->child;
    ASTNode* def2 = def1->sibling;

    if (def1 == nullptr || def2 == nullptr) {
        std::cerr << "Error: malformed within node" << std::endl;
        return;
    }

    ASTNode* x1 = def1->child;
    ASTNode* E1 = x1->sibling;
    ASTNode* x2 = def2->child;
    ASTNode* E2 = x2->sibling;

    x1->sibling  = nullptr;
    E1->sibling  = nullptr;
    x2->sibling  = nullptr;
    E2->sibling  = nullptr;
    def1->sibling = nullptr;
    def2->sibling = nullptr;
    def1->child  = nullptr;
    def2->child  = nullptr;

    ASTNode* lambdaNode = makeNode("lambda");
    lambdaNode->child   = x1;
    x1->sibling         = E2;

    ASTNode* gammaNode  = makeNode("gamma");
    gammaNode->child    = lambdaNode;
    lambdaNode->sibling = E1;

    node->type          = "=";
    node->child         = x2;
    x2->sibling         = gammaNode;

    delete def1;
    delete def2;
}

// Rule 8: E @n R  =>  gamma(gamma(n, E), R)
// Infix application: @n means apply n to E, then apply result to R
void Standardizer::standardizeAt(ASTNode* node) {
    ASTNode* E = node->child;
    ASTNode* n = E->sibling;
    ASTNode* R = n->sibling;

    E->sibling = nullptr;
    n->sibling = nullptr;
    R->sibling = nullptr;

    ASTNode* innerGamma  = makeNode("gamma");
    innerGamma->child    = n;
    n->sibling           = E;

    node->type           = "gamma";
    node->child          = innerGamma;
    innerGamma->sibling  = R;
}

// Rule 9: empty parameter ()  =>  dummy identifier
// Handles zero-argument functions like f() = E
void Standardizer::standardizeEmptyParam(ASTNode* node) {
    node->type  = "IDENTIFIER";
    node->value = "dummy";
}
