// ================================
// ASTNode.h
// Defines the node type for the Abstract Syntax Tree (AST) and the Standardized Tree (ST).
// ================================

#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <vector>

class ASTNode {
public:
    std::string type;     // node type: "let", "lambda", "gamma", "IDENTIFIER", etc.
    std::string value;    // leaf value: identifier name, integer digits, or string text
    ASTNode* child;       // first child (nullptr if leaf)
    ASTNode* sibling;     // next sibling in parent's child list (nullptr if last)

    // Constructs a node with the given type and optional value.
    // child and sibling are initialised to nullptr.
    ASTNode(std::string t, std::string v = "");

    // Recursively deletes the child and sibling subtrees.
    ~ASTNode();

    // Prints the subtree rooted at this node in pre-order.
    // Leaf nodes (IDENTIFIER / INTEGER / STRING) are printed as <TYPE:value>.
    // Internal nodes are printed as their type string.
    // Indentation is 'depth' dots, children are printed at depth+1.
    void print(int depth = 0) const;
};

#endif
