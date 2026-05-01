#include "ASTNode.h"
#include <iostream>

ASTNode::ASTNode(std::string t, std::string v) 
    : type(t), value(v), child(nullptr), sibling(nullptr) {}

ASTNode::~ASTNode() {
    delete child;
    delete sibling;
}

void ASTNode::print(int depth) const {
    for (int i = 0; i < depth; ++i) {
        std::cout << ".";
    }
    
    if (type == "IDENTIFIER" || type == "INTEGER" || type == "STRING") {
        std::cout << "<" << type << ":" << value << ">\n";
    } else {
        std::cout << type << "\n";
    }

    if (child) {
        child->print(depth + 1);
    }
    if (sibling) {
        sibling->print(depth);
    }
}
