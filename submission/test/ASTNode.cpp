// ================================
// ASTNode.cpp
// Implements ASTNode construction, destruction, and printing.
// ================================

#include "ASTNode.h"
#include <iostream>

// Initialise type, value, and null-out both pointers.
ASTNode::ASTNode(std::string t, std::string v)
    : type(t), value(v), child(nullptr), sibling(nullptr) {}

// Recursively free the whole subtree.
// Deleting child walks the child's sibling chain automatically
// because ~ASTNode() is called on each node in turn.
ASTNode::~ASTNode() {
    delete child;
    delete sibling;
}

// Pre-order traversal: print this node, then recurse into child,
// then recurse into sibling (at the SAME depth, not depth+1).
// Output format matches the expected rpal reference output exactly.
void ASTNode::print(int depth) const {
    // Print 'depth' dots as indentation
    for (int i = 0; i < depth; ++i) {
        std::cout << ".";
    }

    // Leaf nodes carry a value; internal nodes are identified by type alone
    if (type == "IDENTIFIER" || type == "INTEGER" || type == "STRING") {
        std::cout << "<" << type << ":" << value << ">\n";
    } else {
        std::cout << type << "\n";
    }

    // Children are one level deeper
    if (child) {
        child->print(depth + 1);
    }
    // Siblings share the same depth as this node
    if (sibling) {
        sibling->print(depth);
    }
}
