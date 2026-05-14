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
    else if (node->type == "@") {
        standardizeAt(node);
    }
    else if (node->type == "()") {
        standardizeEmptyParam(node);
    }
    else if (node->type == "+"  || node->type == "-"  || node->type == "*"  ||
             node->type == "/"  || node->type == "**" || node->type == "aug" ||
             node->type == "or" || node->type == "&"  || node->type == "gr"  ||
             node->type == "ge" || node->type == "ls" || node->type == "le"  ||
             node->type == "eq" || node->type == "ne") {
        standardizeOp(node);
    }
    else if (node->type == "not" || node->type == "neg") {
        standardizeUop(node);
    }
    else if (node->type == "tau") {
        standardizeTau(node);
    }
    else if (node->type == "->") {
        standardizeCond(node);
    }
    // All other nodes (gamma, IDENTIFIER, INTEGER, etc.) pass through

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

    // Re-standardize the created lambda chain: if any Vb is a tuple-pattern
    // (comma node), standardizeTuplePattern must be applied to it.
    standardize(innermost);
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
    int numChildren = countChildren(node);

    if (numChildren > 2) {
        // ── Multiple params: collapse to nested single-param lambdas ──────────
        std::vector<ASTNode*> params;
        ASTNode* cur  = node->child;
        ASTNode* body = nullptr;
        while (cur != nullptr) {
            if (cur->sibling == nullptr) body = cur;
            else params.push_back(cur);
            cur = cur->sibling;
        }
        if (body == nullptr || params.empty()) return;

        for (auto& p : params) p->sibling = nullptr;
        body->sibling = nullptr;

        // Build from right to left; loop to i >= 1 so params[0] is NOT
        // placed inside a newly created lambda (avoids a parent-child cycle).
        ASTNode* innermost      = makeNode("lambda");
        innermost->child        = params.back();
        params.back()->sibling  = body;

        for (int i = (int)params.size() - 2; i >= 1; i--) {
            ASTNode* outer   = makeNode("lambda");
            outer->child     = params[i];
            params[i]->sibling = innermost;
            innermost        = outer;
        }

        node->child         = params[0];
        params[0]->sibling  = innermost;
        innermost->sibling  = nullptr;

        // Re-standardize inner lambdas: if any inner param is a tuple pattern
        // (comma node) it needs to go through standardizeTuplePattern.
        standardize(innermost);
    }

    // ── Tuple-pattern bound variable: lambda(,(x,y,...), E) ──────────────────
    if (node->child && node->child->type == ",") {
        standardizeTuplePattern(node);
    }
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

    // The two tau nodes were just created here (not visited by the bottom-up
    // traversal), so explicitly standardize them now.
    standardize(tauVars);
    standardize(tauVals);
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

// ── Rule 8: @ (infix application) ────────────────────────────────────────────
//
// AST:                      ST:
//   @                         gamma
//    ├── E                      ├── gamma
//    ├── n                      │    ├── n
//    └── R                      │    └── E
//                               └── R
//
// E @n R  means  apply n to E first, then apply result to R.
// Equivalent to: (n E) R  =  gamma(gamma(n, E), R)
void Standardizer::standardizeAt(ASTNode* node) {
    ASTNode* E = node->child;
    ASTNode* n = E->sibling;
    ASTNode* R = n->sibling;

    // Detach all
    E->sibling = nullptr;
    n->sibling = nullptr;
    R->sibling = nullptr;

    // Inner gamma: gamma(n, E)
    ASTNode* innerGamma = makeNode("gamma");
    innerGamma->child   = n;
    n->sibling          = E;

    // Transform node in place to outer gamma: gamma(innerGamma, R)
    node->type          = "gamma";
    node->child         = innerGamma;
    innerGamma->sibling = R;
}

