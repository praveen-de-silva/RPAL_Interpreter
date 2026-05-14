// ============================================================
// Standardizer.h
// Declares the Standardizer class for the RPAL Interpreter.
//
// The Standardizer is Stage 4 of the pipeline.
// It transforms the Abstract Syntax Tree (AST) produced by
// the Parser into a Standardized Tree (ST).
//
// The ST only contains these node types:
//   gamma, lambda, tau, Ystar,
//   IDENTIFIER, INTEGER, STRING,
//   true, false, nil, dummy,
//   =,  +, -, neg, *, /, **, @,
//   or, &, not, gr, ge, ls, le, eq, ne,
//   aug, ->
//
// Transformation rules applied:
//   Rule 1:  let x = E in P    =>  gamma (lambda x P) E
//   Rule 2:  where P x = E     =>  gamma (lambda x P) E
//   Rule 3:  fcn_form f x = E  =>  = f (lambda x E)
//   Rule 4:  lambda (multi-Vb) =>  nested lambdas
//   Rule 5:  and definitions   =>  = (tau X...) (tau E...)
//   Rule 6:  rec f = E         =>  = f (Ystar (lambda f E))
//   Rule 7:  within            =>  gamma (lambda x2 E2) (gamma (lambda x1 E1) E0)
//
// IMPORTANT: ASTNode uses child/sibling pointers (not a vector).
//   node->child   = first child
//   child->sibling = next sibling (linked list of children)
//
// Author: 230123K
// ============================================================

#ifndef STANDARDIZER_H
#define STANDARDIZER_H
 
#include "../parser/ASTNode.h"

class Standardizer {
private:
    // -- Helper: node creation --
 
    // Creates a new node with given type and optional value
    ASTNode* makeNode(const std::string& type,
                      const std::string& value = "");
 
    // Creates a deep copy of a subtree (needed for rec rule)
    ASTNode* copyTree(ASTNode* node);
 
    // -- Helper: child access --
 
    // Returns the Nth child of a node (0-indexed)
    // Children are stored as linked list via sibling pointers
    ASTNode* getChild(ASTNode* node, int n);
 
    // Returns the number of children of a node
    int countChildren(ASTNode* node);
 
    // Detaches and returns the Nth child (removes from sibling chain)
    ASTNode* detachChild(ASTNode* node, int n);
 
    // -- Transformation rules --
 
    // Rule 1: let x = E in P  =>  gamma (lambda x P) E
    void standardizeLet(ASTNode* node);
 
    // Rule 2: where P x = E  =>  gamma (lambda x P) E
    void standardizeWhere(ASTNode* node);
 
    // Rule 3 & 4: fcn_form and multi-param lambda
    // fcn_form f x1 x2 E  =>  = f (lambda x1 (lambda x2 E))
    void standardizeFcnForm(ASTNode* node);
 
    // Rule 4: lambda with multiple Vb params
    // lambda x1 x2 E  =>  lambda x1 (lambda x2 E)
    void standardizeLambda(ASTNode* node);
 
    // Rule 5: and Dr1 Dr2 ...  =>  = (tau X1 X2...) (tau E1 E2...)
    void standardizeAnd(ASTNode* node);
 
    // Rule 6: rec (= f E)  =>  = f (Ystar (lambda f E))
    void standardizeRec(ASTNode* node);
 
    // Rule 7: within  =>  gamma (lambda x2 E2) (gamma (lambda x1 E1) E0)
    void standardizeWithin(ASTNode* node);

    // Rule 8: @ (infix application)
    // E @n R  =>  gamma (gamma n E) R
    void standardizeAt(ASTNode* node);

    // Rule 9: () (zero-param Vb in fcn_form / lambda)
    // f () = E  =>  = f (lambda dummy E)
    void standardizeEmptyParam(ASTNode* node);

    // Rule A1: Binary operator  Op(E1, E2)  =>  gamma (gamma IDENTIFIER(Op) E1) E2
    // Op in [+, -, *, /, **, aug, or, &, gr, ge, ls, le, eq, ne]
    void standardizeOp(ASTNode* node);

    // Rule A2: Unary operator  Uop(E)  =>  gamma IDENTIFIER(Uop) E
    // Uop in [not, neg]
    void standardizeUop(ASTNode* node);

    // Rule A3: tau  =>  aug chain built from nil
    // tau(E1,E2,...,En)  =>  gamma(gamma(IDENTIFIER(aug), ...gamma(gamma(IDENTIFIER(aug), nil), E1)...), En)
    void standardizeTau(ASTNode* node);

    // Rule A4: ->  (conditional)
    // B -> T | E  =>  gamma(gamma(gamma(gamma(IDENTIFIER(Cond),B), lambda(dummy,T)), lambda(dummy,E)), nil)
    // Cond is a 3-arg curried built-in that selects a thunk; nil forces the selected thunk.
    void standardizeCond(ASTNode* node);

    // Rule A5: lambda with comma (tuple-pattern) bound variable
    // lambda(,(x,y,...), E)  =>  lambda(_T, gamma(lambda(x, gamma(lambda(y,...E...), gamma(_T,2))), gamma(_T,1)))
    // A fresh _T variable extracts each element by index.
    void standardizeTuplePattern(ASTNode* node);

public:
    // Main standardize function.
    // Applies all transformation rules recursively to the entire AST.
    // Modifies the tree in-place (no new root returned separately).
    // Returns the root of the Standardized Tree.
    ASTNode* standardize(ASTNode* node);
};

#endif // STANDARDIZER_H