// ── Rule A4: -> (conditional) ─────────────────────────────────────────────────
//
// AST:              ST:
//   ->               gamma
//   ├── B             ├── gamma
//   ├── T             │    ├── gamma
//   └── E             │    │    ├── gamma
//                     │    │    │    ├── IDENTIFIER(Cond)
//                     │    │    │    └── B
//                     │    │    └── lambda
//                     │    │         ├── IDENTIFIER(dummy)
//                     │    │         └── T
//                     │    └── lambda
//                     │         ├── IDENTIFIER(dummy)
//                     │         └── E
//                     └── nil
//
// Cond is a 3-arg curried built-in:
//   (Cond B)          → partial1(Cond, B)
//   (partial1 thunkT) → partial2(Cond, {B, thunkT})   [tuple trick]
//   (partial2 thunkE) → thunkT if B else thunkE
// The outermost gamma forces the selected thunk by applying it to nil.
void Standardizer::standardizeCond(ASTNode* node) {
    ASTNode* B = node->child;
    ASTNode* T = B->sibling;
    ASTNode* E = T->sibling;

    B->sibling = nullptr;
    T->sibling = nullptr;
    E->sibling = nullptr;

    // lambda(dummy, T) — thunk for true branch
    ASTNode* dummyT  = makeNode("IDENTIFIER", "dummy");
    ASTNode* thunkT  = makeNode("lambda");
    thunkT->child    = dummyT;
    dummyT->sibling  = T;

    // lambda(dummy, E) — thunk for false branch
    ASTNode* dummyE  = makeNode("IDENTIFIER", "dummy");
    ASTNode* thunkE  = makeNode("lambda");
    thunkE->child    = dummyE;
    dummyE->sibling  = E;

    // gamma(IDENTIFIER(Cond), B)
    ASTNode* condId  = makeNode("IDENTIFIER", "Cond");
    ASTNode* g1      = makeNode("gamma");
    g1->child        = condId;
    condId->sibling  = B;

    // gamma(g1, thunkT)
    ASTNode* g2      = makeNode("gamma");
    g2->child        = g1;
    g1->sibling      = thunkT;

    // gamma(g2, thunkE)
    ASTNode* g3      = makeNode("gamma");
    g3->child        = g2;
    g2->sibling      = thunkE;

    // nil — dummy argument that forces the selected thunk
    ASTNode* nilNode = makeNode("nil");

    // Transform node in-place to outermost gamma(g3, nil)
    node->type       = "gamma";
    node->value      = "";
    node->child      = g3;
    g3->sibling      = nilNode;
}

// ── Rule A3: tau ──────────────────────────────────────────────────────────────
//
// tau(E1, E2, ..., En)  builds a tuple via aug starting from nil:
//
//   nil aug E1  =>  (E1)
//   (E1) aug E2  =>  (E1, E2)
//   ...
//
// In Approach A, aug is itself an IDENTIFIER built-in, so each step becomes:
//
//   gamma(gamma(IDENTIFIER("aug"), acc), E_i)
//
// Full example for tau(E1, E2, E3):
//   gamma
//    ├── gamma
//    │    ├── IDENTIFIER(aug)
//    │    └── gamma
//    │         ├── gamma
//    │         │    ├── IDENTIFIER(aug)
//    │         │    └── gamma
//    │         │         ├── gamma
//    │         │         │    ├── IDENTIFIER(aug)
//    │         │         │    └── nil
//    │         │         └── E1
//    │         └── E2
//    └── E3
void Standardizer::standardizeTau(ASTNode* node) {
    // Detach and collect all children
    std::vector<ASTNode*> elems;
    ASTNode* child = node->child;
    while (child) {
        ASTNode* next = child->sibling;
        child->sibling = nullptr;
        elems.push_back(child);
        child = next;
    }
    node->child = nullptr;

    if (elems.empty()) {
        // tau() → nil
        node->type  = "nil";
        node->value = "";
        return;
    }

    // Start accumulator: nil
    ASTNode* acc = makeNode("nil");

    // Build aug gamma-chain for all but the last element as fresh nodes
    for (int i = 0; i < (int)elems.size() - 1; ++i) {
        ASTNode* augId  = makeNode("IDENTIFIER", "aug");
        ASTNode* inner  = makeNode("gamma");
        inner->child    = augId;
        augId->sibling  = acc;

        ASTNode* outer  = makeNode("gamma");
        outer->child    = inner;
        inner->sibling  = elems[i];

        acc = outer;
    }

    // For the outermost step, transform node in-place (avoids an extra alloc + copy)
    ASTNode* augId  = makeNode("IDENTIFIER", "aug");
    ASTNode* inner  = makeNode("gamma");
    inner->child    = augId;
    augId->sibling  = acc;

    node->type      = "gamma";
    node->value     = "";
    node->child     = inner;
    inner->sibling  = elems.back();
}

// ── Rule A1: Binary operator ──────────────────────────────────────────────────
//
// AST:                      ST:
//   Op                        gamma
//    ├── E1                     ├── gamma
//    └── E2                     │    ├── IDENTIFIER(Op)
//                               │    └── E1
//                               └── E2
//
// Op(E1,E2)  =>  gamma(gamma(IDENTIFIER(Op), E1), E2)
void Standardizer::standardizeOp(ASTNode* node) {
    ASTNode* E1 = node->child;
    ASTNode* E2 = E1->sibling;

    E1->sibling = nullptr;
    E2->sibling = nullptr;

    // IDENTIFIER node carrying the operator name (e.g. "+")
    ASTNode* opId = makeNode("IDENTIFIER", node->type);

    // Inner gamma: gamma(opId, E1)
    ASTNode* innerGamma  = makeNode("gamma");
    innerGamma->child    = opId;
    opId->sibling        = E1;

    // Transform node in-place to outer gamma: gamma(innerGamma, E2)
    node->type           = "gamma";
    node->value          = "";
    node->child          = innerGamma;
    innerGamma->sibling  = E2;
}

// ── Rule A2: Unary operator ───────────────────────────────────────────────────
//
// AST:                      ST:
//   Uop                       gamma
//    └── E                      ├── IDENTIFIER(Uop)
//                               └── E
//
// Uop(E)  =>  gamma(IDENTIFIER(Uop), E)
void Standardizer::standardizeUop(ASTNode* node) {
    ASTNode* E = node->child;
    E->sibling = nullptr;

    ASTNode* opId = makeNode("IDENTIFIER", node->type);

    node->type    = "gamma";
    node->value   = "";
    node->child   = opId;
    opId->sibling = E;
}

// ── Rule 9: () zero-param Vb ──────────────────────────────────────────────────
//
// A '()' node appearing as a Vb in fcn_form or lambda means
// the function takes no meaningful argument.
// Standardize by replacing it with 'dummy' so that
// standardizeFcnForm / standardizeLambda produce: lambda dummy E
void Standardizer::standardizeEmptyParam(ASTNode* node) {
    node->type  = "IDENTIFIER";
    node->value = "dummy";
}

// ── Rule A5: lambda with comma (tuple-pattern) bound variable ─────────────────
//
// lambda(,(x,y,...), E)  =>  lambda(_T, let_chain)
//
// where let_chain (for vars x,y with Temp _T) is:
//   gamma(lambda(x, gamma(lambda(y, E), gamma(_T,2))), gamma(_T,1))
//
// i.e. each variable is bound to the i-th element of the tuple argument.
// A fresh _T identifier (guaranteed unique by a static counter) is used
// to hold the incoming tuple without colliding with user-defined names.
void Standardizer::standardizeTuplePattern(ASTNode* node) {
    ASTNode* commaNode = node->child;        // ',' node holding var names
    ASTNode* body      = commaNode->sibling; // already-standardized body
    commaNode->sibling = nullptr;
    body->sibling      = nullptr;

    // Extract variable names from the comma list
    std::vector<std::string> vars;
    ASTNode* v = commaNode->child;
    while (v) {
        vars.push_back(v->value);
        v = v->sibling;
    }

    // Fresh temp variable name; static counter ensures uniqueness across calls
    static int tempCount = 0;
    std::string tempName = "_T" + std::to_string(tempCount++);

    // Build nested let-bindings from innermost (last var) to outermost (first var).
    // For vars [x, y] and body E:
    //   i=2: current = gamma(lambda(y, E),    gamma(_T, 2))
    //   i=1: current = gamma(lambda(x, prev), gamma(_T, 1))
    ASTNode* current = body;

    for (int i = (int)vars.size(); i >= 1; --i) {
        // gamma(_T, i)  — fetch i-th element from tuple
        ASTNode* tempRef    = makeNode("IDENTIFIER", tempName);
        ASTNode* idxNode    = makeNode("INTEGER",    std::to_string(i));
        ASTNode* fetchGamma = makeNode("gamma");
        fetchGamma->child   = tempRef;
        tempRef->sibling    = idxNode;

        // lambda(var_i, current_body)
        ASTNode* varId   = makeNode("IDENTIFIER", vars[i - 1]);
        ASTNode* lam     = makeNode("lambda");
        lam->child       = varId;
        varId->sibling   = current;

        // gamma(lam, fetchGamma)  — the let-binding for var_i
        ASTNode* letG    = makeNode("gamma");
        letG->child      = lam;
        lam->sibling     = fetchGamma;

        current = letG;
    }

    // Transform node in-place: lambda(_T, current)
    ASTNode* tempParam  = makeNode("IDENTIFIER", tempName);
    node->child         = tempParam;
    tempParam->sibling  = current;

    // Delete the original comma node and its var-identifier children
    // (we created fresh IDENTIFIER nodes above, so originals are unused)
    ASTNode* toDelete = commaNode->child;
    while (toDelete) {
        ASTNode* next = toDelete->sibling;
        delete toDelete;
        toDelete = next;
    }
    delete commaNode;
}